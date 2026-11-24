/* ##########################################################################
 *               FOCUSQUEST: สรุปเงื่อนไข EL334 MINI-PROJECT
 * ##########################################################################
 * อ้างอิงเงื่อนไขและเกณฑ์คะแนนหน้า 3 ของ 1 Mini_Project.pdf
 * ป้าย REQ / SCORE ใช้ค้นหาในโค้ด ไม่ใช่เลขข้อทางการของเอกสาร
 *
 * ========================= เงื่อนไขบังคับ ===========================
 * [REQ-1] TIMER / COUNTER: อย่างน้อย 1 การทำงาน
 *         SECTION 14: timer_init() ตั้ง PIT0 ให้ทำงานทุก 1 ms
 *         SECTION 13: PIT0_IRQHandler() เพิ่มตัวนับมิลลิวินาที
 *         SECTION 04: app_update() ลดเวลานับถอยหลังตามเวลาที่ผ่านไป
 *         ไม่ใช้ Timer ควบคุม RGB LED บนบอร์ด FRDM-K64F
 *
 * [REQ-2] INTERRUPT: อย่างน้อย 1 การทำงาน
 *         SECTION 13: PORTB_IRQHandler() รับเหตุการณ์ปุ่มกด
 *                     PIT0_IRQHandler() ให้ฐานเวลาและสแกนจอ
 *         SECTION 10 และ 14: ตั้งค่าและเปิดใช้งาน Interrupt
 *
 * [REQ-3] แยกการทำงานของอุปกรณ์เป็นฟังก์ชันย่อย
 *         SECTION 05: buttons_update() - ประมวลผลปุ่มกด
 *         SECTION 09: buzzer_set() - ควบคุมเสียง Buzzer
 *         SECTION 11: display_write() - ส่งข้อมูลไปยังจอ 7-Segment
 *         SECTION 12: leds_write() - แสดงจำนวนรอบที่สำเร็จด้วย LED
 *         SECTION 15: lcd_init()/lcd_service() - เริ่มต้นและอัปเดต LCD
 *         SECTION 16: main() เรียกใช้ฟังก์ชันย่อยตามลำดับ
 *         ฟังก์ชันทั้งหมดอยู่ในไฟล์ .c เดียวได้ ไม่จำเป็นต้องแยกไฟล์
 *
 * ========================== หมวดเกณฑ์คะแนน ==============================
 * [SCORE-SWITCH] SECTION 05: ปุ่มกด S1-S3
 * [SCORE-ONOFF] SECTION 09 และ 12: เปิด/ปิด Buzzer และ LED
 * [SCORE-7SEG] SECTION 06 และ 11: จอ 7-Segment 4 หลัก
 * [SCORE-LCD] SECTION 07 และ 15: LCD ภายนอก (อุปกรณ์เพิ่มเติม)
 * หมายเหตุ: Buzzer ใช้เสียงเปิด/ปิด ไม่ใช่เพลงที่เปลี่ยนความถี่หลายโน้ต
 * ป้ายเหล่านี้ระบุตำแหน่งหลักฐานในโค้ด ไม่ได้รับประกันคะแนนจากอาจารย์
 *
 * ========================== คุณสมบัติโครงงาน ================================
 * SECTION 03: บันทึกประวัติ 4 รอบใน RAM
 * SECTION 04: รอบทำงาน/พัก หยุด/เริ่มต่อ และปรับเวลาตามการประเมิน
 * คุณสมบัติเหล่านี้เป็นการออกแบบของกลุ่ม ไม่ใช่เงื่อนไขบังคับเพิ่มเติม
 *
 * สิ่งที่ต้องส่งเพิ่มเติม (หน้า 4): รายงาน Word พร้อม Diagram/Flowchart
 * โปรแกรมและเอกสารบน GitHub พร้อมลิงก์ในรายงาน และนำเสนอ/ตอบคำถาม
 * เงื่อนไขกลุ่ม (หน้า 3): 2 คน หากมากกว่าต้องได้รับอนุญาตและมีเงื่อนไขเพิ่ม
 *
 * ฉบับนี้เพิ่มเสียงเมื่อกดปุ่มที่ผ่าน Debounce โดยคงตรรกะและฮาร์ดแวร์เดิม
 * ########################################################################## */

/* FOCUSQUEST - คู่มือฟังก์ชันในไฟล์เดียว (คำอธิบายภาษาไทย)
 * แปลเฉพาะคำอธิบาย คงคำสั่งโปรแกรมและการกำหนดขาเดิม
 * ค้นหา "SECTION nn" หรือ "FUNCTION: function_name" ใน Keil เพื่อไปยังส่วนที่ต้องการ
 * ทุกฟังก์ชันอยู่ใน main.c บันทึกไฟล์ด้วย UTF-8
 *
 * 01 การตั้งค่าเริ่มต้น
 * 02 ข้อมูลและสถานะของโปรแกรม
 * 03 ประวัติการทำงาน 4 รอบ
 * 04 การนับถอยหลังและเปลี่ยนสถานะ
 * 05 การประมวลผลเหตุการณ์ปุ่มกด
 * 06 การจัดรูปแบบตัวเลขสำหรับจอ 7-Segment
 * 07 การจัดรูปแบบข้อความ LCD
 * 08 ขาฮาร์ดแวร์และตัวแปรที่ใช้ร่วมกับ Interrupt
 * 09 การควบคุม Buzzer
 * 10 การตั้งค่า GPIO
 * 11 การส่งข้อมูลไปยังจอ 7-Segment
 * 12 LED แสดงจำนวนรอบที่สำเร็จ
 * 13 ฟังก์ชันบริการ Interrupt (ISR)
 * 14 การตั้งค่า PIT Timer
 * 15 การสื่อสารกับ LCD ผ่าน Software I2C
 * 16 ฟังก์ชัน Main และการประสานงานระบบ
 */

