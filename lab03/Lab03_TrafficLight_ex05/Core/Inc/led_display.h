#ifndef INC_LED_DISPLAY_H_
#define INC_LED_DISPLAY_H_

#include <stdint.h>

/* Colour shown by one road */
enum {
    LIGHT_OFF = 0,
    LIGHT_RED,
    LIGHT_AMBER,
    LIGHT_GREEN
};

void setLightH(uint8_t light);   /* horizontal road: D1-D6  */
void setLightV(uint8_t light);   /* vertical road:   D7-D12 */

#endif /* INC_LED_DISPLAY_H_ */