/* ##########################################################################
 *               FOCUSQUEST: EL334 MINI-PROJECT requirements overview
 * ##########################################################################
 * Reference: project requirements and scoring rubric, page 3 of 1 Mini_Project.pdf
 * REQ / SCORE labels below are code search markers, not official document item numbers
 *
 * ========================= MANDATORY REQUIREMENTS ===========================
 * [REQ-1] TIMER / COUNTER: at least one operation
 *         SECTION 14: timer_init() configures PIT0 at 1 ms intervals
 *         SECTION 13: PIT0_IRQHandler() increments the millisecond counter
 *         SECTION 04: app_update() decreases the countdown using elapsed time
 *         The timer is not used to control the FRDM-K64F onboard RGB LED
 *
 * [REQ-2] INTERRUPT: at least one operation
 *         SECTION 13: PORTB_IRQHandler() handles button events
 *                      PIT0_IRQHandler() provides timing and display scanning
 *         SECTIONS 10 and 14: configure and enable interrupts
 *
 * [REQ-3] Separate device operations into individual functions
 *         SECTION 05: buttons_update() - process pushbuttons
 *         SECTION 09: buzzer_set() - control the buzzer
 *         SECTION 11: display_write() - send data to the 7-segment display
 *         SECTION 12: leds_write() - indicate completed rounds using LEDs
 *         SECTION 15: lcd_init()/lcd_service() - initialize and update LCD
 *         SECTION 16: main() calls the individual functions in sequence
 *         All functions can reside in a single .c file; separate source files are optional
 *
 * ========================== SCORING CATEGORIES ==============================
 * [SCORE-SWITCH] SECTION 05: pushbuttons S1-S3
 * [SCORE-ONOFF] SECTIONS 09 and 12: buzzer on/off and LEDs
 * [SCORE-7SEG] SECTIONS 06 and 11: four-digit 7-segment display
 * [SCORE-LCD] SECTIONS 07 and 15: optional external LCD
 * Note: the buzzer uses simple on/off beeps, not a multi-tone melody
 * These markers identify code evidence; they do not guarantee instructor marks
 *
 * ========================== PROJECT FEATURES ================================
 * SECTION 03: history of four rounds stored in RAM
 * SECTION 04: work/break cycles, pause/resume, and time adjustment based on review
 * These features are group design choices, not extra mandatory requirements
 *
 * Additional submissions (page 4): Word report with diagrams/flowcharts,
 * source code and documents on GitHub with repository link in the report, plus presentation/Q&A
 * Group requirement (page 3): two members; larger groups need permission and extra conditions
 *
 * This submission adds a debounced keypress beep; other application and hardware logic is preserved
 * ########################################################################## */

/* FOCUSQUEST - single-file function guide (English comments)
 * Only descriptions are changed; executable code and pin assignments stay the same
 * Search for "SECTION nn" or "FUNCTION: function_name" in Keil to navigate
 * All functions are in main.c; save the file as UTF-8
 *
 * 01 Initial configuration
 * 02 Program data and states
 * 03 Four-round history
 * 04 Countdown and state transitions
 * 05 Button event processing
 * 06 7-segment number formatting
 * 07 LCD text formatting
 * 08 Hardware pins and interrupt-shared data
 * 09 Buzzer control
 * 10 GPIO initialization
 * 11 Sending data to the 7-segment display
 * 12 LEDs for completed rounds
 * 13 Interrupt service routines
 * 14 PIT timer configuration
 * 15 LCD communication using software I2C
 * 16 Main function and task orchestration
 */

/* FocusQuest - FRDM-K64F + Arduino Multi-Function Shield
 * Keil uVision / CMSIS MK64F12.h / C99. English comments; UTF-8 encoding.
 * BUZZER FIX: off = PTA1 input without pulls; on = PTA1 output LOW.
 * Do not use hardware open-drain with a 5 V pull-up on K64.
 * LCD + FOUR-ROUND RAM HISTORY edition. See README.md in the ZIP package.
 * REV-E PIN FIX: original Arduino D8 = PTC12 (verified for Rev E3).
 * Use a working K64F project with startup and system_MK64F12.c.
 * Replace the old main.c; do not compile duplicate main or IRQ handlers.
 * Uses PIT0, GPIO interrupts and software I2C; no Arduino library or RTOS.
 * Check voltage compatibility and pin mapping before connecting hardware.
 * Original stacked shield wiring. No D6 data jumper is required.
 * Older Rev D1 maps D8 to PTA0 instead. Confirm board revision.
 * No ULINK2/ME Device found is a debug-probe detection error, not a font error. */
#include <stdint.h>
#ifndef FOCUSQUEST_HOST_TEST
#include "MK64F12.h"
/* Global interrupt entry points used by the startup vector table.
 * Declare prototypes to avoid missing-prototype warnings. Do not make them static.
 */
void PORTB_IRQHandler(void);
void PIT0_IRQHandler(void);

/* Support old headers (PTA) and new headers (GPIOA). */
#ifndef PTA
#define PTA GPIOA
#define PTB GPIOB
#define PTC GPIOC
#define PTD GPIOD
#define PTE GPIOE
#endif
#endif

/* ==========================================================================
 * SECTION 01 | Initial configuration
 * Set demo-mode timing, buzzer pattern, LCD dimensions, and button debounce period
 * ========================================================================== */
#ifndef DEMO_MODE
#define DEMO_MODE 1          /* 1 = short demo times; 0 = normal minute-based times */
#endif
#define BUZZER_IS_ACTIVE 1   /* 1 = active buzzer; 0 = passive buzzer (500 Hz) */
#define BUZZER_ENABLED   1   /* Set to 0 for silent testing */

/* LCD: HD44780 with a common PCF8574 backpack.
 * Set 16/2 for a 1602 LCD, or 20/4 for a 2004 LCD.
 * Backpack map: P0=RS, P1=RW, P2=EN, P3=backlight, P4..P7=D4..D7.
 */
#ifndef LCD_COLS
#define LCD_COLS 16U
#endif
#ifndef LCD_ROWS
#define LCD_ROWS 2U
#endif
#if !((LCD_COLS == 16 && LCD_ROWS == 2) || (LCD_COLS == 20 && LCD_ROWS == 4))
#error Supported LCD layouts are 16x2 and 20x4.
#endif
#define LCD_I2C_ADDRESS 0U /* 0=scan PCF8574 ranges; otherwise a 7-bit address */

