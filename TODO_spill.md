# Ping pong-spill: status og hva som gjenstår

Ingenting er committet. Alt ligger som endringer på branchen `claude_sjef`.

## Ferdig

### Felles
- `common/protocol.h`: CAN-protokollen.
  - CONTROL `0x10` går fra node 1 til node 2 hvert 20. ms. Den har med racket, vinkel, skyt, modus, spill-id, liv og nivå.
  - STATUS `0x20` går fra node 2 til node 1 hvert 100. ms og straks ved endring. Den har med tilstand, liv, poeng, IR og motorposisjon.

### Node 2 (bygger uten advarsler)
- `game.c/.h` (ny): tilstandsmaskin med tilstandene IDLE, WAIT_BALL, RUNNING, PAUSED og GAME_OVER.
  - Mister du ballen, går du ned ett liv.
  - Det kommer poeng hvert 100. ms, ganget med nivået.
  - Spillet går over i pause hvis meldingene fra node 1 stopper i 300 ms.
- `IR.c`: filtrert balldeteksjon (`ir_update`). Den gamle score-koden er fjernet. Score-variablene var uint8 og rant over.
- `motor.c`:
  - Solenoiden blokkerer ikke lenger. Før stoppet den alt i 100 ms.
  - PID-en har fått fartsgrense og nullstilling (`motor_pid_reset`), og `motor_get_pos` er lagt til.
  - Klokka til PIOC er slått på.
  - Servopulsen er begrenset til 0,9–2,1 ms. Før gikk den opp til 2,23 ms.
- `can.c`: `can_tx` blokkerer ikke og sender standard 11-bit-ID, sånn at MCP2515 tar imot.
- `time.c`: `now` er gjort volatile, og lesingen tåler avbrudd.
- `main.c`: ryddet, og printf-spammen i 20 ms-løkka er borte.
- `Makefile`: `-O2 -Wall`, `game.c` og `-I ../common` er lagt til.

### Node 1 (skrevet, men IKKE kompilert ennå)
- `spi.c`: egen SPI-hastighet per enhet. IO-kortet kjører på fosc/16, OLED og CAN på fosc/4.
- `mcp2515_driver.c`: `mcp2515_load_tx0` og `mcp2515_read_rx` gjør hele bufferen i én SPI-overføring.
- `CAN_communication.c`: sender i én omgang, tar imot fra RXB0 og RXB1 og har timeout når node 2 er av. Ny funksjon: `CAN_tx_errors()`.
- `oled.c/.h`: skriver i bolker, har æøå og ikoner (hjerte og piler), invertert tekst og kontrast.
- `screen.c/.h` (ny): tekstbuffer på 8x21 tegn som bare tegner om de linjene som er endret.
- `timer.c/.h` (ny): Timer0 gir et tick på nøyaktig 20 ms, og CPU-en sover mellom tickene.
- `input.c/.h` (ny): tastehendelser med autorepetisjon.
  - Navigering: joystick og nav-bryteren.
  - OK: joystick-knappen, R5 eller nav-knappen.
  - Skyt: L2.
- `storage.c/.h` (ny): innstillinger og topp 5 lagres i EEPROM med CRC.
- `adc.c`: `getAnalogInputs` leser alt på én gang. Slideren er lineær over hele lengden, og joysticken har dødsone.
- `game.c/.h` (ny): sender CONTROL og tolker STATUS. Har to styringsmoduser: slider og joystick.
- `menu.c/.h` (skrevet om): datadrevne menyer.
  - Hovedmeny: Start spill, Trening, Toppliste, Innstillinger, Diagnostikk, Hjelp.
  - Innstillinger: Liv, Nivå, Styring, Snu vinkel, Lysstyrke, Slett toppliste.
  - I tillegg en pausemeny og en bekreft-meny.

## Gjenstår (node 1 bygger IKKE nå)

