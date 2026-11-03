#include <avr/eeprom.h>
#include <util/crc16.h>
#include <string.h>
#include "storage.h"
#include "protocol.h"

#define STORAGE_MAGIC 0xB8

StorageData storage;
static StorageData EEMEM eeStorage;

static uint8_t checksum(const StorageData *d) {
    const uint8_t *p = (const uint8_t *)d;
    uint8_t crc = 0;
    for (uint8_t i = 0; i < sizeof(StorageData) - 1; i++) {
        crc = _crc8_ccitt_update(crc, p[i]);
    }
    return crc;
}

static void defaults(void) {
    memset(&storage, 0, sizeof(storage));
    storage.magic = STORAGE_MAGIC;
    storage.settings.lives = 3;
    storage.settings.level = LEVEL_NORMAL;
    storage.settings.control = CONTROL_SLIDER;
    storage.settings.invertServo = 0;
    storage.settings.brightness = 4;
}

void storageLoad(void) {
    eeprom_read_block(&storage, &eeStorage, sizeof(storage));
    Settings *s = &storage.settings;
    if (storage.magic != STORAGE_MAGIC || storage.checksum != checksum(&storage)
        || s->lives < 1 || s->lives > 5 || s->level >= LEVEL_COUNT || s->control > 1
        || s->invertServo > 1 || s->brightness < 1 || s->brightness > 5) {
        defaults();
    }
}

void storageSave(void) {
    storage.checksum = checksum(&storage);
    eeprom_update_block(&storage, &eeStorage, sizeof(storage));
}

uint8_t highscoreInsert(uint16_t score, uint8_t level) {
    if (score == 0) return 0xFF;
    for (uint8_t i = 0; i < HIGHSCORE_COUNT; i++) {
        if (score > storage.hs[i].score) {
            memmove(&storage.hs[i + 1], &storage.hs[i], (HIGHSCORE_COUNT - 1 - i) * sizeof(HighscoreEntry));
            storage.hs[i].score = score;
            storage.hs[i].level = level;
            storageSave();
            return i;
        }
    }
    return 0xFF;
}

void highscoreClear(void) {
    memset(storage.hs, 0, sizeof(storage.hs));
    storageSave();
}