#define BUTTON_DEBOUNCE_MS 25U
#define BUTTON_LONG_MS     1200U

/* New: one audible click per debounced physical S1/S2/S3 press.
 * A short quiet gap before/after each beep makes it distinguishable from
 * the original countdown alert sounds, without blocking the main loop.
 */
#define KEYPRESS_BEEP_LEAD_MS  15U
#define KEYPRESS_BEEP_ON_MS    60U
#define KEYPRESS_BEEP_TAIL_MS  25U
#define KEYPRESS_BEEP_TOTAL_MS (KEYPRESS_BEEP_LEAD_MS + \
                                KEYPRESS_BEEP_ON_MS + \
                                KEYPRESS_BEEP_TAIL_MS)

#if DEMO_MODE
#define FOCUS_DEFAULT_S 20U
#define FOCUS_MIN_S     10U
#define FOCUS_MAX_S     40U
#define FOCUS_STEP_S     5U
#define BREAK_DEFAULT_S  5U
#define BREAK_MIN_S      1U
#define BREAK_MAX_S     15U
#define BREAK_STEP_S     1U
#define LONG_BREAK_S    15U
#else
#define FOCUS_DEFAULT_S (25U * 60U)
#define FOCUS_MIN_S     (10U * 60U)
#define FOCUS_MAX_S     (40U * 60U)
#define FOCUS_STEP_S     (5U * 60U)
#define BREAK_DEFAULT_S  (5U * 60U)
#define BREAK_MIN_S      (1U * 60U)
#define BREAK_MAX_S     (30U * 60U)
#define BREAK_STEP_S     (1U * 60U)
#define LONG_BREAK_S    (15U * 60U)
#endif

/* ==========================================================================
 * SECTION 02 | Program data and states
 * Define button events, application states, system variables, and history storage
 * ========================================================================== */
#define EV_S1      1U
#define EV_S2      2U
#define EV_S3      4U
#define EV_S3_LONG 8U

typedef enum {
    ST_READY, ST_FOCUS, ST_RATE, ST_BREAK, ST_SET_FOCUS, ST_SET_BREAK
} State;

typedef struct {
    State state;
    uint32_t focus_s, break_s, remaining_ms, last_ms;
    uint32_t show_rounds_ms, sound_ms;
    uint8_t rounds, paused;
} App;

typedef struct {
    uint32_t changed_ms, pressed_ms;
    uint8_t raw, stable, armed, long_sent;
} Button;

static App app;
static Button buttons[3];
/* Reloaded by buttons_update(), decremented by the 1-ms PIT0 interrupt. */
static volatile uint16_t keypress_beep_ticks;

typedef enum { REC_EMPTY, REC_ACTIVE, REC_DONE, REC_CANCELLED } RecordStatus;
typedef struct {
    uint32_t target_ms;
    uint32_t used_ms;
    uint32_t left_ms;
    RecordStatus status;
} RoundRecord;

/* RAM only. Slots correspond to LED D1, D2, D3, D4 in the current set. */
static RoundRecord history[4];
static uint8_t history_view;
static uint8_t history_index;
static char lcd_text[LCD_ROWS][LCD_COLS];

/* ==========================================================================
 * SECTION 03 | Four-round history
 * Clear, start, record, and aggregate the history of four rounds in RAM
 * ========================================================================== */
/* FUNCTION: history_clear
 * Purpose: clear all four RAM history slots when beginning a new session
 */
static void history_clear(void)
{
    uint32_t i;
    for (i = 0U; i < 4U; ++i) {
        history[i].target_ms = 0U;
        history[i].used_ms = 0U;
        history[i].left_ms = 0U;
        history[i].status = REC_EMPTY;
    }
    history_view = 0U;
    history_index = 0U;
}

/* FUNCTION: history_begin
 * Purpose: capture the configured duration when a work round starts
 */
static void history_begin(void)
{
    RoundRecord *r;
    if (app.rounds >= 4U) return;
    r = &history[app.rounds];
    r->target_ms = app.focus_s * 1000U;
    r->used_ms = 0U;
    r->left_ms = r->target_ms;
    r->status = REC_ACTIVE;
}

/* FUNCTION: history_progress
 * Purpose: save elapsed and remaining time for the current round
 */
static void history_progress(void)
{
    RoundRecord *r;
    if (app.rounds >= 4U) return;
    r = &history[app.rounds];
    if (r->status != REC_ACTIVE) return;
    r->left_ms = app.remaining_ms;
    r->used_ms = r->target_ms - r->left_ms;
}

/* FUNCTION: history_total_done_ms
 * Purpose: sum time for completed rounds only; exclude breaks and canceled rounds
 */
static uint32_t history_total_done_ms(void)
{
    uint32_t i, total = 0U;
    for (i = 0U; i < 4U; ++i)
        if (history[i].status == REC_DONE) total += history[i].used_ms;
    return total;
}


/* These functions do not access hardware registers and can be tested on a PC. */
/* ==========================================================================
 * SECTION 04 | Countdown and state transitions
 * Calculate time remaining; start, pause, evaluate, and manage break intervals
 * ========================================================================== */
/* FUNCTION: subtract_floor
 * Purpose: subtract with saturation at zero to prevent unsigned wraparound
 */
static uint32_t subtract_floor(uint32_t value, uint32_t amount)
{
    return value > amount ? value - amount : 0U;
}

/* FUNCTION: adjust_value
 * Purpose: adjust a configured value while enforcing minimum and maximum limits
 */
static uint32_t adjust_value(uint32_t value, uint32_t step,
                             uint32_t minimum, uint32_t maximum, int up)
{
    if (up) return value + step > maximum ? maximum : value + step;
    return value < minimum + step ? minimum : value - step;
}

/* FUNCTION: app_init
 * Purpose: enter READY state and clear the session history
 */
static void app_init(uint32_t now)
{
    App initial = { ST_READY, FOCUS_DEFAULT_S, BREAK_DEFAULT_S,
                    0U, 0U, 0U, 0U, 0U, 0U };
    app = initial;
    history_clear();
    app.remaining_ms = app.focus_s * 1000U;
    app.last_ms = now;
}

/* Use the current timestamp instead of delay(), keeping buttons and display responsive. */
/* FUNCTION: app_update
 * Purpose: decrease the active interval according to elapsed milliseconds, excluding pauses
 */