/* FocusQuest - FRDM-K64F + Arduino Multifunction Shield
 * Keil uVision / CMSIS MK64F12.h / C99 คำอธิบายไทย เข้ารหัส UTF-8
 * การแก้ Buzzer: ปิด = PTA1 เป็น Input ไม่ใช้ Pull; เปิด = Output LOW
 * ห้ามใช้ Hardware Open-drain ร่วมกับ Pull-up 5 V บน K64
 * ฉบับ LCD และประวัติ 4 รอบใน RAM ดู README.md ในชุด ZIP เดิม
 * แก้ขาสำหรับ Rev E: Arduino D8 เดิมคือ PTC12 (ตรวจสอบกับ Rev E3)
 * ใช้โปรเจกต์ K64F ที่ทำงานได้พร้อม startup และ system_MK64F12.c
 * แทนที่ main.c เดิม ห้ามคอมไพล์ main หรือ IRQ Handler ซ้ำ
 * ใช้ PIT0, GPIO Interrupt และ Software I2C ไม่ใช้ Arduino Library หรือ RTOS
 * ตรวจสอบความเข้ากันได้ของแรงดันและการกำหนดขาก่อนต่อฮาร์ดแวร์
 * เสียบ Shield ตามขาเดิม ไม่ต้องต่อสาย DATA ไป D6
 * บอร์ด Rev D1 รุ่นเก่าใช้ D8 เป็น PTA0 ให้ตรวจสอบรุ่นบอร์ด
 * No ULINK2/ME Device found คือข้อผิดพลาดในการตรวจพบ Debug Probe ไม่ใช่ปัญหาฟอนต์ */
#include <stdint.h>
#ifndef FOCUSQUEST_HOST_TEST
#include "MK64F12.h"
/* จุดเข้าฟังก์ชัน Interrupt ที่ Startup Vector Table เรียกใช้
 * ประกาศ Prototype เพื่อป้องกันคำเตือนว่าขาด Prototype ห้ามประกาศเป็น static
 */
void PORTB_IRQHandler(void);
void PIT0_IRQHandler(void);

/* รองรับ Header รุ่นเก่า (PTA) และรุ่นใหม่ (GPIOA) */
#ifndef PTA
#define PTA GPIOA
#define PTB GPIOB
#define PTC GPIOC
#define PTD GPIOD
#define PTE GPIOE
#endif
#endif

/* ==========================================================================
 * SECTION 01 | การตั้งค่าเริ่มต้น
 * ตั้งเวลา Demo รูปแบบเสียง ขนาด LCD และช่วงเวลา Debounce
 * ==========================================================================  */
#ifndef DEMO_MODE
#define DEMO_MODE 1          /* 1 = เวลาสั้นสำหรับสาธิต; 0 = เวลาปกติหน่วยนาที */
#endif
#define BUZZER_IS_ACTIVE 1   /* 1 = Active Buzzer; 0 = Passive Buzzer (500 Hz) */
#define BUZZER_ENABLED   1   /* ตั้งเป็น 0 เพื่อทดสอบแบบไม่มีเสียง */

/* LCD: HD44780 ร่วมกับ PCF8574 Backpack ทั่วไป
 * กำหนด 16/2 สำหรับ LCD 1602 หรือ 20/4 สำหรับ LCD 2004
 * ขา Backpack: P0=RS, P1=RW, P2=EN, P3=ไฟพื้นหลัง, P4..P7=D4..D7
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
#define LCD_I2C_ADDRESS 0U /* 0 = สแกนช่วง Address ของ PCF8574; ค่าอื่นคือ Address 7 บิต */

#define BUTTON_DEBOUNCE_MS 25U
#define BUTTON_LONG_MS     1200U

/* เพิ่มเสียงคลิกหนึ่งครั้งเมื่อกด S1/S2/S3 จริงและผ่าน Debounce
 * เว้นช่วงเงียบสั้น ๆ ก่อนและหลังเสียง เพื่อแยกจากเสียงแจ้งเตือนนับถอยหลัง
 * โดยไม่หยุดการทำงานของ Main Loop
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
 * SECTION 02 | ข้อมูลและสถานะของโปรแกรม
 * กำหนดเหตุการณ์ปุ่ม สถานะ ตัวแปรระบบ และที่เก็บประวัติ
 * ==========================================================================  */
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
/* ตั้งค่าใหม่โดย buttons_update() และลดค่าทุก 1 ms ใน PIT0 Interrupt */
static volatile uint16_t keypress_beep_ticks;

typedef enum { REC_EMPTY, REC_ACTIVE, REC_DONE, REC_CANCELLED } RecordStatus;
typedef struct {
    uint32_t target_ms;
    uint32_t used_ms;
    uint32_t left_ms;
    RecordStatus status;
} RoundRecord;

/* เก็บเฉพาะ RAM แต่ละช่องตรงกับ LED D1, D2, D3, D4 ของชุดปัจจุบัน */
static RoundRecord history[4];
static uint8_t history_view;
static uint8_t history_index;
static char lcd_text[LCD_ROWS][LCD_COLS];

