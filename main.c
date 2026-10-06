/*
 * Byggern.c
 *
 * Created: 01.09.2026 13:33:28
 * Author : trygvegn
 *
 * Test av CAN-kontrolleren (MCP2515) i loopback-modus.
 *
 * Del 1 kjorer en serie engangstester og skriver OK/FEIL for hver:
 *   1. SPI-lesing/skriving av register
 *   2. MCP2515 er i loopback-modus
 *   3. Bit Modify endrer bare bitene i masken
 *   4. Read Status viser mottatt melding
 *   5. Faste testmeldinger (ulike ID-er, lengder og data) kommer tilbake uendret
 *   6. INT-pinnen (PD2/INT0) gir avbrudd ved mottak
 *   7. Mottak uten ny melding returnerer 0
 *
 * Del 2 sender joystick- og sliderposisjon kontinuerlig og sammenligner
 * mottatt melding med det som ble sendt.
 */

#include <avr/io.h>
#include <stdio.h>
#include "UART.h"
#include "adc.h"
#include "spi.h"
#include "oled.h"
#include "menu.h"
#include "io.h"
#include "mpc2515_driver.h"
#include "CAN_communication.h"
#include "mcp2515.h"

#define F_CPU 4915200UL
#include <util/delay.h>

#define MCP_TXB1D0  0x46   // dataregister i TXB1, brukes ikke ellers

extern volatile uint8_t can_flag;   // settes i ISR(INT0_vect) i CAN_communication.c

static uint8_t tests_ok = 0;
static uint8_t tests_fail = 0;

static void report(const char *name, uint8_t ok) {
    printf("[%s] %s\r\n", ok ? " OK " : "FEIL", name);
    if (ok) tests_ok++; else tests_fail++;
}

static void print_msg(const char *prefix, const CAN_message *m) {
    printf("   %s id=0x%03x len=%u data:", prefix, m->id, m->length);
    for (uint8_t i = 0; i < m->length && i < 8; i++) {
        printf(" %02x", m->data[i]);
    }
    printf("\r\n");
}

static uint8_t messages_equal(const CAN_message *a, const CAN_message *b) {
    if (a->id != b->id || a->length != b->length) return 0;
    for (uint8_t i = 0; i < a->length; i++) {
        if (a->data[i] != b->data[i]) return 0;
    }
    return 1;
}

// Venter til RX0IF er satt, med timeout. Returnerer 1 hvis melding kom.
static uint8_t wait_rx(void) {
    for (uint16_t i = 0; i < 1000; i++) {
        if (mpc2515_read(MCP_CANINTF) & MCP_RX0IF) return 1;
    }
    return 0;
}

// Sender en melding og leser den tilbake. Returnerer 1 hvis noe ble mottatt.
static uint8_t loopback(const CAN_message *tx, CAN_message *rx) {
    rx->id = 0;   // CAN_recieve_message bruker |= paa id, saa den maa nullstilles
    rx->length = 0;
    CAN_send_message(tx);
    if (!wait_rx()) return 0;
    return CAN_recieve_message(rx);
}

/* ---------------- Engangstester ---------------- */

static void test_register_rw(void) {
    const uint8_t patterns[] = {0x00, 0xFF, 0xAA, 0x55};
    uint8_t ok = 1;
    for (uint8_t i = 0; i < sizeof(patterns); i++) {
        mpc2515_write(MCP_TXB1D0, patterns[i]);
        uint8_t r = mpc2515_read(MCP_TXB1D0);
        if (r != patterns[i]) {
            printf("   skrev 0x%02x, leste 0x%02x\r\n", patterns[i], r);
            ok = 0;
        }
    }
    report("1. SPI skriv/les register", ok);
}

static void test_mode(void) {
    uint8_t canstat = mpc2515_read(MCP_CANSTAT);
    uint8_t ok = (canstat & MODE_MASK) == MODE_LOOPBACK;
    if (!ok) printf("   CANSTAT=0x%02x (forventet 0x4X)\r\n", canstat);
    report("2. Loopback-modus", ok);
}

static void test_bit_modify(void) {
    uint8_t before = mpc2515_read(MCP_CANINTE);

    mpc2515_bit_modify(MCP_CANINTE, 0x80, 0xFF);   // data utenfor masken skal ignoreres
    uint8_t set = mpc2515_read(MCP_CANINTE);

    mpc2515_bit_modify(MCP_CANINTE, 0x80, 0x00);
    uint8_t cleared = mpc2515_read(MCP_CANINTE);

    uint8_t ok = (set == (before | 0x80)) && (cleared == (before & ~0x80));
    if (!ok) printf("   foer=0x%02x satt=0x%02x nullstilt=0x%02x\r\n", before, set, cleared);
    report("3. Bit Modify", ok);
}