static void app_update(uint32_t now)
{
    uint32_t elapsed = now - app.last_ms; /* Unsigned subtraction handles counter wraparound. */
    app.last_ms = now;
    app.sound_ms = subtract_floor(app.sound_ms, elapsed);
    app.show_rounds_ms = subtract_floor(app.show_rounds_ms, elapsed);
    if ((app.state != ST_FOCUS && app.state != ST_BREAK) || app.paused)
        return;

    app.remaining_ms = subtract_floor(app.remaining_ms, elapsed);
    if (app.state == ST_FOCUS) history_progress();
    if (app.remaining_ms != 0U) return;

    app.show_rounds_ms = 0U;
    if (app.state == ST_FOCUS) {
        if (app.rounds < 4U) {
            history[app.rounds].status = REC_DONE;
            ++app.rounds;
        } /* Freeze this record before feedback changes the next duration. */
        app.state = ST_RATE;
        app.sound_ms = app.rounds == 4U ? 1000U : 600U;
    } else {
        app.state = ST_READY; /* Wait for the user after a break; do not start work automatically. */
        app.remaining_ms = app.focus_s * 1000U;
        app.sound_ms = 400U;
    }
}

/* FUNCTION: app_on_event
 * Purpose: handle one button command according to the current screen or state
 */
static void app_on_event(uint32_t event)
{
    uint32_t *setting;
    if (history_view) {
        if (event == EV_S1) history_index = (uint8_t)((history_index + 3U) % 4U);
        else if (event == EV_S2) history_index = (uint8_t)((history_index + 1U) % 4U);
        else if (event == EV_S3 || event == EV_S3_LONG) history_view = 0U;
        return;
    }
    if (event == EV_S3_LONG) {
        if (app.state == ST_READY || app.state == ST_RATE) {
            history_view = 1U;
            history_index = app.rounds ? (uint8_t)(app.rounds - 1U) : 0U;
            return;
        }
        if (app.state == ST_FOCUS && app.rounds < 4U) {
            history_progress();
            history[app.rounds].status = REC_CANCELLED;
        }
        if (app.state == ST_FOCUS || app.state == ST_BREAK) {
            app.state = ST_READY;
            app.paused = 0U;
            app.remaining_ms = app.focus_s * 1000U;
            app.show_rounds_ms = 0U;
            app.sound_ms = 150U;
        }
        return; /* A long press cancels the current round but preserves completed rounds. */
    }

    if (app.state == ST_READY) {
        if (event == EV_S1) {
            if (app.rounds == 4U) {
                app.rounds = 0U;
                history_clear(); /* Keep all four records until a new set starts. */
            }
            history_begin(); /* Retrying a cancelled slot replaces that attempt. */
            app.state = ST_FOCUS;
            app.paused = 0U;
            app.remaining_ms = app.focus_s * 1000U;
            app.sound_ms = 100U;
        } else if (event == EV_S2) app.state = ST_SET_FOCUS;
        else if (event == EV_S3) app.state = ST_SET_BREAK;
    } else if (app.state == ST_SET_FOCUS || app.state == ST_SET_BREAK) {
        setting = app.state == ST_SET_FOCUS ? &app.focus_s : &app.break_s;
        if (event == EV_S1 || event == EV_S2) {
            if (app.state == ST_SET_FOCUS)
                *setting = adjust_value(*setting, FOCUS_STEP_S, FOCUS_MIN_S,
                                        FOCUS_MAX_S, event == EV_S2);
            else
                *setting = adjust_value(*setting, BREAK_STEP_S, BREAK_MIN_S,
                                        BREAK_MAX_S, event == EV_S2);
        } else if (event == EV_S3) {
            app.state = ST_READY;
            app.remaining_ms = app.focus_s * 1000U;
        }
    } else if (app.state == ST_RATE) {
        /* S1 = too long; S2 = just right; S3 = increase. These are rules, not attention measurements. */
        if (event == EV_S1 || event == EV_S3)
            app.focus_s = adjust_value(app.focus_s, FOCUS_STEP_S,
                                        FOCUS_MIN_S, FOCUS_MAX_S, event == EV_S3);
        if (event == EV_S1 || event == EV_S2 || event == EV_S3) {
            app.state = ST_BREAK;
            /* The fourth-round break must not be shorter than the configured normal break. */
            app.remaining_ms = ((app.rounds == 4U && LONG_BREAK_S > app.break_s)
                                ? LONG_BREAK_S : app.break_s) * 1000U;
            app.paused = 0U;
            app.sound_ms = 100U;
        }
    } else { /* ST_FOCUS or ST_BREAK */
        if (event == EV_S1) {
            app.paused = (uint8_t)!app.paused; /* Preserve the remaining fractional milliseconds. */
            app.sound_ms = 80U;
        } else if (event == EV_S2) app.show_rounds_ms = 2000U;
    }
}

/* A GPIO interrupt arms each press; require 25 ms of stable input.
 * S1/S2 act on press. A short S3 action occurs on release to distinguish a long press.
 * Holding a button does not repeat the action. Release before pressing again. */
/* ##########################################################################
 * [SCORE-SWITCH] Pushbutton controls
 * S1-S3 control the system; contact bounce is filtered in software
 * ########################################################################## */
/* ==========================================================================
 * SECTION 05 | Button event processing
 * Debounce buttons and detect short and long presses after GPIO interrupts
 * ========================================================================== */
/* FUNCTION: buttons_update
 * Purpose: return a debounced button event, including a long S3 press
 */
static uint32_t buttons_update(uint32_t now, uint32_t pressed, uint32_t edges)
{
    uint32_t events = 0U, i, mask;
    uint8_t raw;
    Button *b;
    for (i = 0U; i < 3U; ++i) {
        b = &buttons[i];
        mask = 1U << i;
        raw = (uint8_t)((pressed & mask) != 0U);
        if ((edges & mask) && !b->stable) b->armed = 1U;
        if (raw != b->raw) {
            b->raw = raw;
            b->changed_ms = now;
        }
        if (b->stable != b->raw && now - b->changed_ms >= BUTTON_DEBOUNCE_MS) {
            b->stable = b->raw;
            if (b->stable) {
                b->pressed_ms = now;
                b->long_sent = 0U;
                /* Beep on each confirmed press, including S3. S3 is still
                 * handled on release/hold as in the original application.
                 * No repeat sounds while the button stays held down.
                 */
                if (b->armed) keypress_beep_ticks = KEYPRESS_BEEP_TOTAL_MS;
                if (b->armed && i < 2U) events |= mask;
            } else {
                if (b->armed && i == 2U && !b->long_sent) events |= EV_S3;
                b->armed = 0U;
            }
        }
        if (i == 2U && b->raw && b->stable && b->armed && !b->long_sent &&
            now - b->pressed_ms >= BUTTON_LONG_MS) {
            b->long_sent = 1U;
            events |= EV_S3_LONG;
        }
    }
    return events;
}