/* ==========================================================================
 * SECTION 03 | ประวัติการทำงาน 4 รอบ
 * ล้าง เริ่ม บันทึก และรวมข้อมูลประวัติ 4 รอบใน RAM
 * ==========================================================================  */
/* FUNCTION: history_clear
 * หน้าที่: ล้างข้อมูลประวัติทั้ง 4 ช่องใน RAM เมื่อเริ่มชุดใหม่
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
 * หน้าที่: บันทึกเวลาเป้าหมายที่กำหนดไว้เมื่อเริ่มรอบ Focus
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
 * หน้าที่: บันทึกเวลาที่ใช้และเวลาคงเหลือของรอบปัจจุบัน
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
 * หน้าที่: รวมเวลาเฉพาะรอบที่สำเร็จ ไม่รวมเวลาพักและรอบที่ยกเลิก
 */
static uint32_t history_total_done_ms(void)
{
    uint32_t i, total = 0U;
    for (i = 0U; i < 4U; ++i)
        if (history[i].status == REC_DONE) total += history[i].used_ms;
    return total;
}


/* ฟังก์ชันเหล่านี้ไม่เข้าถึง Register ฮาร์ดแวร์ จึงทดสอบบน PC ได้ */
/* ==========================================================================
 * SECTION 04 | การนับถอยหลังและเปลี่ยนสถานะ
 * คำนวณเวลาคงเหลือ เริ่ม หยุดชั่วคราว ประเมินผล และจัดการเวลาพัก
 * ==========================================================================  */
/* FUNCTION: subtract_floor
 * หน้าที่: ลบโดยจำกัดค่าต่ำสุดที่ศูนย์ ป้องกันการวนค่าของ unsigned เมื่อค่าติดลบ
 */
static uint32_t subtract_floor(uint32_t value, uint32_t amount)
{
    return value > amount ? value - amount : 0U;
}

/* FUNCTION: adjust_value
 * หน้าที่: ปรับค่าที่ตั้งไว้โดยจำกัดให้อยู่ระหว่างค่าต่ำสุดและสูงสุด
 */
static uint32_t adjust_value(uint32_t value, uint32_t step,
                             uint32_t minimum, uint32_t maximum, int up)
{
    if (up) return value + step > maximum ? maximum : value + step;
    return value < minimum + step ? minimum : value - step;
}

/* FUNCTION: app_init
 * หน้าที่: เข้าสู่สถานะ READY และล้างประวัติการทำงาน
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

/* ใช้เวลาปัจจุบันแทน delay() เพื่อให้ปุ่มและจอยังตอบสนองได้ */
/* FUNCTION: app_update
 * หน้าที่: ลดเวลาของช่วงที่กำลังทำงานตามมิลลิวินาทีที่ผ่านไป ไม่รวมช่วง Pause
 */
static void app_update(uint32_t now)
{
    uint32_t elapsed = now - app.last_ms; /* การลบแบบ unsigned รองรับกรณีตัวนับเวลาวนกลับ */
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
        } /* ตรึงข้อมูลรอบนี้ไว้ก่อนนำผลประเมินไปเปลี่ยนเวลารอบถัดไป */
        app.state = ST_RATE;
        app.sound_ms = app.rounds == 4U ? 1000U : 600U;
    } else {
        app.state = ST_READY; /* รอผู้ใช้หลังหมดเวลาพัก ไม่เริ่ม Focus อัตโนมัติ */
        app.remaining_ms = app.focus_s * 1000U;
        app.sound_ms = 400U;
    }
}

/* FUNCTION: app_on_event
 * หน้าที่: ประมวลผลคำสั่งปุ่มหนึ่งรายการตามหน้าจอหรือสถานะปัจจุบัน
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
        return; /* กดค้างเพื่อยกเลิกรอบปัจจุบัน โดยเก็บรอบที่สำเร็จไว้ */
    }

    if (app.state == ST_READY) {
        if (event == EV_S1) {
            if (app.rounds == 4U) {
                app.rounds = 0U;
                history_clear(); /* เก็บประวัติทั้ง 4 รอบจนกว่าจะเริ่มชุดใหม่ */
            }
            history_begin(); /* เริ่มใหม่ในช่องที่ยกเลิกจะเขียนทับข้อมูลความพยายามเดิม */
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
        /* S1 = นานเกินไป; S2 = เหมาะสม; S3 = เพิ่มเวลา เป็นกฎ ไม่ใช่การวัดสมาธิ */
        if (event == EV_S1 || event == EV_S3)
            app.focus_s = adjust_value(app.focus_s, FOCUS_STEP_S,
                                        FOCUS_MIN_S, FOCUS_MAX_S, event == EV_S3);
        if (event == EV_S1 || event == EV_S2 || event == EV_S3) {
            app.state = ST_BREAK;
            /* เวลาพักรอบที่ 4 ต้องไม่น้อยกว่าเวลาพักปกติที่กำหนด */
            app.remaining_ms = ((app.rounds == 4U && LONG_BREAK_S > app.break_s)
                                ? LONG_BREAK_S : app.break_s) * 1000U;
            app.paused = 0U;
            app.sound_ms = 100U;
        }
    } else { /* สถานะ ST_FOCUS หรือ ST_BREAK */
        if (event == EV_S1) {
            app.paused = (uint8_t)!app.paused; /* รักษาเวลาคงเหลือระดับเศษมิลลิวินาทีไว้ */
            app.sound_ms = 80U;
        } else if (event == EV_S2) app.show_rounds_ms = 2000U;
    }
}

