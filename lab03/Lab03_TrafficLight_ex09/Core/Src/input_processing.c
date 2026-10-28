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

/* Exercise 7: B2 adds 1 to the edited value, 99 -> 1 */
static void increase_temp(void)
{
    temp_value++;
    if (temp_value > 99) {
        temp_value = 1;
    }
}

/* Exercise 7: B3 saves the red duration.
   Rule: T_red = T_green + T_amber, so green is adjusted
   and must stay >= 1 (red must be greater than amber). */
static void save_red(void)
{
    if (temp_value > time_amber) {
        time_red   = temp_value;
        time_green = time_red - time_amber;
    } else {
        temp_value = time_red;    /* invalid: show the saved value again */
    }
}

/* Exercise 8: B3 saves the amber duration.
   Red is kept, green is adjusted: T_green = T_red - T_amber,
   so amber must be smaller than red (green >= 1). */
static void save_amber(void)
{
    if (temp_value < time_red) {
        time_amber = temp_value;
        time_green = time_red - time_amber;
    } else {
        temp_value = time_amber;  /* invalid: show the saved value again */
    }
}

/* Exercise 9: B3 saves the green duration.
   Amber is kept, red is adjusted: T_red = T_green + T_amber,
   so red must still fit in two digits (red <= 99). */
static void save_green(void)
{
    if (temp_value + time_amber <= 99) {
        time_green = temp_value;
        time_red   = time_green + time_amber;
    } else {
        temp_value = time_green;  /* invalid: show the saved value again */
    }
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

    /* Exercise 7: read B2 and B3 in every mode, so no old event is
       left over when an edit mode is entered */
    uint8_t inc = take(&pressEvent[BUTTON_INC]);
    inc |= take(&repeatEvent[BUTTON_INC]);    /* held > 1 s: +1 every 500 ms */
    uint8_t set = take(&pressEvent[BUTTON_SET]);
    repeatEvent[BUTTON_SET] = 0;              /* holding B3 saves only once */

    /* B2: increase the draft in the three edit modes */
    if (inc && (mode == MODE_RED || mode == MODE_AMBER || mode == MODE_GREEN)) {
        increase_temp();
    }

    /* B3: save the draft into the duration of the selected colour */
    if (set) {
        if (mode == MODE_RED)   save_red();      /* Exercise 7 */
        if (mode == MODE_AMBER) save_amber();    /* Exercise 8 */
        if (mode == MODE_GREEN) save_green();    /* Exercise 9 */
    }
}

uint8_t get_mode(void)
{
    return mode;
}

/* ---------- Exercise 6: display the mode ---------- */
static uint8_t shown_mode  = 0;  /* 0 = nothing shown yet */
static uint8_t shown_value = 0;  /* value now on road H (Exercise 7) */
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
        shown_value = temp_value;

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

    /* Exercise 7: the value was changed by B2 (or restored by B3) */
    if (mode != MODE_NORMAL && temp_value != shown_value) {
        shown_value = temp_value;
        set7SegH(temp_value);
    }
}