1. **`app.c/.h`**: tilstandsmaskin for skjermene. Bruk `menuUpdate`/`menuDraw` og tegn linje 7 selv.
   - MENU:
     - Hovedmenyen har bunnlinjen `Siste: X  Best: Y`. Best er `storage.hs[0].score`.
     - `ACT_SETTING_CHANGED` skal kalle `oledSetContrast(tabell[brightness-1])`, for eksempel med `{0x08,0x30,0x70,0xB0,0xFF}`.
     - `ACT_CLEAR_HS` skal kalle `highscoreClear()` og så `menuOpen(MENU_SETTINGS)`.
   - GAME:
     - Vis poeng, hjerter (`GLYPH_HEART`/`GLYPH_HEART_O`) og en statustekst ut fra `gameStatus()->state`.
     - Vis «Venter på node 2...» når `!gameOnline()`.
     - Vis «Ball tapt!» når `lives` endrer seg.
     - KEY_OK skal kalle `gamePause()` og åpne `MENU_PAUSE`.
     - Når `state == GS_GAME_OVER` og `gameCurrent()` er sann, gå til GAMEOVER.
   - PAUSE: `ACT_RESUME` skal kalle `gameResume()`, og `ACT_QUIT` skal kalle `gameStop()` og gå til hovedmenyen.
   - GAMEOVER:
     - Lagre poengsummen og kjør `highscoreInsert(score, level)` hvis det ikke er trening.
     - Kall `gameStop()`.
     - Vis «Ny rekord!» eller plassen på listen.
   - HIGHSCORES, HELP og DIAG: enkle skjermer.
     - DIAG viser joystick, slider, knapper (hex), node 2-status, IR, motorposisjon og `CAN_tx_errors()`.
     - Gå ut med OK, eller ved å holde venstre i 2 s.
2. **`main.c`**: skriv om. Den kaller fortsatt de gamle `menuInit`, `checkMenuPos` og `drawCross`, som ikke finnes lenger.
   - Init-rekkefølge: XMEM (SRE/XMM2), `uartInit`, `stdout` via `FDEV_SETUP_STREAM` (ikke `fdevopen`), `inputInit`, `adcInit`, `joystickCalibrate`, `spiInit`, `storageLoad`, `oledInit` + kontrast, `CAN_init`, `timerInit`, `appInit`.
   - Løkke: `timerWaitTick()`, `inputUpdate()`, `while (CAN_recieve_message(&m)) gameOnMessage(&m);`, `appUpdate()`, `gameTick(inputState())`, `screenFlush()`.
3. **`node1/Makefile`**:
   - `SOURCE_FILES` må få `screen.c timer.c input.c storage.c game.c app.c`.
   - `CFLAGS := -Os -std=gnu11 -mmcu=$(TARGET_CPU) -ggdb -Wall -DF_CPU=4915200UL -I../common -ffunction-sections -fdata-sections`, og legg til `-Wl,--gc-sections` ved linking.
   - **Viktig:** `CAN_communication.c`, `timer.c` og `oled.c` definerer ikke lenger `F_CPU` selv. Den må komme fra Makefile. Fjern også `#define F_CPU` i de andre filene, ellers blir det advarsler om at den er definert to ganger.
   - Bygg til en annen mappe, for eksempel `make BUILD_DIR=/tmp/n1`, fordi `node1/build/` er sporet i git.
4. Småfiks:
   - `menu.c` mangler `#include <string.h>`.
   - `UART.h` trenger `#include <stdio.h>`, fordi den bruker `FILE`.
   - I `menu.c` teller `strlen` bytes, ikke tegn, så «På» blir én kolonne for bred. Bruk en tegnteller eller skriv «Ja/Nei».
5. Bygg begge nodene, sjekk størrelsen (`avr-size`, maks 16 KB flash og 1 KB RAM) og test på hardware:
   - Node 2 sender nå standard-ID. Sjekk at node 1 tar imot STATUS.
   - Sjekk at nav-bitene (NB/NU/ND/NL/NR) i `io.h` stemmer. Hvis ikke, sett `USE_NAV_SWITCH 0` i `input.c`.
   - Sjekk IR-terskelen (1000) og filteret (3 tick).
   - Hvis OLED eller CAN oppfører seg rart på fosc/4, sett tilbake til `SPCR_DIV16` i `spi.c`.

## Merk
- `node1/notater.md` inneholder et GitHub-token i klartekst. Fila er gitignored, men tokenet bør trekkes tilbake og fjernes.