/* GPIO Interrupt เปิดรับแต่ละการกด ต้องมีสัญญาณคงที่ 25 ms
 * S1/S2 ทำงานเมื่อกด ส่วน S3 กดสั้นทำงานเมื่อปล่อย เพื่อแยกจากการกดค้าง
 * การกดค้างไม่ทำคำสั่งซ้ำ ต้องปล่อยก่อนกดอีกครั้ง */
/* ##########################################################################
 * [SCORE-SWITCH] การควบคุมด้วยปุ่มกด
 * S1-S3 ควบคุมระบบ กรองสัญญาณเด้งด้วยซอฟต์แวร์
 * ########################################################################## */
/* ==========================================================================
 * SECTION 05 | การประมวลผลเหตุการณ์ปุ่มกด
 * กรองสัญญาณเด้งและตรวจการกดสั้น/ค้างหลัง GPIO Interrupt
 * ==========================================================================  */
/* FUNCTION: buttons_update
 * หน้าที่: คืนค่าเหตุการณ์ปุ่มที่ผ่าน Debounce รวมถึงการกด S3 ค้าง
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
                /* ส่งเสียงทุกครั้งที่ยืนยันการกดจริง รวม S3 ด้วย
                 * คำสั่ง S3 ยังประมวลผลเมื่อปล่อยหรือกดค้างเช่นเดิม
                 * ไม่มีเสียงซ้ำระหว่างกดปุ่มค้าง
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

/* Active Low: 0 ทำให้ Segment ติด ลำดับบิต DP,G,F,E,D,C,B,A */
/* ==========================================================================
 * SECTION 06 | การจัดรูปแบบตัวเลขสำหรับจอ 7-Segment
 * แปลงเวลาและสถานะเป็นรหัส Segment ของจอ 4 หลัก
 * ==========================================================================  */
static const uint8_t segment[10] = {
    0xC0U, 0xF9U, 0xA4U, 0xB0U, 0x99U,
    0x92U, 0x82U, 0xF8U, 0x80U, 0x90U
};

/* FUNCTION: pack_display
 * หน้าที่: รวมรหัส Segment 4 ไบต์เป็นข้อมูลแสดงผลขนาด 32 บิต
 */
static uint32_t pack_display(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return (uint32_t)a | ((uint32_t)b << 8) |
           ((uint32_t)c << 16) | ((uint32_t)d << 24);
}

/* FUNCTION: display_time
 * หน้าที่: แปลงเวลาเป็น MM.SS สำหรับจอ 4 หลัก
 */
static uint32_t display_time(uint32_t seconds, int dot)
{
    uint32_t minutes = seconds / 60U;
    uint8_t b = segment[minutes % 10U];
    if (dot) b &= 0x7FU; /* จุดทศนิยมหลังหลักที่ 2: MM.SS */
    return pack_display(segment[(minutes / 10U) % 10U], b,
                        segment[(seconds % 60U) / 10U], segment[seconds % 10U]);
}

/* FUNCTION: app_display
 * หน้าที่: เลือกแสดงเวลาหรือข้อความสถานะบนจอ 7-Segment
 */
static uint32_t app_display(uint32_t now)
{
    uint32_t seconds;
    if (app.state == ST_RATE)
        return pack_display(0xAFU, 0x88U, 0x87U, 0x86U); /* rAtE (ประเมินผล) */
    if (app.show_rounds_ms != 0U)
        return pack_display(0xC6U, 0xFFU, 0xFFU, segment[app.rounds]); /* C  n (จำนวนรอบ) */
    if (app.paused && (now / 500U) % 2U)
        return 0xFFFFFFFFU; /* กระพริบเมื่อ Pause โดย ISR ยังสแกนจอต่อ */

    /* แสดงอักษรบอกโหมดบนจอ 4 หลักช่วงสั้น ๆ */
    if (now % 2000U < 400U) {
        if (app.state == ST_SET_FOCUS)
            return pack_display(0x8EU, 0xBFU, 0xBFU, 0xBFU); /* F--- (ตั้ง Focus) */
        if (app.state == ST_SET_BREAK)
            return pack_display(0x83U, 0xBFU, 0xBFU, 0xBFU); /* b--- (ตั้ง Break) */
        if (app.state == ST_BREAK && !app.paused)
            return pack_display(0x83U, 0xFFU, 0xFFU, 0xFFU); /* b (เวลาพัก) */
    }
    if (app.state == ST_SET_FOCUS || app.state == ST_READY) seconds = app.focus_s;
    else if (app.state == ST_SET_BREAK) seconds = app.break_s;
    else seconds = (app.remaining_ms + 999U) / 1000U; /* ปัดขึ้นเพื่อไม่ให้วินาทีแรกหายไปทันที */
    return display_time(seconds, 1);
}

/* ==========================================================================
 * SECTION 07 | การจัดรูปแบบข้อความ LCD
 * เตรียมข้อความเวลา การตั้งค่า และประวัติในหน่วยความจำก่อนแสดง
 * ==========================================================================  */
/* FUNCTION: lcd_put
 * หน้าที่: เขียนข้อความลง LCD Buffer โดยไม่เกินจำนวนแถวและคอลัมน์
 */
static void lcd_put(uint32_t row, uint32_t col, const char *s)
{
    if (row >= LCD_ROWS) return;
    while (*s && col < LCD_COLS) lcd_text[row][col++] = *s++;
}

/* FUNCTION: lcd_digit
 * หน้าที่: เขียนเลขฐานสิบหนึ่งหลักลง LCD Buffer
 */
