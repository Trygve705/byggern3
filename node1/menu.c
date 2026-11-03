#include <avr/pgmspace.h>
#include <stdio.h>
#include "menu.h"
#include "screen.h"
#include "oled.h"
#include "input.h"
#include "storage.h"
#include "protocol.h"

#define VISIBLE_ITEMS 6

enum { ITEM_ACTION, ITEM_SUBMENU, ITEM_SETTING, ITEM_BACK };
enum { SET_LIVES, SET_LEVEL, SET_CONTROL, SET_INVERT, SET_BRIGHTNESS };

typedef struct {
    const char *label;
    uint8_t type;
    uint8_t arg;
} MenuItem;

typedef struct {
    const char *title;
    const MenuItem *items;
    uint8_t count;
    uint8_t parent;
} MenuDef;

#define S(name, text) static const char name[] PROGMEM = text

S(t_main, "PING PONG");
S(t_settings, "INNSTILLINGER");
S(t_confirm, "SLETTE TOPPLISTEN?");
S(t_pause, "PAUSE");

S(l_start, "Start spill");
S(l_practice, "Trening");
S(l_highscores, "Toppliste");
S(l_settings, "Innstillinger");
S(l_diag, "Diagnostikk");
S(l_help, "Hjelp");
S(l_lives, "Liv");
S(l_level, "Nivå");
S(l_control, "Styring");
S(l_invert, "Snu vinkel");
S(l_bright, "Lysstyrke");
S(l_clear, "Slett toppliste");
S(l_back, "Tilbake");
S(l_no, "Nei");
S(l_yes, "Ja, slett");
S(l_resume, "Fortsett");
S(l_quit, "Avslutt spill");

static const MenuItem PROGMEM mainItems[] = {
    {l_start, ITEM_ACTION, ACT_START},
    {l_practice, ITEM_ACTION, ACT_PRACTICE},
    {l_highscores, ITEM_ACTION, ACT_HIGHSCORES},
    {l_settings, ITEM_SUBMENU, MENU_SETTINGS},
    {l_diag, ITEM_ACTION, ACT_DIAG},
    {l_help, ITEM_ACTION, ACT_HELP},
};
static const MenuItem PROGMEM settingsItems[] = {
    {l_lives, ITEM_SETTING, SET_LIVES},
    {l_level, ITEM_SETTING, SET_LEVEL},
    {l_control, ITEM_SETTING, SET_CONTROL},
    {l_invert, ITEM_SETTING, SET_INVERT},
    {l_bright, ITEM_SETTING, SET_BRIGHTNESS},
    {l_clear, ITEM_SUBMENU, MENU_CONFIRM_CLEAR},
    {l_back, ITEM_BACK, 0},
};
static const MenuItem PROGMEM confirmItems[] = {
    {l_no, ITEM_BACK, 0},
    {l_yes, ITEM_ACTION, ACT_CLEAR_HS},
};
static const MenuItem PROGMEM pauseItems[] = {
    {l_resume, ITEM_ACTION, ACT_RESUME},
    {l_quit, ITEM_ACTION, ACT_QUIT},
};

#define N(a) (sizeof(a) / sizeof(a[0]))
static const MenuDef PROGMEM menus[MENU_COUNT] = {
    [MENU_MAIN] = {t_main, mainItems, N(mainItems), MENU_MAIN},
    [MENU_SETTINGS] = {t_settings, settingsItems, N(settingsItems), MENU_MAIN},
    [MENU_CONFIRM_CLEAR] = {t_confirm, confirmItems, N(confirmItems), MENU_SETTINGS},
    [MENU_PAUSE] = {t_pause, pauseItems, N(pauseItems), MENU_PAUSE},
};

S(v_easy, "Lett");
S(v_normal, "Normal");
S(v_hard, "Vanskelig");
S(v_slider, "Slider");
S(v_joystick, "Joystick");
S(v_off, "Av");
S(v_on, "På");
static const char *const PROGMEM levelNames[LEVEL_COUNT] = {v_easy, v_normal, v_hard};

static uint8_t current;
static uint8_t selected[MENU_COUNT];
static uint8_t top[MENU_COUNT];
static MenuDef def; // gjeldende meny, kopiert fra flash

const char *levelName(uint8_t level) {
    return (const char *)pgm_read_ptr(&levelNames[level < LEVEL_COUNT ? level : LEVEL_NORMAL]);
}

static void getItem(uint8_t i, MenuItem *item) {
    memcpy_P(item, &def.items[i], sizeof(MenuItem));
}

