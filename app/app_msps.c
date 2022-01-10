#include "stm32g0xx.h"
#include <stdint.h>
#include "app_bsp.h"

void HAL_MspInit( void )
{
    
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_struct;

    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio_struct.Alternate = GPIO_AF1_USART2;
    gpio_struct.Mode = GPIO_MODE_AF_PP;
    gpio_struct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_struct.Pull = GPIO_PULLUP;
    gpio_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio_struct);

    HAL_NVIC_SetPriority(USART2_LPUART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_LPUART2_IRQn);
}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)
{
    GPIO_InitTypeDef GpioCanStruct;

    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    if(hfdcan->Instance == FDCAN1)
    {
        GpioCanStruct.Mode = GPIO_MODE_AF_PP;
        GpioCanStruct.Alternate = GPIO_AF3_FDCAN1;
        GpioCanStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        GpioCanStruct.Pull = GPIO_NOPULL;
        GpioCanStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GpioCanStruct);
    }

    if(hfdcan->Instance == FDCAN2)
    {
        GpioCanStruct.Mode = GPIO_MODE_AF_PP;
        GpioCanStruct.Alternate = GPIO_AF3_FDCAN2;
        GpioCanStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GpioCanStruct.Pull = GPIO_NOPULL;
        GpioCanStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GpioCanStruct);
    }
}