static void lcd_digit(uint32_t row, uint32_t col, uint32_t n)
{
    if (row < LCD_ROWS && col < LCD_COLS)
        lcd_text[row][col] = (char)('0' + (n % 10U));
}

/* เวลารายรอบใช้ MM:SS เวลารวมใช้ MMM:SS เมื่อจำเป็น */
/* FUNCTION: lcd_time
 * หน้าที่: แปลงมิลลิวินาทีเป็นนาที:วินาที พร้อมเลือกปัดขึ้นหรือปัดลง
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
 * หน้าที่: คืนข้อความภาษาอังกฤษของสถานะรอบที่บันทึกไว้
 */
static const char *record_name(RecordStatus status)
{
    if (status == REC_ACTIVE) return "RUN";
    if (status == REC_DONE) return "DONE";
    if (status == REC_CANCELLED) return "CANCEL";
    return "EMPTY";
}

/* FUNCTION: lcd_render
 * หน้าที่: สร้างข้อความหน้าจอ LCD ใน RAM โดยไม่เข้าถึงฮาร์ดแวร์
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
 * SECTION 08 | ขาฮาร์ดแวร์และตัวแปรที่ใช้ร่วมกับ Interrupt
 * ใช้การกำหนดขาจากฉบับที่ทดสอบแล้วและประกาศตัวแปรที่ใช้ร่วมกับ ISR
 * ==========================================================================  */
#define BIT(n) (1UL << (n))
#define LATCH_MASK BIT(23)   /* D4 -> PTB23 (LATCH) */
#define CLOCK_MASK BIT(3)    /* D7 -> PTC3 (CLOCK) */
/* FRDM-K64F Rev E3: Arduino D8 เดิมคือ PTC12
 * เสียบ Shield ตามขาเดิม ไม่ย้าย DATA ไป D6
 * ฉบับนี้ไม่แตะ PTA0/SWD_CLK
 */
#define DATA_GPIO PTC
#define DATA_PORT PORTC
#define DATA_PIN  12U
#define DATA_MASK BIT(DATA_PIN)
#define BUZZ_MASK  BIT(1)    /* D3 -> PTA1, Active Low ผ่านทรานซิสเตอร์ */
#define BUTTON_MASK (BIT(3) | BIT(10) | BIT(11)) /* A1,A2,A3 -> PTB3,10,11 (ปุ่มกด) */
#define LED_MASK 0x0FU      /* D13,D12,D11,D10 -> PTD1,3,2,0 (LED) */

static volatile uint32_t milliseconds;
static volatile uint32_t button_edges;
static volatile uint32_t display_frame = 0xFFFFFFFFU;
static volatile uint8_t buzzer_gate;
/* 4 บิตล่างเลือกหลักด้วย HIGH ส่วน IC อีกตัวขับ Segment แบบ Active Low */
static const uint8_t digit_select[4] = {0xF1U, 0xF2U, 0xF4U, 0xF8U};
static const uint8_t led_pin[4] = {1U, 3U, 2U, 0U};

/* ##########################################################################
 * [SCORE-ONOFF] เสียงแจ้งเตือน Buzzer แบบเปิด/ปิด
 * ควบคุมผ่าน buzzer_set() ไม่ใช่คุณสมบัติ Melody
 * ########################################################################## */
/* ==========================================================================
 * SECTION 09 | การควบคุม Buzzer
 * เปิดเสียงด้วย Output LOW และปิดด้วย Input High Impedance
 * ==========================================================================  */
/* ขา Digital Input ของ K64 ทน 5 V ตามเงื่อนไขที่รองรับ ส่วน GPIO Output
 * (รวม Hardware Open-drain Output) ห้ามถูกดึงภายนอกให้สูงกว่า VDD
 * ดังนั้น OFF ใช้ INPUT/Hi-Z จริงโดย PE=0 ไม่ใช่ Output HIGH หรือ ODE
 * ON ขับ LOW ผ่าน R9=10k เดิมของ Shield ไปยัง Base ของ Q1 PNP
 * ใช้เฉพาะ FRDM-K64F ที่มีไฟเลี้ยงและวงจร Shield 3906 5 V ที่ระบุ
 * Base ของ PNP ที่ลอยไม่ได้รับประกันว่าจะปิดเสียงได้ทุกบอร์ด
 * หากปิดแล้วยังมีเสียง ให้ตรวจสายและวงจร Pull-up ระหว่าง Base-Emitter/Driver
 * กับอาจารย์ผู้สอน ห้ามจ่าย 5 V เข้าขา Output ของ MCU
 */
/* FUNCTION: buzzer_set
 * หน้าที่: ขับหรือปล่อยขาควบคุมทรานซิสเตอร์ Buzzer บน Shield
 */
static void buzzer_set(uint8_t on)
{
    if (on) {
        PTA->PCOR = BUZZ_MASK;  /* ตั้ง Output Latch เป็น LOW ก่อนเปิดโหมด Output */
        PTA->PDDR |= BUZZ_MASK;
    } else {
        PTA->PDDR &= ~BUZZ_MASK; /* ปิดวงจรขับ Output โดยคงค่า Latch เป็น LOW */
    }
}

/* ==========================================================================
 * SECTION 10 | การตั้งค่า GPIO
 * ตั้งขา Shield และเปิด Falling-edge Interrupt สำหรับปุ่มกด
 * ==========================================================================  */