static void test_read_status(void) {
    CAN_message tx = {.id = 0x100, .length = 1, .data = {0x99}};
    CAN_message rx;

    CAN_send_message(&tx);
    uint8_t got = wait_rx();
    uint8_t status_before = mpc2515_read_status();

    rx.id = 0;
    CAN_recieve_message(&rx);
    uint8_t status_after = mpc2515_read_status();

    uint8_t ok = got && (status_before & 0x01) && !(status_after & 0x01);
    if (!ok) printf("   status foer=0x%02x etter=0x%02x\r\n", status_before, status_after);
    report("4. Read Status (RX0IF)", ok);
}

static void test_fixed_messages(void) {
    const CAN_message tests[] = {
        {.id = 0x000, .length = 0},
        {.id = 0x7FF, .length = 8, .data = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
        {.id = 0x555, .length = 8, .data = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA}},
        {.id = 0x2AA, .length = 1, .data = {0x42}},
        {.id = 0x123, .length = 8, .data = {1, 2, 3, 4, 5, 6, 7, 8}},
        {.id = 0x004, .length = 4, .data = {0x9C, 0x64, 0x00, 0x80}},   // -100, 100, 0, -128
    };
    const uint8_t n = sizeof(tests) / sizeof(tests[0]);
    uint8_t ok = 1;

    for (uint8_t i = 0; i < n; i++) {
        CAN_message rx;
        if (!loopback(&tests[i], &rx)) {
            printf("   melding %u: ingenting mottatt\r\n", i);
            ok = 0;
        } else if (!messages_equal(&tests[i], &rx)) {
            printf("   melding %u: ulik\r\n", i);
            print_msg("sendt:  ", &tests[i]);
            print_msg("mottatt:", &rx);
            ok = 0;
        }
    }
    report("5. Faste testmeldinger (ID, lengde, data)", ok);
}

static void test_interrupt(void) {
    CAN_message tx = {.id = 0x321, .length = 2, .data = {0xDE, 0xAD}};
    CAN_message rx;

    can_flag = 0;
    CAN_send_message(&tx);
    wait_rx();
    _delay_us(50);
    uint8_t flag = can_flag;

    rx.id = 0;
    CAN_recieve_message(&rx);   // tom bufferet slik at INT gaar hoy igjen

    if (!flag) printf("   ingen avbrudd. Sjekk INT-pinnen (MCP2515 pinne 12) -> PD2\r\n");
    report("6. Avbrudd paa INT0 (PD2)", flag);
}

static void test_no_message(void) {
    CAN_message rx;
    rx.id = 0;
    uint8_t got = CAN_recieve_message(&rx);
    report("7. Ingen ny melding -> receive returnerer 0", got == 0);
}

/* ---------------- main ---------------- */

int main(void)
{
    MCUCR |= (1 << SRE);
    SFIOR |= (1 << XMM2);

    uartInit();
    fdevopen(uart_putchar, NULL);
    adcInit();
    joystickCalibrate();

    spiInit();
    menuInit();     // initialiserer ogsaa OLED
    CAN_init();

    printf("\r\n===== CAN-test (loopback) =====\r\n");
    printf("CANSTAT: 0x%02x  CANCTRL: 0x%02x\r\n",
           mpc2515_read(MCP_CANSTAT), mpc2515_read(MCP_CANCTRL));

    test_register_rw();
    test_mode();
    test_bit_modify();
    test_read_status();
    test_fixed_messages();
    test_interrupt();
    test_no_message();

    printf("===== %u OK, %u FEIL =====\r\n\r\n", tests_ok, tests_fail);

    /* -------- Kontinuerlig test med joystick og slider -------- */

    CAN_message msg = {.id = 0x04, .length = 4};
    CAN_message rx;
    uint16_t sent = 0, ok_count = 0;

    while (1) {
        // OLED-trafikk mellom CAN-operasjonene tester at SPI-bussen deles riktig
        checkMenuPos();
        drawCross();

        JoystickPosition joy = getJoystickPosition();
        JoystickPosition slider = getSliderPosition();

        msg.data[0] = (uint8_t)joy.x;
        msg.data[1] = (uint8_t)joy.y;
        msg.data[2] = (uint8_t)slider.x;
        msg.data[3] = (uint8_t)slider.y;

        Buttons btns = readButtons();
        setLed(3, btns.R5);

        sent++;
        uint8_t got = loopback(&msg, &rx);
        uint8_t ok = got && messages_equal(&msg, &rx);
        if (ok) ok_count++;

        printf("sendt   joyX:%4d joyY:%4d slX:%4d slY:%4d\r\n",
               joy.x, joy.y, slider.x, slider.y);
        if (got) {
            printf("mottatt joyX:%4d joyY:%4d slX:%4d slY:%4d  id=0x%03x  %s  (%u/%u OK)\r\n\r\n",
                   (int8_t)rx.data[0], (int8_t)rx.data[1],
                   (int8_t)rx.data[2], (int8_t)rx.data[3],
                   rx.id, ok ? "OK" : "FEIL", ok_count, sent);
        } else {
            printf("mottatt: INGENTING  (%u/%u OK)\r\n\r\n", ok_count, sent);
        }

        _delay_ms(200);
    }
}
