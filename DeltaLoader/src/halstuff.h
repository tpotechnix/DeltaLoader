#ifndef __HALSTUFF_H
#define __HALSTUFF_H

#ifdef __cplusplus
extern "C" {
#endif

extern ADC_HandleTypeDef hadc;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim14;

extern UART_HandleTypeDef huart1;

void halstuffInit(void);

#define POWER_Pin          GPIO_PIN_8
#define POWER_GPIO_Port    GPIOA

/* blue */
#define LED_SBUS_Pin       GPIO_PIN_7
#define LED_SBUS_GPIO_Port GPIOB
/* green */
#define LED_PPM_Pin        GPIO_PIN_8
#define LED_PPM_GPIO_Port  GPIOB
/* red */
#define LED_3RD_Pin        GPIO_PIN_6
#define LED_3RD_GPIO_Port  GPIOB

#define FREE0_Pin          GPIO_PIN_1
#define FREE0_GPIO_Port    GPIOA
#define LIGHTS_Pin         GPIO_PIN_2
#define LIGHTS_GPIO_Port   GPIOA

#ifdef __cplusplus
}
#endif

#endif /* __HALSTUFF_H */