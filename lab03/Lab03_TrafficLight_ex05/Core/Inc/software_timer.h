#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include <stdint.h>

/* TIM2 interrupt period in ms. Must match PSC/ARR in CubeMX. */
#ifndef TIMER_CYCLE
#define TIMER_CYCLE 10
#endif

/* One index per software timer */
enum {
    TIMER_BLINK = 0,      /* 2 Hz blinking (used again in Exercise 6) */
    TIMER_BTN0_REPEAT,    /* auto-repeat of button 0 when held      */
    TIMER_BTN1_REPEAT,    /* auto-repeat of button 1 when held      */
    TIMER_BTN2_REPEAT,    /* auto-repeat of button 2 when held      */
    NUM_OF_TIMERS
};

void    setTimer(uint8_t index, uint32_t duration_ms);
uint8_t isTimerExpired(uint8_t index);
void    timerRun(void);

#endif /* INC_SOFTWARE_TIMER_H_ */