/* Active-low: 0 lights a segment. Bit order: DP,G,F,E,D,C,B,A. */
/* ==========================================================================
 * SECTION 06 | Formatting numbers for the 7-segment display
 * Convert time and status to four-digit segment patterns
 * ========================================================================== */
static const uint8_t segment[10] = {
    0xC0U, 0xF9U, 0xA4U, 0xB0U, 0x99U,
    0x92U, 0x82U, 0xF8U, 0x80U, 0x90U
};

/* FUNCTION: pack_display
 * Purpose: pack four segment bytes into one 32-bit display value
 */
static uint32_t pack_display(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return (uint32_t)a | ((uint32_t)b << 8) |
           ((uint32_t)c << 16) | ((uint32_t)d << 24);
}

/* FUNCTION: display_time
 * Purpose: convert time to MM.SS on the four-digit display
 */
static uint32_t display_time(uint32_t seconds, int dot)
{
    uint32_t minutes = seconds / 60U;
    uint8_t b = segment[minutes % 10U];
    if (dot) b &= 0x7FU; /* Decimal point after digit 2: MM.SS */
    return pack_display(segment[(minutes / 10U) % 10U], b,
                        segment[(seconds % 60U) / 10U], segment[seconds % 10U]);
}

/* FUNCTION: app_display
 * Purpose: choose a time or status message for the 7-segment display
 */
static uint32_t app_display(uint32_t now)
{
    uint32_t seconds;
    if (app.state == ST_RATE)
        return pack_display(0xAFU, 0x88U, 0x87U, 0x86U); /* rAtE */
    if (app.show_rounds_ms != 0U)
        return pack_display(0xC6U, 0xFFU, 0xFFU, segment[app.rounds]); /* C  n */
    if (app.paused && (now / 500U) % 2U)
        return 0xFFFFFFFFU; /* Blink while paused; the ISR continues scanning the display. */

    /* Briefly show a mode label on the four-digit display. */
    if (now % 2000U < 400U) {
        if (app.state == ST_SET_FOCUS)
            return pack_display(0x8EU, 0xBFU, 0xBFU, 0xBFU); /* F--- */
        if (app.state == ST_SET_BREAK)
            return pack_display(0x83U, 0xBFU, 0xBFU, 0xBFU); /* b--- */
        if (app.state == ST_BREAK && !app.paused)
            return pack_display(0x83U, 0xFFU, 0xFFU, 0xFFU); /* b */
    }
    if (app.state == ST_SET_FOCUS || app.state == ST_READY) seconds = app.focus_s;
    else if (app.state == ST_SET_BREAK) seconds = app.break_s;
    else seconds = (app.remaining_ms + 999U) / 1000U; /* Round upward so the first second does not disappear immediately. */
    return display_time(seconds, 1);
}

/* ==========================================================================
 * SECTION 07 | LCD text formatting
 * Prepare time, settings, and history text in memory before displaying
 * ========================================================================== */
/* FUNCTION: lcd_put
 * Purpose: write into the LCD buffer without exceeding row or column bounds
 */
static void lcd_put(uint32_t row, uint32_t col, const char *s)
{
    if (row >= LCD_ROWS) return;
    while (*s && col < LCD_COLS) lcd_text[row][col++] = *s++;
}

/* FUNCTION: lcd_digit
 * Purpose: write one decimal digit into the LCD buffer
 */
static void lcd_digit(uint32_t row, uint32_t col, uint32_t n)
{
    if (row < LCD_ROWS && col < LCD_COLS)
        lcd_text[row][col] = (char)('0' + (n % 10U));
}

/* Individual durations fit MM:SS; session totals use MMM:SS if needed. */
/* FUNCTION: lcd_time
 * Purpose: convert milliseconds to minutes:seconds, optionally rounding up or down
 */
static void lcd_time(uint32_t row, uint32_t col, uint32_t ms, uint8_t round_up)
{
    uint32_t s = (ms + (round_up ? 999U : 0U)) / 1000U;
    uint32_t m = s / 60U;
    if (m > 99U) {
        lcd_digit(row, col++, m / 100U);
    }
    lcd_digit(row, col, m / 10U);
    lcd_digit(row, col + 1U, m);
    lcd_put(row, col + 2U, ":");
    lcd_digit(row, col + 3U, (s % 60U) / 10U);
    lcd_digit(row, col + 4U, s % 10U);
}

/* FUNCTION: record_name
 * Purpose: return an English status label for the saved round
 */
static const char *record_name(RecordStatus status)
{
    if (status == REC_ACTIVE) return "RUN";
    if (status == REC_DONE) return "DONE";
    if (status == REC_CANCELLED) return "CANCEL";
    return "EMPTY";
}

/* FUNCTION: lcd_render
 * Purpose: build the LCD screen contents in RAM without accessing hardware
 */
