#include "main.h"
#include "led_display.h"

/* Everything is active low: RESET = ON, SET = OFF */

/* ---------------- Traffic LEDs (unchanged from Exercise 5) ---------------- */
void setLightH(uint8_t light)
{
    HAL_GPIO_WritePin(GPIOA, red_h_Pin | yel_h_Pin | grn_h_Pin, GPIO_PIN_SET);

    if (light == LIGHT_RED)   HAL_GPIO_WritePin(red_h_GPIO_Port, red_h_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_AMBER) HAL_GPIO_WritePin(yel_h_GPIO_Port, yel_h_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_GREEN) HAL_GPIO_WritePin(grn_h_GPIO_Port, grn_h_Pin, GPIO_PIN_RESET);
}

void setLightV(uint8_t light)
{
    HAL_GPIO_WritePin(GPIOA, red_v_Pin | yel_v_Pin | grn_v_Pin, GPIO_PIN_SET);

    if (light == LIGHT_RED)   HAL_GPIO_WritePin(red_v_GPIO_Port, red_v_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_AMBER) HAL_GPIO_WritePin(yel_v_GPIO_Port, yel_v_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_GREEN) HAL_GPIO_WritePin(grn_v_GPIO_Port, grn_v_Pin, GPIO_PIN_RESET);
}

/* ---------------- Seven-segment LEDs (Exercise 6) ---------------- */
#define DIGIT_BLANK 10

/* Bit 0 = segment a ... bit 6 = segment g; 1 = segment lit */
static const uint8_t SEG_CODE[11] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,   /* 0 1 2 3 4 */
    0x6D, 0x7D, 0x07, 0x7F, 0x6F,   /* 5 6 7 8 9 */
    0x00                            /* blank     */
};

/* Segment pins a..g of each road (all on GPIOB) */
static const uint16_t seg_pin_h[7] = {
    a_h_Pin, b_h_Pin, c_h_Pin, d_h_Pin, e_h_Pin, f_h_Pin, g_h_Pin };
static const uint16_t seg_pin_v[7] = {
    a_v_Pin, b_v_Pin, c_v_Pin, d_v_Pin, e_v_Pin, f_v_Pin, g_v_Pin };

/* Output buffer: {H tens, H units, V tens, V units} */
static uint8_t led_buffer[4] = {
    DIGIT_BLANK, DIGIT_BLANK, DIGIT_BLANK, DIGIT_BLANK };

static uint8_t scan_step = 0;   /* 0 = tens digits, 1 = units digits */

/* Light the segments of one digit on one bus */
static void writeSegments(const uint16_t pin[7], uint8_t digit)
{
    uint8_t code = SEG_CODE[digit];
    for (uint8_t i = 0; i < 7; i++) {
        if (code & (1 << i)) {
            HAL_GPIO_WritePin(GPIOB, pin[i], GPIO_PIN_RESET);   /* on  */
        } else {
            HAL_GPIO_WritePin(GPIOB, pin[i], GPIO_PIN_SET);     /* off */
        }
    }
}

void set7SegH(uint8_t value)
{
    if (value > 99) {
        led_buffer[0] = DIGIT_BLANK;
        led_buffer[1] = DIGIT_BLANK;
    } else {
        led_buffer[0] = value / 10;    /* tens  */
        led_buffer[1] = value % 10;    /* units */
    }
}

void set7SegV(uint8_t value)
{
    if (value > 99) {
        led_buffer[2] = DIGIT_BLANK;
        led_buffer[3] = DIGIT_BLANK;
    } else {
        led_buffer[2] = value / 10;
        led_buffer[3] = value % 10;
    }
}

/* Called every SCAN_MS: digit 1 of both roads, then digit 2 of both roads */
void scan7Seg(void)
{
    /* 1. Turn all 4 digits off first (no ghosting) */
    HAL_GPIO_WritePin(GPIOA, en0_Pin | en1_Pin | en2_Pin | en3_Pin, GPIO_PIN_SET);

    if (scan_step == 0) {
        /* 2. Tens digits of both roads */
        writeSegments(seg_pin_h, led_buffer[0]);
        writeSegments(seg_pin_v, led_buffer[2]);
        HAL_GPIO_WritePin(GPIOA, en0_Pin | en2_Pin, GPIO_PIN_RESET);
        scan_step = 1;
    } else {
        /* 2. Units digits of both roads */
        writeSegments(seg_pin_h, led_buffer[1]);
        writeSegments(seg_pin_v, led_buffer[3]);
        HAL_GPIO_WritePin(GPIOA, en1_Pin | en3_Pin, GPIO_PIN_RESET);
        scan_step = 0;
    }
}