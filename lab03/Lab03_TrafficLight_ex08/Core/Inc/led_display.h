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

/* Value > 99 turns the two digits of a road off */
#define SEG_BLANK 0xFF

void setLightH(uint8_t light);   /* horizontal road: D1-D6  */
void setLightV(uint8_t light);   /* vertical road:   D7-D12 */

void set7SegH(uint8_t value);    /* 2 digits of road H (en0, en1) */
void set7SegV(uint8_t value);    /* 2 digits of road V (en2, en3) */
void scan7Seg(void);             /* show the next digit pair      */

#endif /* INC_LED_DISPLAY_H_ */