static void lcd_render(uint32_t now)
{
    uint32_t row, col, current;
    const RoundRecord *r;
    for (row = 0U; row < LCD_ROWS; ++row)
        for (col = 0U; col < LCD_COLS; ++col) lcd_text[row][col] = ' ';

    if (history_view) {
        r = &history[history_index];
        lcd_put(0U, 0U, "R");
        lcd_digit(0U, 1U, history_index + 1U);
        lcd_put(0U, 3U, "D");
        lcd_digit(0U, 4U, history_index + 1U);
        lcd_put(0U, 6U, record_name(r->status));
        if (r->status == REC_EMPTY) {
            lcd_put(1U, 0U, "No record yet");
        } else if (LCD_ROWS == 4U || (now / 2500U) % 2U == 0U) {
            lcd_put(1U, 0U, "T");
            lcd_time(1U, 1U, r->target_ms, 0U);
            lcd_put(1U, 7U, "U");
            lcd_time(1U, 8U, r->used_ms, 0U);
        } else {
            lcd_put(1U, 0U, "Left ");
            lcd_time(1U, 5U, r->left_ms, 1U);
        }
        if (LCD_ROWS == 4U) {
            if (r->status != REC_EMPTY) {
                lcd_put(2U, 0U, "Left ");
                lcd_time(2U, 5U, r->left_ms, 1U);
            }
            lcd_put(3U, 0U, "1:Prev 2:Next 3:Back");
        }
        return;
    }

    current = app.rounds < 4U ? app.rounds : 3U;
    r = &history[current];
    if (app.state == ST_READY) {
        lcd_put(0U, 0U, "READY S1:Start");
        lcd_put(1U, 0U, "Done ");
        lcd_digit(1U, 5U, app.rounds);
        lcd_put(1U, 6U, "/4 T");
        lcd_time(1U, 10U, history_total_done_ms(), 0U);
    } else if (app.state == ST_SET_FOCUS || app.state == ST_SET_BREAK) {
        lcd_put(0U, 0U, app.state == ST_SET_FOCUS ? "SET FOCUS" : "SET BREAK");
        lcd_time(1U, 0U, (app.state == ST_SET_FOCUS ? app.focus_s : app.break_s) * 1000U, 0U);
        lcd_put(1U, 6U, "S3:Save");
    } else if (app.state == ST_RATE) {
        lcd_put(0U, 0U, "DONE R");
        lcd_digit(0U, 6U, app.rounds);
        lcd_put(1U, 0U, "1:- 2:= 3:+");
    } else {
        lcd_put(0U, 0U, app.paused ? "PAUSED " : (app.state == ST_FOCUS ? "FOCUS " : "BREAK "));
        lcd_put(0U, 7U, "R");
        lcd_digit(0U, 8U, app.state == ST_FOCUS ? app.rounds + 1U : app.rounds);
        lcd_put(0U, 9U, "/4");
        lcd_put(1U, 0U, "Left ");
        lcd_time(1U, 5U, app.remaining_ms, 1U);
    }
    if (LCD_ROWS == 4U) {
        if (app.state == ST_FOCUS) {
            lcd_put(2U, 0U, "Used ");
            lcd_time(2U, 5U, r->used_ms, 0U);
            lcd_put(2U, 10U, " / ");
            lcd_time(2U, 13U, r->target_ms, 0U);
        } else {
            lcd_put(2U, 0U, "Total ");
            lcd_time(2U, 6U, history_total_done_ms(), 0U);
        }
        if (app.state == ST_READY || app.state == ST_RATE)
            lcd_put(3U, 0U, "Hold S3: History");
        else if (app.state == ST_SET_FOCUS || app.state == ST_SET_BREAK)
            lcd_put(3U, 0U, "S1:- S2:+ S3:Save");
        else lcd_put(3U, 0U, "S1:Pause S3hold:Quit");
    }
}

#ifndef FOCUSQUEST_HOST_TEST
/* ==========================================================================
 * SECTION 08 | Hardware pins and interrupt-shared data
 * Use the pin assignments from the tested version and declare ISR-shared variables
 * ========================================================================== */
#define BIT(n) (1UL << (n))
#define LATCH_MASK BIT(23)   /* D4 -> PTB23 */
#define CLOCK_MASK BIT(3)    /* D7 -> PTC3 */
/* FRDM-K64F Rev E3: original Arduino D8 is PTC12.
 * Keep the shield on its original headers. Do not reroute DATA to D6.
 * PTA0/SWD_CLK is deliberately untouched by this edition.
 */
#define DATA_GPIO PTC
#define DATA_PORT PORTC
#define DATA_PIN  12U
#define DATA_MASK BIT(DATA_PIN)
#define BUZZ_MASK  BIT(1)    /* D3 -> PTA1, active-low through a transistor */
#define BUTTON_MASK (BIT(3) | BIT(10) | BIT(11)) /* A1,A2,A3 -> PTB3,10,11 */
#define LED_MASK 0x0FU      /* D13,D12,D11,D10 -> PTD1,3,2,0 */

static volatile uint32_t milliseconds;
static volatile uint32_t button_edges;
static volatile uint32_t display_frame = 0xFFFFFFFFU;
static volatile uint8_t buzzer_gate;
/* Low nibble selects a digit with HIGH; the other chip drives active-low segments. */
static const uint8_t digit_select[4] = {0xF1U, 0xF2U, 0xF4U, 0xF8U};
static const uint8_t led_pin[4] = {1U, 3U, 2U, 0U};

/* ##########################################################################
 * [SCORE-ONOFF] On/off buzzer alert
 * Control the buzzer via buzzer_set(); this is not a melody feature
 * ########################################################################## */
/* ==========================================================================
 * SECTION 09 | Buzzer control
 * Enable the buzzer with an output LOW; disable it using a high-impedance input
 * ========================================================================== */
/* K64 digital inputs are 5 V tolerant; GPIO outputs (including hardware
 * open-drain outputs) must not be externally pulled above VDD.
 * Therefore OFF uses actual INPUT/Hi-Z with PE=0, not a HIGH output or ODE.
 * ON drives LOW through the shield's existing R9=10k to the Q1 PNP base.
 * Intended only for powered FRDM-K64F + the supplied 5 V 3906 shield circuit.
 * A floating PNP base is not a guaranteed hardware OFF state on every board.
 * If OFF still produces sound, check wiring and a base-emitter pull-up/driver
 * with the lab instructor rather than applying 5 V to an MCU output.
 */
/* FUNCTION: buzzer_set
 * Purpose: drive or release the Shield buzzer transistor control pin
 */
static void buzzer_set(uint8_t on)
{
    if (on) {
        PTA->PCOR = BUZZ_MASK;  /* Set output latch LOW BEFORE enabling output. */
        PTA->PDDR |= BUZZ_MASK;
    } else {
        PTA->PDDR &= ~BUZZ_MASK; /* Disable output driver; leave latch LOW. */
    }
}

/* ==========================================================================
 * SECTION 10 | GPIO configuration
 * Configure Shield pins and enable falling-edge interrupts for pushbuttons
 * ========================================================================== */
/* FUNCTION: gpio_init
 * Purpose: configure Shield GPIO and enable button interrupts
 */
