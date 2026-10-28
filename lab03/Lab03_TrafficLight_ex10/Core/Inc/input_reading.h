#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include <stdint.h>

#define N0_OF_BUTTONS 3

/* Button indexes */
#define BUTTON_MODE  0   /* sw0 - PA11 */
#define BUTTON_INC   1   /* sw1 - PA12 */
#define BUTTON_SET   2   /* sw2 - PA13 */

void    button_reading(void);
uint8_t is_button_pressed(uint8_t index);
uint8_t is_button_pressed_1s(uint8_t index);

#endif /* INC_INPUT_READING_H_ */