void menuOpen(uint8_t menu) {
    current = menu;
    memcpy_P(&def, &menus[menu], sizeof(MenuDef));
    if (menu == MENU_CONFIRM_CLEAR || menu == MENU_PAUSE) selected[menu] = 0;
}

uint8_t menuCurrent(void) {
    return current;
}

// Endrer en innstilling med delta (+1/-1), med omløp
static void changeSetting(uint8_t id, int8_t delta) {
    Settings *s = &storage.settings;
    uint8_t *v;
    uint8_t min = 0, max;
    switch (id) {
    case SET_LIVES:      v = &s->lives; min = 1; max = 5; break;
    case SET_LEVEL:      v = &s->level; max = LEVEL_COUNT - 1; break;
    case SET_CONTROL:    v = &s->control; max = 1; break;
    case SET_INVERT:     v = &s->invertServo; max = 1; break;
    default:             v = &s->brightness; min = 1; max = 5; break;
    }
    if (delta > 0) *v = (*v >= max) ? min : *v + 1;
    else *v = (*v <= min) ? max : *v - 1;
    storageSave();
}

static void settingText(uint8_t id, char *buf, uint8_t size) {
    const Settings *s = &storage.settings;
    switch (id) {
    case SET_LIVES:      snprintf_P(buf, size, PSTR("%u"), s->lives); break;
    case SET_LEVEL:      strncpy_P(buf, levelName(s->level), size); break;
    case SET_CONTROL:    strncpy_P(buf, s->control ? v_joystick : v_slider, size); break;
    case SET_INVERT:     strncpy_P(buf, s->invertServo ? v_on : v_off, size); break;
    default:             snprintf_P(buf, size, PSTR("%u"), s->brightness); break;
    }
    buf[size - 1] = 0;
}

uint8_t menuUpdate(uint8_t pressed, uint8_t repeated) {
    uint8_t *sel = &selected[current];
    MenuItem item;

    if (repeated & KEY_UP) *sel = (*sel == 0) ? def.count - 1 : *sel - 1;
    if (repeated & KEY_DOWN) *sel = (*sel + 1 >= def.count) ? 0 : *sel + 1;

    getItem(*sel, &item);

    if (item.type == ITEM_SETTING) {
        if (repeated & (KEY_RIGHT | KEY_OK)) { changeSetting(item.arg, 1); return ACT_SETTING_CHANGED; }
        if (repeated & KEY_LEFT) { changeSetting(item.arg, -1); return ACT_SETTING_CHANGED; }
        return ACT_NONE;
    }

    if (pressed & (KEY_RIGHT | KEY_OK)) {
        switch (item.type) {
        case ITEM_ACTION:  return item.arg;
        case ITEM_SUBMENU: menuOpen(item.arg); return ACT_NONE;
        case ITEM_BACK:    menuOpen(def.parent); return ACT_NONE;
        }
    }
    if ((pressed & KEY_LEFT) && def.parent != current) {
        menuOpen(def.parent);
    }
    return ACT_NONE;
}

void menuDraw(void) {
    uint8_t sel = selected[current];
    uint8_t *t = &top[current];
    if (sel < *t) *t = sel;
    if (sel >= *t + VISIBLE_ITEMS) *t = sel - VISIBLE_ITEMS + 1;

    screenInvert(0, 1);
    screenCenter_P(0, def.title);
    if (*t > 0) screenPutGlyph(0, 0, GLYPH_UP);
    if (*t + VISIBLE_ITEMS < def.count) screenPutGlyph(0, SCREEN_COLS - 1, GLYPH_DOWN);

    for (uint8_t row = 0; row < VISIBLE_ITEMS && *t + row < def.count; row++) {
        uint8_t i = *t + row;
        uint8_t line = 1 + row;
        MenuItem item;
        getItem(i, &item);

        screenPuts_P(line, 1, item.label);
        if (item.type == ITEM_SETTING) {
            char buf[11];
            settingText(item.arg, buf, sizeof(buf));
            screenPuts(line, SCREEN_COLS - 2 - strlen(buf), buf); // æøå i verdiene teller ikke (bare "På", ok)
            if (i == sel) {
                screenPutGlyph(line, SCREEN_COLS - 3 - strlen(buf), GLYPH_LEFT);
                screenPutGlyph(line, SCREEN_COLS - 1, GLYPH_RIGHT);
            }
        } else if (item.type == ITEM_SUBMENU) {
            screenPutGlyph(line, SCREEN_COLS - 1, GLYPH_RIGHT);
        }
        screenInvert(line, i == sel);
    }
}