static void gpio_init(void)
{
    uint32_t i;
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK |
                  SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;
    /* Set output levels before enabling outputs to reduce startup glitches. */
    /* Release PTA1 before selecting GPIO: off = input, no internal pulls.
     * This avoids driving 3.3 V against the shield's 5 V PNP base circuit.
     */
    PTA->PDDR &= ~BUZZ_MASK;
    PTA->PCOR = BUZZ_MASK; /* Preload LOW for the next ON command. */
    DATA_GPIO->PCOR = DATA_MASK;
    PTB->PCOR = LATCH_MASK;
    PTC->PCOR = CLOCK_MASK;
    PTD->PSOR = LED_MASK;
    DATA_PORT->PCR[DATA_PIN] = PORT_PCR_MUX(1);
    PORTA->PCR[1] = PORT_PCR_MUX(1);
    PORTB->PCR[23] = PORT_PCR_MUX(1);
    PORTC->PCR[3] = PORT_PCR_MUX(1);
    for (i = 0U; i < 4U; ++i) PORTD->PCR[i] = PORT_PCR_MUX(1);
    DATA_GPIO->PDDR |= DATA_MASK;
    /* Keep PTA1 as an input until an audible alert is requested. */
    PTB->PDDR |= LATCH_MASK;
    PTC->PDDR |= CLOCK_MASK;
    PTD->PDDR |= LED_MASK;
    PTB->PDDR &= ~BUTTON_MASK;

    /* IRQC=10: interrupt on HIGH-to-LOW transition (button press). */
    PORTB->PCR[3] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(10);
    PORTB->PCR[10] = PORTB->PCR[3];
    PORTB->PCR[11] = PORTB->PCR[3];
    PORTB->ISFR = BUTTON_MASK;
    NVIC_ClearPendingIRQ(PORTB_IRQn);
    NVIC_SetPriority(PORTB_IRQn, 2U);
    NVIC_EnableIRQ(PORTB_IRQn);
}

/* ##########################################################################
 * [SCORE-7SEG] 7-segment display
 * Send data via 74HC595; PIT0_IRQHandler() scans one digit per invocation
 * ########################################################################## */
/* ==========================================================================
 * SECTION 11 | Sending data to the 7-segment display
 * Shift data bit by bit into the 74HC595 driver IC
 * ========================================================================== */
/* Short cycle-level settling time for the 74HC595 clock/data signals.
 * This does not control the countdown; PIT provides the time base. */
/* FUNCTION: shift_settle
 * Purpose: introduce a short delay to stabilize display data and clock signals
 */
static void shift_settle(void)
{
    uint32_t i;
    for (i = 0U; i < 16U; ++i) __NOP();
}

/* FUNCTION: shift_byte
 * Purpose: send one byte to the shift register, most-significant bit first
 */
static void shift_byte(uint8_t value)
{
    uint32_t i;
    for (i = 0U; i < 8U; ++i) {
        PTC->PCOR = CLOCK_MASK;
        if (value & 0x80U) DATA_GPIO->PSOR = DATA_MASK;
        else DATA_GPIO->PCOR = DATA_MASK;
        shift_settle();
        PTC->PSOR = CLOCK_MASK;
        shift_settle();
        value = (uint8_t)(value << 1);
    }
    PTC->PCOR = CLOCK_MASK;
}

/* FUNCTION: display_write
 * Purpose: shift and latch both segment and digit-select data
 */
static void display_write(uint8_t segments, uint8_t digit)
{
    PTB->PCOR = LATCH_MASK;
    shift_byte(segments); /* Send the segment byte first, matching the shield example. */
    shift_byte(digit);
    shift_settle();
    PTB->PSOR = LATCH_MASK;
    shift_settle();
}

/* ##########################################################################
 * [SCORE-ONOFF] Progress indicator LEDs
 * Use Shield LEDs to indicate the number of completed rounds
 * ########################################################################## */
/* ==========================================================================
 * SECTION 12 | LEDs for completed rounds
 * Turn on D1 through D4 according to completed work rounds
 * ========================================================================== */
/* FUNCTION: leds_write
 * Purpose: update four active-low LEDs based on the completion count
 */
static void leds_write(uint8_t count)
{
    uint32_t i, on = 0U;
    for (i = 0U; i < count && i < 4U; ++i) on |= BIT(led_pin[i]);
    PTD->PSOR = LED_MASK & ~on; /* Active-low: HIGH turns the LED off. */
    PTD->PCOR = on;
}

/* ##########################################################################
 * [REQ-2] Mandatory requirement: INTERRUPT
 * Uses both GPIO button interrupts and the PIT0 timer interrupt
 * ########################################################################## */
/* ==========================================================================
 * SECTION 13 | Interrupt service routines
 * PORTB captures button events; PIT0 advances time, scans the display, and controls sound
 * ========================================================================== */
/* FUNCTION: PORTB_IRQHandler
 * Purpose: record falling-edge button events and clear interrupt flags
 */
void PORTB_IRQHandler(void)
{
    uint32_t flags = PORTB->ISFR & BUTTON_MASK;
    PORTB->ISFR = flags; /* Write one to clear the interrupt flag. */
    if (flags & BIT(3)) button_edges |= EV_S1;
    if (flags & BIT(10)) button_edges |= EV_S2;
    if (flags & BIT(11)) button_edges |= EV_S3;
    /* Do not wait for debounce in the ISR, which would delay timer servicing. */
    __DSB();
}

/* FUNCTION: PIT0_IRQHandler
 * Purpose: increment milliseconds, scan one display digit, and update buzzer output
 */
void PIT0_IRQHandler(void)
{
    static uint8_t digit;
    uint32_t frame;
    uint16_t key_beep;
    uint8_t sound_on;
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    ++milliseconds;
    frame = display_frame; /* Read all four digits with one 32-bit load. */
    display_write(0xFFU, 0xF0U); /* Blank all digits before switching to reduce ghosting. */
    display_write((uint8_t)(frame >> (digit * 8U)), digit_select[digit]);
    digit = (uint8_t)((digit + 1U) & 3U);
    /* A key click overrides the original status alert for a brief moment.
     * OFF (lead) -> ON (60 ms) -> OFF (tail), then normal alerts resume.
     * This avoids a click blending into an already-active status beep.
     * PIT0 handles the pulse, so LCD updates cannot stretch the sound.
     */
    key_beep = keypress_beep_ticks;
    if (key_beep != 0U) {
        keypress_beep_ticks = (uint16_t)(key_beep - 1U);
        sound_on = (uint8_t)(key_beep > KEYPRESS_BEEP_TAIL_MS &&
                  key_beep <= KEYPRESS_BEEP_TAIL_MS + KEYPRESS_BEEP_ON_MS);
    } else {
        sound_on = buzzer_gate;
    }
    if (BUZZER_ENABLED && sound_on) {
#if BUZZER_IS_ACTIVE
        buzzer_set(1U);
#else
        /* Passive buzzer: alternate LOW and released input at 500 Hz. */
        buzzer_set((uint8_t)(milliseconds & 1U));
#endif
    } else buzzer_set(0U);
    __DSB();
}

