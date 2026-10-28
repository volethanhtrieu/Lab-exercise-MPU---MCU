#include "main.h"
#include "input_reading.h"
#include "software_timer.h"

/* Held for 1 s, expressed in interrupts (independent of TIMER_CYCLE) */
#define DURATION_FOR_1S     (1000 / TIMER_CYCLE)

#define BUTTON_IS_PRESSED   GPIO_PIN_RESET
#define BUTTON_IS_RELEASED  GPIO_PIN_SET

/* Port and pin of each button, in index order */
static GPIO_TypeDef *const button_port[N0_OF_BUTTONS] =
    { sw0_GPIO_Port, sw1_GPIO_Port, sw2_GPIO_Port };
static const uint16_t button_pin[N0_OF_BUTTONS] =
    { sw0_Pin, sw1_Pin, sw2_Pin };

/* Final debounced state. Start as RELEASED to avoid a false press. */
static GPIO_PinState buttonBuffer[N0_OF_BUTTONS] =
    { BUTTON_IS_RELEASED, BUTTON_IS_RELEASED, BUTTON_IS_RELEASED };

/* The two most recent raw readings */
static GPIO_PinState debounceButtonBuffer1[N0_OF_BUTTONS] =
    { BUTTON_IS_RELEASED, BUTTON_IS_RELEASED, BUTTON_IS_RELEASED };
static GPIO_PinState debounceButtonBuffer2[N0_OF_BUTTONS] =
    { BUTTON_IS_RELEASED, BUTTON_IS_RELEASED, BUTTON_IS_RELEASED };

/* Long-press detection */
static uint8_t  flagForButtonPress1s[N0_OF_BUTTONS];
static uint16_t counterForButtonPress1s[N0_OF_BUTTONS];

/* Called every TIMER_CYCLE ms from the TIM2 interrupt */
void button_reading(void)
{
    for (uint8_t i = 0; i < N0_OF_BUTTONS; i++) {
        debounceButtonBuffer2[i] = debounceButtonBuffer1[i];
        debounceButtonBuffer1[i] = HAL_GPIO_ReadPin(button_port[i],
                                                    button_pin[i]);

        /* Accept the new level only if two readings agree */
        if (debounceButtonBuffer1[i] == debounceButtonBuffer2[i]) {
            buttonBuffer[i] = debounceButtonBuffer1[i];
        }

        if (buttonBuffer[i] == BUTTON_IS_PRESSED) {
            if (counterForButtonPress1s[i] < DURATION_FOR_1S) {
                counterForButtonPress1s[i]++;
            } else {
                flagForButtonPress1s[i] = 1;
            }
        } else {
            counterForButtonPress1s[i] = 0;
            flagForButtonPress1s[i] = 0;
        }
    }
}

uint8_t is_button_pressed(uint8_t index)
{
    if (index >= N0_OF_BUTTONS) return 0;
    return (buttonBuffer[index] == BUTTON_IS_PRESSED);
}

uint8_t is_button_pressed_1s(uint8_t index)
{
    if (index >= N0_OF_BUTTONS) return 0;
    return (flagForButtonPress1s[index] == 1);
}