/* FUNCTION: gpio_init
 * หน้าที่: ตั้งค่า GPIO ของ Shield และเปิด Interrupt สำหรับปุ่มกด
 */
static void gpio_init(void)
{
    uint32_t i;
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK |
                  SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;
    /* กำหนดระดับ Output ก่อนเปิดใช้งาน Output เพื่อลดสัญญาณกระตุกตอนเริ่ม */
    /* ปล่อย PTA1 ก่อนเลือก GPIO: ปิด = Input ไม่ใช้ Pull ภายใน
     * เพื่อไม่ให้ขับ 3.3 V สวนวงจร Base ของ PNP 5 V บน Shield
     */
    PTA->PDDR &= ~BUZZ_MASK;
    PTA->PCOR = BUZZ_MASK; /* เตรียมค่า LOW สำหรับคำสั่งเปิดเสียงครั้งถัดไป */
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
    /* ให้ PTA1 เป็น Input จนกว่าจะมีคำสั่งแจ้งเตือนด้วยเสียง */
    PTB->PDDR |= LATCH_MASK;
    PTC->PDDR |= CLOCK_MASK;
    PTD->PDDR |= LED_MASK;
    PTB->PDDR &= ~BUTTON_MASK;

    /* IRQC=10: Interrupt เมื่อสัญญาณเปลี่ยนจาก HIGH เป็น LOW (กดปุ่ม) */
    PORTB->PCR[3] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(10);
    PORTB->PCR[10] = PORTB->PCR[3];
    PORTB->PCR[11] = PORTB->PCR[3];
    PORTB->ISFR = BUTTON_MASK;
    NVIC_ClearPendingIRQ(PORTB_IRQn);
    NVIC_SetPriority(PORTB_IRQn, 2U);
    NVIC_EnableIRQ(PORTB_IRQn);
}

/* ##########################################################################
 * [SCORE-7SEG] จอ 7-Segment
 * ส่งข้อมูลผ่าน 74HC595 โดย PIT0_IRQHandler() สแกนหนึ่งหลักต่อการเรียก
 * ########################################################################## */
/* ==========================================================================
 * SECTION 11 | การส่งข้อมูลไปยังจอ 7-Segment
 * เลื่อนข้อมูลทีละบิตเข้าสู่ IC 74HC595
 * ==========================================================================  */
/* หน่วงระดับรอบคำสั่งสั้น ๆ เพื่อให้ DATA/CLOCK ของ 74HC595 คงที่
 * ไม่ใช่การจับเวลานับถอยหลัง เพราะ PIT เป็นฐานเวลา */
/* FUNCTION: shift_settle
 * หน้าที่: หน่วงสั้น ๆ เพื่อให้สัญญาณ DATA และ CLOCK ของจอคงที่
 */
static void shift_settle(void)
{
    uint32_t i;
    for (i = 0U; i < 16U; ++i) __NOP();
}

/* FUNCTION: shift_byte
 * หน้าที่: ส่งข้อมูลหนึ่งไบต์เข้า Shift Register โดยเริ่มจากบิตสูงสุด (MSB)
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
 * หน้าที่: เลื่อนและ Latch ข้อมูล Segment กับข้อมูลเลือกหลัก
 */
static void display_write(uint8_t segments, uint8_t digit)
{
    PTB->PCOR = LATCH_MASK;
    shift_byte(segments); /* ส่งไบต์ Segment ก่อนตามตัวอย่าง Shield */
    shift_byte(digit);
    shift_settle();
    PTB->PSOR = LATCH_MASK;
    shift_settle();
}

/* ##########################################################################
 * [SCORE-ONOFF] LED แสดงความคืบหน้า
 * ใช้ LED บน Shield แสดงจำนวนรอบที่สำเร็จ
 * ########################################################################## */
/* ==========================================================================
 * SECTION 12 | LED แสดงจำนวนรอบที่สำเร็จ
 * เปิด LED D1 ถึง D4 ตามจำนวนรอบที่ทำสำเร็จ
 * ==========================================================================  */
/* FUNCTION: leds_write
 * หน้าที่: อัปเดต LED แบบ Active Low ทั้ง 4 ดวงตามจำนวนรอบที่สำเร็จ
 */
static void leds_write(uint8_t count)
{
    uint32_t i, on = 0U;
    for (i = 0U; i < count && i < 4U; ++i) on |= BIT(led_pin[i]);
    PTD->PSOR = LED_MASK & ~on; /* Active Low: HIGH ทำให้ LED ดับ */
    PTD->PCOR = on;
}

/* ##########################################################################
 * [REQ-2] เงื่อนไขบังคับ: INTERRUPT
 * ใช้ GPIO Interrupt สำหรับปุ่มกด และ PIT0 Timer Interrupt
 * ########################################################################## */
/* ==========================================================================
 * SECTION 13 | ฟังก์ชันบริการ Interrupt (ISR)
 * PORTB รับเหตุการณ์ปุ่ม ส่วน PIT0 เพิ่มเวลา สแกนจอ และควบคุมเสียง
 * ==========================================================================  */
/* FUNCTION: PORTB_IRQHandler
 * หน้าที่: บันทึกเหตุการณ์ปุ่มแบบ Falling Edge และล้าง Interrupt Flag
 */
void PORTB_IRQHandler(void)
{
    uint32_t flags = PORTB->ISFR & BUTTON_MASK;
    PORTB->ISFR = flags; /* เขียน 1 เพื่อล้าง Interrupt Flag */
    if (flags & BIT(3)) button_edges |= EV_S1;
    if (flags & BIT(10)) button_edges |= EV_S2;
    if (flags & BIT(11)) button_edges |= EV_S3;
    /* ไม่รอ Debounce ใน ISR เพราะจะทำให้การบริการ Timer ล่าช้า */
    __DSB();
}

