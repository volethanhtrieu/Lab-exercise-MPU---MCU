#include "main.h"
#include "led_display.h"

/* LEDs are active low: RESET = ON, SET = OFF */

void setLightH(uint8_t light)
{
    /* Turn the three colours off first */
    HAL_GPIO_WritePin(GPIOA, red_h_Pin | yel_h_Pin | grn_h_Pin,
                      GPIO_PIN_SET);

    if (light == LIGHT_RED)   HAL_GPIO_WritePin(red_h_GPIO_Port, red_h_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_AMBER) HAL_GPIO_WritePin(yel_h_GPIO_Port, yel_h_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_GREEN) HAL_GPIO_WritePin(grn_h_GPIO_Port, grn_h_Pin, GPIO_PIN_RESET);
}

void setLightV(uint8_t light)
{
    HAL_GPIO_WritePin(GPIOA, red_v_Pin | yel_v_Pin | grn_v_Pin,
                      GPIO_PIN_SET);

    if (light == LIGHT_RED)   HAL_GPIO_WritePin(red_v_GPIO_Port, red_v_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_AMBER) HAL_GPIO_WritePin(yel_v_GPIO_Port, yel_v_Pin, GPIO_PIN_RESET);
    if (light == LIGHT_GREEN) HAL_GPIO_WritePin(grn_v_GPIO_Port, grn_v_Pin, GPIO_PIN_RESET);
}