/* ##########################################################################
 * [REQ-1] Mandatory requirement: TIMER / COUNTER
 * Use hardware PIT0 as the 1-millisecond time base
 * ########################################################################## */
/* ==========================================================================
 * SECTION 14 | PIT timer configuration
 * Calculate the bus frequency and trigger a PIT0 interrupt every millisecond
 * ========================================================================== */
/* FUNCTION: timer_init
 * Purpose: enable the PIT0 timer and its time-base interrupt
 */
static void timer_init(void)
{
    uint32_t core_div, bus_div, bus_hz;
    SystemCoreClockUpdate();
    core_div = ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> SIM_CLKDIV1_OUTDIV1_SHIFT) + 1U;
    bus_div = ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> SIM_CLKDIV1_OUTDIV2_SHIFT) + 1U;
    /* PIT uses the bus clock, not the core clock. Read the dividers instead of assuming 60 MHz. */
    bus_hz = (SystemCoreClock * core_div) / bus_div;
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT->MCR = PIT_MCR_FRZ_MASK; /* Enable PIT and freeze it when the debugger halts. */
    PIT->CHANNEL[0].TCTRL = 0U;
    PIT->CHANNEL[0].LDVAL = bus_hz / 1000U - 1U;
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    NVIC_ClearPendingIRQ(PIT0_IRQn);
    NVIC_SetPriority(PIT0_IRQn, 1U); /* Display refresh has higher priority than buttons. */
    NVIC_EnableIRQ(PIT0_IRQn);
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
}

/* ##########################################################################
 * [SCORE-LCD] Optional peripheral: LCD
 * The LCD uses software I2C on D1/PTC17 and D0/PTC16
 * ########################################################################## */
/* ==========================================================================
 * SECTION 15 | LCD communication using software I2C
 * Use D1/PTC17 for SDA and D0/PTC16 for SCL via the Shield wireless header
 * ========================================================================== */
/* Software I2C through shield wireless header: D0/PTC16=SCL, D1/PTC17=SDA. Use a 3.3V/5V I2C level shifter.
 * All waits are bounded; an absent LCD never stops the timer.
 * Common backpack wiring: RS=P0, RW=P1, EN=P2, backlight=P3.
 */
#define LCD_SCL (1UL << 16)
#define LCD_SDA (1UL << 17)
static uint32_t lcd_delay_count;
static volatile uint8_t lcd_online, lcd_address; /* Inspect in Keil Watch if needed. */
static char lcd_sent[LCD_ROWS][LCD_COLS];
static uint32_t lcd_scan;

/* FUNCTION: bus_delay
 * Purpose: provide a brief delay for software I2C timing
 */
static void bus_delay(void)
{
    volatile uint32_t n;
    for (n = 0U; n < lcd_delay_count; ++n) __NOP();
}

/* FUNCTION: bus_low
 * Purpose: drive the selected I2C signal line LOW
 */
static void bus_low(uint32_t mask)
{
    PTC->PCOR = mask;
    PTC->PDDR |= mask;
}

/* FUNCTION: bus_release
 * Purpose: release the I2C line to input so an external pull-up can take it HIGH
 */
static void bus_release(uint32_t mask) { PTC->PDDR &= ~mask; }

/* FUNCTION: clock_high
 * Purpose: release SCL and wait for HIGH with a bounded retry count
 */
static uint8_t clock_high(void)
{
    uint32_t tries = 100U;
    bus_release(LCD_SCL);
    do {
        bus_delay();
        if (PTC->PDIR & LCD_SCL) return 1U;
    } while (--tries);
    return 0U;
}

/* FUNCTION: bus_stop
 * Purpose: generate an I2C STOP condition
 */
static void bus_stop(void)
{
    bus_low(LCD_SCL | LCD_SDA);
    bus_delay();
    (void)clock_high();
    bus_release(LCD_SDA);
    bus_delay();
}

/* FUNCTION: bus_start
 * Purpose: generate an I2C START condition once the bus is released and ready
 */
static uint8_t bus_start(void)
{
    bus_release(LCD_SDA);
    if (!clock_high() || !(PTC->PDIR & LCD_SDA)) return 0U;
    bus_low(LCD_SDA);
    bus_delay();
    bus_low(LCD_SCL);
    return 1U;
}

/* FUNCTION: bus_byte
 * Purpose: transmit eight bits and check the peripheral ACK response
 */
static uint8_t bus_byte(uint8_t value)
{
    uint32_t i;
    uint8_t ack;
    for (i = 0U; i < 8U; ++i) {
        if (value & 0x80U) bus_release(LCD_SDA);
        else bus_low(LCD_SDA);
        bus_delay();
        if (!clock_high()) return 0U;
        bus_low(LCD_SCL);
bus_delay();
        value <<= 1;
    }
    bus_release(LCD_SDA);
    bus_delay();
    if (!clock_high()) return 0U;
    ack = (uint8_t)((PTC->PDIR & LCD_SDA) == 0U);
    bus_low(LCD_SCL);
    bus_delay();
    return ack;
}

/* FUNCTION: lcd_transfer
 * Purpose: send the device address and data bytes to the LCD I2C adapter
 */
static uint8_t lcd_transfer(const uint8_t *data, uint32_t count)
{
    uint8_t ok = bus_start();
    uint32_t i;
    if (ok) ok = bus_byte((uint8_t)(lcd_address << 1));
    for (i = 0U; ok && i < count; ++i) ok = bus_byte(data[i]);
    bus_stop();
    return ok;
}

/* FUNCTION: lcd_pack
 * Purpose: encode one LCD byte as two four-bit transfers with Enable pulses
 */
