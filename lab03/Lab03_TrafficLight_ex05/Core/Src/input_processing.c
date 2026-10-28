#include "main.h"
#include "input_reading.h"
#include "input_processing.h"
#include "software_timer.h"

#define AUTO_REPEAT_MS 500

/* ---------- FSM A: one copy per button (Exercise 1) ---------- */
enum ButtonState {
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_PRESSED_MORE_THAN_1_SECOND
};

static enum ButtonState buttonState[N0_OF_BUTTONS];   /* all RELEASED */

/* One-shot events produced by the FSM */
static uint8_t pressEvent[N0_OF_BUTTONS];    /* "output +1" on a new press */
static uint8_t repeatEvent[N0_OF_BUTTONS];   /* "+1 every 500 ms" when held */

static void fsm_button(uint8_t i)
{
    switch (buttonState[i]) {
    case BUTTON_RELEASED:
        if (is_button_pressed(i)) {
            buttonState[i] = BUTTON_PRESSED;
            pressEvent[i] = 1;                         /* output +1 */
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
            repeatEvent[i] = 1;                        /* +1 every 500 ms */
            setTimer(TIMER_BTN0_REPEAT + i, AUTO_REPEAT_MS);
        }
        break;
    }
}

/* Read and clear an event (returns 1 once per event) */
static uint8_t take(uint8_t *event)
{
    uint8_t value = *event;
    *event = 0;
    return value;
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
    }
    repeatEvent[BUTTON_MODE] = 0;   /* holding B1 never changes the mode */
}

uint8_t get_mode(void)
{
    return mode;
}