/* FUNCTION: PIT0_IRQHandler
 * หน้าที่: เพิ่มตัวนับ milliseconds สแกนจอหนึ่งหลัก และอัปเดตเสียง Buzzer
 */
void PIT0_IRQHandler(void)
{
    static uint8_t digit;
    uint32_t frame;
    uint16_t key_beep;
    uint8_t sound_on;
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    ++milliseconds;
    frame = display_frame; /* อ่านข้อมูลจอทั้ง 4 หลักพร้อมกันด้วยการอ่าน 32 บิตหนึ่งครั้ง */
    display_write(0xFFU, 0xF0U); /* ดับทุกหลักก่อนสลับหลักเพื่อลดภาพซ้อน (Ghosting) */
    display_write((uint8_t)(frame >> (digit * 8U)), digit_select[digit]);
    digit = (uint8_t)((digit + 1U) & 3U);
    /* เสียงกดปุ่มมีสิทธิ์แทรกเสียงแจ้งสถานะเดิมชั่วคราว
     * OFF (ก่อนเสียง) -> ON (60 ms) -> OFF (หลังเสียง) แล้วกลับสู่เสียงแจ้งเตือนปกติ
     * ป้องกันเสียงคลิกกลืนกับเสียงแจ้งเตือนที่กำลังดัง
     * PIT0 ควบคุมพัลส์เสียง จึงไม่ถูกการอัปเดต LCD ยืดระยะเวลา
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
        /* Passive Buzzer: สลับ LOW กับ Input ที่ปล่อยขา ที่ความถี่ 500 Hz */
        buzzer_set((uint8_t)(milliseconds & 1U));
#endif
    } else buzzer_set(0U);
    __DSB();
}

/* ##########################################################################
 * [REQ-1] เงื่อนไขบังคับ: TIMER / COUNTER
 * ใช้ PIT0 ฮาร์ดแวร์สร้างฐานเวลา 1 มิลลิวินาที
 * ########################################################################## */
/* ==========================================================================
 * SECTION 14 | การตั้งค่า PIT Timer
 * คำนวณความถี่ Bus Clock และสร้าง PIT0 Interrupt ทุก 1 ms
 * ==========================================================================  */
/* FUNCTION: timer_init
 * หน้าที่: เปิด PIT0 Timer และ Interrupt สำหรับสร้างฐานเวลา
 */
static void timer_init(void)
{
    uint32_t core_div, bus_div, bus_hz;
    SystemCoreClockUpdate();
    core_div = ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> SIM_CLKDIV1_OUTDIV1_SHIFT) + 1U;
    bus_div = ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> SIM_CLKDIV1_OUTDIV2_SHIFT) + 1U;
    /* PIT ใช้ Bus Clock ไม่ใช่ Core Clock จึงอ่านตัวหารแทนการสมมติ 60 MHz */
    bus_hz = (SystemCoreClock * core_div) / bus_div;
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT->MCR = PIT_MCR_FRZ_MASK; /* เปิด PIT และหยุดการนับเมื่อ Debugger หยุดโปรแกรม */
    PIT->CHANNEL[0].TCTRL = 0U;
    PIT->CHANNEL[0].LDVAL = bus_hz / 1000U - 1U;
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    NVIC_ClearPendingIRQ(PIT0_IRQn);
    NVIC_SetPriority(PIT0_IRQn, 1U); /* การรีเฟรชจอมี Priority สูงกว่าปุ่มกด */
    NVIC_EnableIRQ(PIT0_IRQn);
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
}

/* ##########################################################################
 * [SCORE-LCD] อุปกรณ์เพิ่มเติม: LCD
 * LCD ใช้ Software I2C ผ่าน D1/PTC17 และ D0/PTC16
 * ########################################################################## */
/* ==========================================================================
 * SECTION 15 | การสื่อสารกับ LCD ผ่าน Software I2C
 * ใช้ D1/PTC17 เป็น SDA และ D0/PTC16 เป็น SCL ผ่านช่อง Wireless ของ Shield
 * ==========================================================================  */
/* Software I2C ผ่านช่อง Wireless ของ Shield: D0/PTC16=SCL, D1/PTC17=SDA ใช้ I2C Level Shifter 3.3V/5V
 * การรอมีขอบเขตจำกัด เมื่อไม่มี LCD จะไม่ทำให้ Timer หยุด
 * ขา Backpack ทั่วไป: RS=P0, RW=P1, EN=P2, ไฟพื้นหลัง=P3
 */
#define LCD_SCL (1UL << 16)
#define LCD_SDA (1UL << 17)
static uint32_t lcd_delay_count;
static volatile uint8_t lcd_online, lcd_address; /* ตรวจค่าผ่าน Keil Watch ได้เมื่อจำเป็น */
static char lcd_sent[LCD_ROWS][LCD_COLS];
static uint32_t lcd_scan;

/* FUNCTION: bus_delay
 * หน้าที่: หน่วงสั้น ๆ เพื่อกำหนดจังหวะ Software I2C
 */
static void bus_delay(void)
{
    volatile uint32_t n;
    for (n = 0U; n < lcd_delay_count; ++n) __NOP();
}

/* FUNCTION: bus_low
 * หน้าที่: ขับสายสัญญาณ I2C ที่เลือกให้เป็น LOW
 */
