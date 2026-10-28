#include "software_timer.h"

/* Remaining time of each timer, in milliseconds */
static volatile int32_t timer_counter[NUM_OF_TIMERS];
/* 1 = the timer has expired */
static volatile uint8_t timer_flag[NUM_OF_TIMERS];

void setTimer(uint8_t index, uint32_t duration_ms)
{
    if (index >= NUM_OF_TIMERS) return;

    int32_t start = (int32_t)duration_ms;

    /* Re-armed after expiring: keep the small overshoot so the
       average period stays exact even if TIMER_CYCLE does not
       divide duration_ms. */
    if (timer_flag[index]) {
        start += timer_counter[index];   /* counter is <= 0 here */
    }

    timer_flag[index]    = 0;
    timer_counter[index] = start;

    if (start <= 0) {
        timer_flag[index] = 1;           /* already due */
    }
}

uint8_t isTimerExpired(uint8_t index)
{
    if (index >= NUM_OF_TIMERS) return 0;
    return timer_flag[index];
}

void timerRun(void)
{
    for (uint8_t i = 0; i < NUM_OF_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i] -= TIMER_CYCLE;
            if (timer_counter[i] <= 0) {
                timer_flag[i] = 1;
            }
        }
    }
}