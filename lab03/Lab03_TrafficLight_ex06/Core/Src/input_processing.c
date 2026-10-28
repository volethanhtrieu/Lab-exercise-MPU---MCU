#include "main.h"
#include "input_reading.h"
#include "input_processing.h"
#include "software_timer.h"
#include "led_display.h"

#define AUTO_REPEAT_MS 500
#define BLINK_MS       250     /* toggle every 250 ms = 2 Hz */

/* ---------- FSM A: one copy per button (unchanged from Exercise 5) ---------- */
enum ButtonState {
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_PRESSED_MORE_THAN_1_SECOND
};

static enum ButtonState buttonState[N0_OF_BUTTONS];
static uint8_t pressEvent[N0_OF_BUTTONS];
static uint8_t repeatEvent[N0_OF_BUTTONS];

static void fsm_button(uint8_t i)
{
    switch (buttonState[i]) {
    case BUTTON_RELEASED:
        if (is_button_pressed(i)) {
            buttonState[i] = BUTTON_PRESSED;
            pressEvent[i] = 1;
        }
        break;

    case BUTTON_PRESSED:
        if (!is_button_pressed(i)) {
            buttonState[i] = BUTTON_RELEASED;
        } else if (is_button_pressed_1s(i)) {
            buttonState[i] = BUTTON_PRESSED_MORE_THAN_1_SECOND;
            setTimer(TIMER_BTN0_REPEAT + i, AUTO_REPEAT_MS);
        }
        break;

    case BUTTON_PRESSED_MORE_THAN_1_SECOND:
        if (!is_button_pressed(i)) {
            buttonState[i] = BUTTON_RELEASED;
        } else if (isTimerExpired(TIMER_BTN0_REPEAT + i)) {
            repeatEvent[i] = 1;
            setTimer(TIMER_BTN0_REPEAT + i, AUTO_REPEAT_MS);
        }
        break;
    }
}

static uint8_t take(uint8_t *event)
{
    uint8_t value = *event;
    *event = 0;
    return value;
}

/* ---------- Durations (seconds) and the edited value ---------- */
static uint8_t time_red   = 5;
static uint8_t time_amber = 2;
static uint8_t time_green = 3;
static uint8_t temp_value = 0;   /* value shown/edited in Modes 2-4 */

/* Copy the duration of the selected colour into temp_value */
static void load_temp(void)
{
    if (get_mode() == MODE_RED)   temp_value = time_red;
    if (get_mode() == MODE_AMBER) temp_value = time_amber;
    if (get_mode() == MODE_GREEN) temp_value = time_green;
}

/* ---------- FSM B: mode selection ---------- */
static uint8_t mode = MODE_NORMAL;

void fsm_for_input_processing(void)
{
    for (uint8_t i = 0; i < N0_OF_BUTTONS; i++) {
        fsm_button(i);
    }

    /* B1: one press = next mode, 1 -> 2 -> 3 -> 4 -> 1 */
    if (take(&pressEvent[BUTTON_MODE])) {
        mode++;
        if (mode > MODE_GREEN) {
            mode = MODE_NORMAL;
        }
        load_temp();              /* Exercise 6: value of the new mode */
    }
    repeatEvent[BUTTON_MODE] = 0;
}

uint8_t get_mode(void)
{
    return mode;
}

/* ---------- Exercise 6: display the mode ---------- */
static uint8_t shown_mode = 0;   /* 0 = nothing shown yet */
static uint8_t blink_on   = 0;   /* 1 = blinking LEDs are on now */

/* Colour that blinks in the current mode */
static uint8_t mode_colour(void)
{
    if (mode == MODE_RED)   return LIGHT_RED;
    if (mode == MODE_AMBER) return LIGHT_AMBER;
    if (mode == MODE_GREEN) return LIGHT_GREEN;
    return LIGHT_OFF;             /* Mode 1: traffic FSM added in Exercise 10 */
}

static void update_lights(void)
{
    uint8_t light = blink_on ? mode_colour() : LIGHT_OFF;
    setLightH(light);
    setLightV(light);
}

void display_mode(void)
{
    if (mode != shown_mode) {
        /* A new mode was entered: update everything once */
        shown_mode = mode;

        if (mode == MODE_NORMAL) {
            set7SegH(SEG_BLANK);     /* no value in Mode 1 (yet) */
        } else {
            set7SegH(temp_value);    /* road H: the value  */
        }
        set7SegV(mode);              /* road V: the mode, e.g. 02 */

        blink_on = 1;                /* start with the LEDs on */
        setTimer(TIMER_BLINK, BLINK_MS);
        update_lights();
    }
    else if (mode != MODE_NORMAL && isTimerExpired(TIMER_BLINK)) {
        /* Same mode: toggle the colour every 250 ms (2 Hz) */
        blink_on = !blink_on;
        setTimer(TIMER_BLINK, BLINK_MS);
        update_lights();
    }
}