static void bus_low(uint32_t mask)
{
    PTC->PCOR = mask;
    PTC->PDDR |= mask;
}

/* FUNCTION: bus_release
 * หน้าที่: ปล่อยขา I2C เป็น Input เพื่อให้ Pull-up ภายนอกดึงขึ้นเป็น HIGH
 */
static void bus_release(uint32_t mask) { PTC->PDDR &= ~mask; }

/* FUNCTION: clock_high
 * หน้าที่: ปล่อย SCL และรอให้เป็น HIGH โดยจำกัดจำนวนครั้งที่รอ
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
 * หน้าที่: สร้างเงื่อนไข I2C STOP
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
 * หน้าที่: สร้างเงื่อนไข I2C START เมื่อปล่อยสายและบัสพร้อม
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
 * หน้าที่: ส่งข้อมูล 8 บิตและตรวจสัญญาณตอบรับ ACK จากอุปกรณ์
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
 * หน้าที่: ส่ง Address และข้อมูลไปยัง I2C Adapter ของ LCD
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
 * หน้าที่: แปลงข้อมูล LCD หนึ่งไบต์เป็นการส่ง 4 บิตสองครั้ง พร้อมพัลส์ Enable
 */
static void lcd_pack(uint8_t *data, uint8_t value, uint8_t rs)
{
    uint8_t high = (uint8_t)((value & 0xF0U) | 0x08U | rs);
    uint8_t low = (uint8_t)((value << 4) | 0x08U | rs);
    data[0] = high; data[1] = high | 0x04U; data[2] = high;
    data[3] = low; data[4] = low | 0x04U; data[5] = low;
}

/* FUNCTION: lcd_command
 * หน้าที่: ส่งคำสั่งหนึ่งไบต์ไปยัง LCD
 */
static uint8_t lcd_command(uint8_t command)
{
    uint8_t data[6];
    lcd_pack(data, command, 0U);
    return lcd_transfer(data, 6U);
}

/* ใช้เฉพาะตอนเริ่มระบบ โดยยังเปิด PIT Interrupt */
/* FUNCTION: lcd_boot_wait
 * หน้าที่: รอช่วงเริ่มต้น LCD โดยยังเปิดใช้งาน PIT Interrupt
 */
static void lcd_boot_wait(uint32_t ms)
{
    uint32_t start = milliseconds;
    while ((uint32_t)(milliseconds - start) < ms) __NOP();
}

/* FUNCTION: lcd_init
 * หน้าที่: ค้นหา I2C Adapter และตั้งค่า LCD ให้ทำงานแบบ 4 บิต
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
    /* ตรวจสอบ Address เท่านั้น ไม่ส่งคำสั่ง LCD ไปยังช่วง Address อื่น */
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
 * หน้าที่: ส่งข้อมูลเฉพาะตำแหน่งตัวอักษรที่เปลี่ยน ไม่เกินหนึ่งตำแหน่งต่อการเรียก
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
 * [REQ-3] เงื่อนไขบังคับ: แยกฟังก์ชันย่อย
 * main() ประสานงาน ส่วนรายละเอียดอุปกรณ์อยู่ในฟังก์ชันเฉพาะด้านบน
 * ########################################################################## */
/* ==========================================================================
 * SECTION 16 | ฟังก์ชัน Main และการประสานงานระบบ
 * ตั้งค่าฮาร์ดแวร์ ตรวจปุ่ม อัปเดตสถานะ และแสดงผล
 * ==========================================================================  */
/* FUNCTION: main
 * หน้าที่: ประสานฟังก์ชันของระบบใน Main Loop
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
    lcd_init(); /* เรียกครั้งเดียวตอนเปิดเครื่อง โดย IRQ ยังรีเฟรชจอ 7-Segment */
    app.last_ms = milliseconds;
    previous = milliseconds;

    for (;;) {
        now = milliseconds;
        if (now == previous) continue; /* ทำงาน Main Loop ไม่เกินหนึ่งครั้งต่อ Tick */
        previous = now;

        /* อ่านและล้าง Flag แบบ Atomic เพื่อไม่ให้เหตุการณ์ Interrupt สูญหาย */
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
        /* หากหมดเวลาและเปลี่ยนสถานะในรอบนี้ ไม่ใช้คำสั่งปุ่มเดิมกับหน้าประเมินผล */
        if (app.state != previous_state) events = 0U;
        /* หากเกิดหลายเหตุการณ์พร้อมกัน ให้เลือกหนึ่งคำสั่ง โดยคำสั่งกดยกเลิกสำคัญสูงสุด */
        if (events & EV_S3_LONG) app_on_event(EV_S3_LONG);
        else if (events & EV_S1) app_on_event(EV_S1);
        else if (events & EV_S2) app_on_event(EV_S2);
        else if (events & EV_S3) app_on_event(EV_S3);

        leds_write(app.rounds);
        display_frame = app_display(now); /* เผยแพร่ข้อมูลจอด้วยการเขียน 32 บิตหนึ่งครั้ง */
        buzzer_gate = (uint8_t)(app.sound_ms != 0U && (app.sound_ms / 100U) % 2U == 0U);
        lcd_render(now);
        lcd_service(); /* ส่งตัวอักษรที่เปลี่ยนหนึ่งตำแหน่งต่อครั้ง ไม่เรียกจาก ISR */
    }
}
#endif /* จบเงื่อนไข FOCUSQUEST_HOST_TEST */
