#ifndef INC_INPUT_PROCESSING_H_
#define INC_INPUT_PROCESSING_H_

#include <stdint.h>

/* Modes of the application */
enum {
    MODE_NORMAL = 1,   /* Mode 1 */
    MODE_RED,          /* Mode 2 */
    MODE_AMBER,        /* Mode 3 */
    MODE_GREEN         /* Mode 4 */
};

void    fsm_for_input_processing(void);
void    display_mode(void);          /* Exercise 6 */
uint8_t get_mode(void);

#endif /* INC_INPUT_PROCESSING_H_ */