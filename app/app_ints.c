#include "stm32g0xx.h"
#include <stdint.h>
#include "app_bsp.h"

extern UART_HandleTypeDef UART_struct; 
extern FDCAN_HandleTypeDef CUBA_CAN_Structure; 
extern DMA_HandleTypeDef CUBA_DMA_Structure;

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void NMI_Handler( void )
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void HardFault_Handler( void )
{
    assert_param( 0u );
}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void SVC_Handler( void )
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void PendSV_Handler( void )
{

}

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
void SysTick_Handler( void )
{
    HAL_IncTick( );
}

void USART2_LPUART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&UART_struct);
}

void TIM17_FDCAN_IT1_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&CUBA_CAN_Structure);
}

void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&CUBA_DMA_Structure);
}