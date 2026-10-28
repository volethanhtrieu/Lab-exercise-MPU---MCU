#include "main.h"
#include "traffic_light.h"
#include "input_processing.h"
#include "led_display.h"
#include "software_timer.h"

#define ONE_SECOND_MS 1000

/* ---------- FSM C: traffic light (Exercise 1) ---------- */
enum TrafficState {
    S0_H_RED_V_GREEN,
    S1_H_RED_V_AMBER,
    S2_H_GREEN_V_RED,
    S3_H_AMBER_V_RED
};

static enum TrafficState state = S0_H_RED_V_GREEN;
static uint8_t count = 0;   /* seconds left in the current state */

/* Each road shows the time left of its own light.
   A red road waits for the other road's green and amber,
   so during green its red countdown is count + T_amber. */
static void show_countdown(void)
{
    switch (state) {
    case S0_H_RED_V_GREEN:
        set7SegH(count + get_time_amber());   /* red   */
        set7SegV(count);                      /* green */
        break;
    case S1_H_RED_V_AMBER:
        set7SegH(count);                      /* red   */
        set7SegV(count);                      /* amber */
        break;
    case S2_H_GREEN_V_RED:
        set7SegH(count);                      /* green */
        set7SegV(count + get_time_amber());   /* red   */
        break;
    case S3_H_AMBER_V_RED:
        set7SegH(count);                      /* amber */
        set7SegV(count);                      /* red   */
        break;
    }
}

/* Entry action of a state: load count and set the lights */
static void enter_state(enum TrafficState next)
{
    state = next;

    switch (state) {
    case S0_H_RED_V_GREEN:
        count = get_time_green();
        setLightH(LIGHT_RED);
        setLightV(LIGHT_GREEN);
        break;
    case S1_H_RED_V_AMBER:
        count = get_time_amber();
        setLightH(LIGHT_RED);
        setLightV(LIGHT_AMBER);
        break;
    case S2_H_GREEN_V_RED:
        count = get_time_green();
        setLightH(LIGHT_GREEN);
        setLightV(LIGHT_RED);
        break;
    case S3_H_AMBER_V_RED:
        count = get_time_amber();
        setLightH(LIGHT_AMBER);
        setLightV(LIGHT_RED);
        break;
    }

    show_countdown();
}

void traffic_init(void)
{
    enter_state(S0_H_RED_V_GREEN);
    setTimer(TIMER_SECOND, ONE_SECOND_MS);
}

void traffic_run(void)
{
    if (!isTimerExpired(TIMER_SECOND)) {
        return;                        /* wait for the next second */
    }
    setTimer(TIMER_SECOND, ONE_SECOND_MS);

    count--;                           /* count > 0 / count-- (1 s) */

    if (count > 0) {
        show_countdown();
        return;
    }

    /* count = 0: go to the next state */
    switch (state) {
    case S0_H_RED_V_GREEN: enter_state(S1_H_RED_V_AMBER); break;
    case S1_H_RED_V_AMBER: enter_state(S2_H_GREEN_V_RED); break;
    case S2_H_GREEN_V_RED: enter_state(S3_H_AMBER_V_RED); break;
    case S3_H_AMBER_V_RED: enter_state(S0_H_RED_V_GREEN); break;
    }
}