static void lcd_pack(uint8_t *data, uint8_t value, uint8_t rs)
{
    uint8_t high = (uint8_t)((value & 0xF0U) | 0x08U | rs);
    uint8_t low = (uint8_t)((value << 4) | 0x08U | rs);
    data[0] = high; data[1] = high | 0x04U; data[2] = high;
    data[3] = low; data[4] = low | 0x04U; data[5] = low;
}

/* FUNCTION: lcd_command
 * Purpose: transmit one LCD command byte
 */
static uint8_t lcd_command(uint8_t command)
{
    uint8_t data[6];
    lcd_pack(data, command, 0U);
    return lcd_transfer(data, 6U);
}

/* Only used at startup, with PIT interrupts enabled. */
/* FUNCTION: lcd_boot_wait
 * Purpose: wait during LCD startup while PIT interrupts remain enabled
 */
static void lcd_boot_wait(uint32_t ms)
{
    uint32_t start = milliseconds;
    while ((uint32_t)(milliseconds - start) < ms) __NOP();
}

/* FUNCTION: lcd_init
 * Purpose: probe for the I2C adapter and initialize the LCD in four-bit mode
 */
static void lcd_init(void)
{
    uint32_t i, row, col;
    uint8_t n, data[3];
    static const uint8_t init_nibble[4] = {0x30U, 0x30U, 0x30U, 0x20U};
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    PTC->PDDR &= ~(LCD_SCL | LCD_SDA);
    PORTC->PCR[16] = PORT_PCR_MUX(1);
    PORTC->PCR[17] = PORT_PCR_MUX(1);
    lcd_delay_count = SystemCoreClock / 200000U + 1U;
    lcd_online = 0U;
    lcd_scan = 0U;
    lcd_boot_wait(50U);
    /* Probe addresses only; no LCD commands sent to other address ranges. */
    if (LCD_I2C_ADDRESS != 0U) {
        lcd_address = (uint8_t)LCD_I2C_ADDRESS;
        if (!lcd_transfer(0, 0U)) return;
    } else {
        for (i = 0U; i < 16U; ++i) {
            lcd_address = (uint8_t)(i < 8U ? 0x27U - i : 0x3FU - (i - 8U));
            if (lcd_transfer(0, 0U)) break;
        }
        if (i == 16U) return;
    }
    for (i = 0U; i < 4U; ++i) {
        n = (uint8_t)(init_nibble[i] | 0x08U);
        data[0] = n; data[1] = n | 0x04U; data[2] = n;
        if (!lcd_transfer(data, 3U)) return;
        lcd_boot_wait(i == 0U ? 5U : 2U);
    }
    if (!lcd_command(0x28U) || !lcd_command(0x08U) || !lcd_command(0x01U)) return;
    lcd_boot_wait(3U);
    if (!lcd_command(0x06U) || !lcd_command(0x0CU)) return;
    for (row = 0U; row < LCD_ROWS; ++row)
        for (col = 0U; col < LCD_COLS; ++col) lcd_sent[row][col] = '\0';
    lcd_online = 1U;
}

/* FUNCTION: lcd_service
 * Purpose: send at most one changed character position per service call
 */
static void lcd_service(void)
{
    static const uint8_t row_address[4] = {0x00U, 0x40U, 0x14U, 0x54U};
    uint32_t i, row, col;
    uint8_t data[12];
    if (!lcd_online) return;
    for (i = 0U; i < LCD_ROWS * LCD_COLS; ++i) {
        row = lcd_scan / LCD_COLS;
        col = lcd_scan % LCD_COLS;
        lcd_scan = (lcd_scan + 1U) % (LCD_ROWS * LCD_COLS);
        if (lcd_sent[row][col] == lcd_text[row][col]) continue;
        lcd_pack(data, (uint8_t)(0x80U | (row_address[row] + col)), 0U);
        lcd_pack(data + 6, (uint8_t)lcd_text[row][col], 1U);
        if (!lcd_transfer(data, 12U)) { lcd_online = 0U; return; }
        lcd_sent[row][col] = lcd_text[row][col];
        return;
    }
}

/* ##########################################################################
 * [REQ-3] Mandatory requirement: separate functions
 * main() coordinates tasks; peripheral details are implemented in dedicated functions above
 * ########################################################################## */
/* ==========================================================================
 * SECTION 16 | Main function and task orchestration
 * Initialize hardware, then poll buttons, update state, and refresh outputs
 * ========================================================================== */
/* FUNCTION: main
 * Purpose: coordinate the application functions in the main loop
 */
int main(void)
{
    uint32_t now, previous, edges, raw, events, saved_mask;
    State previous_state;
    __disable_irq();
    gpio_init();
    display_write(0xFFU, 0xF0U);
    app_init(0U);
    display_frame = app_display(500U);
    timer_init();
    __enable_irq();
    lcd_init(); /* Called once at boot; IRQs keep the 7-segment refreshing. */
    app.last_ms = milliseconds;
    previous = milliseconds;

    for (;;) {
        now = milliseconds;
        if (now == previous) continue; /* Run main-loop work once per tick. */
        previous = now;

        /* Read and clear flags atomically to avoid losing interrupt events. */
        saved_mask = __get_PRIMASK();
        __disable_irq();
        edges = button_edges;
        button_edges = 0U;
        __set_PRIMASK(saved_mask);

        raw = PTB->PDIR;
        raw = ((raw & BIT(3)) ? 0U : EV_S1) |
              ((raw & BIT(10)) ? 0U : EV_S2) |
              ((raw & BIT(11)) ? 0U : EV_S3);
        events = buttons_update(now, raw, edges);
        previous_state = app.state;
        app_update(now);
        /* If a timeout changes state now, do not apply an old button command to the rating screen. */
        if (app.state != previous_state) events = 0U;
        /* For simultaneous events, handle only one command; cancel has highest priority. */
        if (events & EV_S3_LONG) app_on_event(EV_S3_LONG);
        else if (events & EV_S1) app_on_event(EV_S1);
        else if (events & EV_S2) app_on_event(EV_S2);
        else if (events & EV_S3) app_on_event(EV_S3);

        leds_write(app.rounds);
        display_frame = app_display(now); /* Publish the display with a single 32-bit store. */
        buzzer_gate = (uint8_t)(app.sound_ms != 0U && (app.sound_ms / 100U) % 2U == 0U);
        lcd_render(now);
        lcd_service(); /* One changed character per call; never from an ISR. */
    }
}
#endif /* FOCUSQUEST_HOST_TEST */
