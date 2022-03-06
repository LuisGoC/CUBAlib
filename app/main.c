#include "stm32g0xx.h"
#include "app_bsp.h"
#include "CUBA.h" //<-- Include CUBA library

/* Function prototypes */
void UART_init(void);
void CUBA_init(void);

/* Global structures */
CUBA_HandleTypeDef      CUBA_Handle;                     //Structure type variable that contains the configuration information for the CUBA library
UART_HandleTypeDef      UART_struct;                     //Structure type variable that contains the configuration information for the specified UART 
FDCAN_HandleTypeDef     CUBA_CAN_Structure;              //Structure type variable that contains the configuration information for the specified CAN
FDCAN_TxHeaderTypeDef   CUBA_TxHeader_CAN_Structure;     //Structure type variable that contains the configuration information for the CAN Tx Header
FDCAN_RxHeaderTypeDef   CUBA_RxHeader_CAN_Structure;     //Structure type variable that contains the configuration information for the CAN Rx Header
FDCAN_FilterTypeDef     CUBA_Filter_CAN_Structure;       //Structure type variable that contains the configuration information for the CAN Filter 
DMA_HandleTypeDef       CUBA_DMA_Structure;              //Structure type variable that contains the configuration information for the specified DMA

uint8_t uart_rx_byte; //UART reception variable

int main( void )
{
    /* Initialization function calls */
    HAL_Init();
    UART_init();
    CUBA_init();

    /* Receive an amount of data in interrupt mode. */
    (void)HAL_UART_Receive_IT(&UART_struct, &uart_rx_byte, 1);
    for( ; ; )
    {
        /* Add your code here */
    }

    return 0u;
}

/**
  * @brief UART initialization function.
  * @param none
  * @retval None
  */
void UART_init(void)
{
    UART_struct.Instance            = USART2;
    UART_struct.Init.BaudRate       = 115200;
    UART_struct.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    UART_struct.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
    UART_struct.Init.Mode           = UART_MODE_TX_RX;
    UART_struct.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLED;
    UART_struct.Init.OverSampling   = UART_OVERSAMPLING_16;
    UART_struct.Init.Parity         = UART_PARITY_NONE;
    UART_struct.Init.StopBits       = UART_STOPBITS_1;
    UART_struct.Init.WordLength     = UART_WORDLENGTH_8B;
    (void)HAL_UART_Init(&UART_struct);
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    /* CUBA library stores the Rx data */
    MOD_CUBA_GetUartData(huart, uart_rx_byte);  

    /* Receive an amount of data in interrupt mode. */
    (void)HAL_UART_Receive_IT(&UART_struct, &uart_rx_byte, 1);
}

/**
  * @brief Tx Transfer completed callback.
  * @param huart UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    /* CUBA library confirms the Tx Transfer is completed */
    MOD_CUBA_GetUartTxCpltFlag( huart );
}

/**
  * @brief CUBA library initialization function.
  * @param none
  * @retval None
  */
void CUBA_init(void)
{
    CUBA_Handle.UARTHandler     = &UART_struct;                     // Initialized by user
    CUBA_Handle.CANHandler      = &CUBA_CAN_Structure;              // Initialized by CUBA library
    CUBA_Handle.CANRxHeader     = &CUBA_RxHeader_CAN_Structure;     // Initialized by CUBA library
    CUBA_Handle.CANTxHeader     = &CUBA_TxHeader_CAN_Structure;     // Initialized by CUBA library
    CUBA_Handle.CANFilterHeader = &CUBA_Filter_CAN_Structure;       // Initialized by CUBA library
    CUBA_Handle.DMAHandler      = &CUBA_DMA_Structure;              // Initialized by CUBA library
    MOD_CUBA_Init(&CUBA_Handle);
}