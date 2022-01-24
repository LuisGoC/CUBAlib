#include "stm32g0xx.h"
#include "app_bsp.h"
#include "CUBA.h"

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
/* Prototype Functions*/
void CAN1_init(void);
void GPIO_init(void);
void UART_init(void);
void CAN1_Tx(void);
void CAN1_Rx(void);
void CAN1_transmits(void);
void CUBA_init(void);



/* Global structures */
//Uart structs
UART_HandleTypeDef UART_struct; //Uart structure
//FDCAN structs
FDCAN_HandleTypeDef CAN1_struct; //structure with CAN controller settings
FDCAN_TxHeaderTypeDef TxHeader1; //Data structure with transmission message settings
FDCAN_RxHeaderTypeDef RxHeader1; //Data structure with reception message settings
FDCAN_HandleTypeDef CAN2_struct; //structure with CAN controller settings
FDCAN_TxHeaderTypeDef TxHeader2; //Data structure with transmission message settings
FDCAN_RxHeaderTypeDef RxHeader2; //Data structure with reception message settings
FDCAN_FilterTypeDef Filter2_struct;
DMA_HandleTypeDef hdma_usart2_tx;
uint8_t msgBuffer[8];


CUBA_HandleTypeDef CUBA_Handle;

/* Global variables */
uint8_t CAN1_tx_message[8] = {'R', 'E', 'C', 'E', 'I', 'V', 'E', 'D'};
uint8_t CAN1_rx_message[10] = {0};
uint8_t CAN2_rx_message[10] = {0};
uint8_t uart_rx_byte;
uint8_t uart_rx_buffer[10] = {0};
uint8_t TxIndex;
uint8_t *tok[2] = {0};

__IO uint8_t uart_rx_cplt_flag = RESET;

int main( void )
{
    uint32_t tick1 = 0;
    uint32_t tick2 = 0;
    uint32_t tick3 = 0;

    HAL_Init();
    GPIO_init();
    UART_init();
    CAN1_init();
    CAN1_Tx();
    CUBA_init();

    tick1 = HAL_GetTick();
    tick2 = HAL_GetTick();
    tick3 = HAL_GetTick();

    (void)HAL_FDCAN_Start(&CAN1_struct); //FDCAN1 leaves initialization mode and starts normal mode
    (void)HAL_UART_Receive_IT(&UART_struct, &uart_rx_byte, 1);
    for( ; ; )
    {
        if((HAL_GetTick() - tick1) >= 1000)
        {
            tick1 = HAL_GetTick();
            CAN1_transmits();
        }
        if((HAL_GetTick() - tick2) >= 1000)
        {
            tick2 = HAL_GetTick();
            MOD_CUBA_PeriodicTask(&CUBA_Handle);
        }
        if((HAL_GetTick() - tick3) >= 50)
        {
            tick3 = HAL_GetTick();
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
        }
    }

    return 0u;
}

void GPIO_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

     __HAL_RCC_GPIOA_CLK_ENABLE();
     __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin   = GPIO_PIN_5 | GPIO_PIN_6;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
    HAL_GPIO_WritePin(GPIOC, (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2), RESET);
}

void UART_init(void)
{
    UART_struct.Instance = USART2;
    UART_struct.Init.BaudRate = 115200;
    UART_struct.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    UART_struct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UART_struct.Init.Mode = UART_MODE_TX_RX;
    UART_struct.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLED;
    UART_struct.Init.OverSampling = UART_OVERSAMPLING_16;
    UART_struct.Init.Parity = UART_PARITY_NONE;
    UART_struct.Init.StopBits = UART_STOPBITS_1;
    UART_struct.Init.WordLength = UART_WORDLENGTH_8B;
    (void)HAL_UART_Init(&UART_struct);
}

void CAN1_init(void)
{
    /* SysClock/Pres = 48Mhz */
    CAN1_struct.Instance = FDCAN1; 
    CAN1_struct.Init.Mode = FDCAN_MODE_NORMAL;
    CAN1_struct.Init.AutoRetransmission = ENABLE;
    CAN1_struct.Init.ClockDivider = FDCAN_CLOCK_DIV1;
    CAN1_struct.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    CAN1_struct.Init.TransmitPause = DISABLE;
    CAN1_struct.Init.ExtFiltersNbr = 0;
    CAN1_struct.Init.ProtocolException = DISABLE;
    CAN1_struct.Init.StdFiltersNbr = 0;
    CAN1_struct.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    CAN1_struct.Init.NominalPrescaler = 30;
    CAN1_struct.Init.NominalSyncJumpWidth = 1;
    CAN1_struct.Init.NominalTimeSeg1 = 13;
    CAN1_struct.Init.NominalTimeSeg2 = 2;
    (void)HAL_FDCAN_Init(&CAN1_struct);
}

void CAN1_Tx(void)
{
    TxHeader1.DataLength = FDCAN_DLC_BYTES_8; //Data frame size of 8 bytes
    TxHeader1.Identifier = 0x1FF;             // 11 bit identifier  (Lower value, higher priority)
    TxHeader1.IdType = FDCAN_STANDARD_ID;     // standar Id (11 bits)
    TxHeader1.FDFormat = FDCAN_CLASSIC_CAN;   
    TxHeader1.TxFrameType = FDCAN_DATA_FRAME; //DATA FRAME (It also can be remote frame RTR = 1)
    TxHeader1.BitRateSwitch = FDCAN_BRS_OFF;  
    TxHeader1.TxEventFifoControl = FDCAN_NO_TX_EVENTS; 
}

void CAN1_Rx(void)
{
    RxHeader1.BitRateSwitch = FDCAN_BRS_OFF;
    RxHeader1.DataLength = FDCAN_DLC_BYTES_4;    //Data frame size of 5 bytes
    RxHeader1.ErrorStateIndicator = FDCAN_ESI_PASSIVE; 
    RxHeader1.FDFormat = FDCAN_CLASSIC_CAN;   
}

void CAN1_transmits(void)
{
    (void)HAL_FDCAN_AddMessageToTxFifoQ(&CAN1_struct, &TxHeader1, CAN1_tx_message); //Message add to the TxQueue
    TxIndex = HAL_FDCAN_GetLatestTxFifoQRequestBuffer(&CAN1_struct);    //Gets last transmission request index
    while(HAL_FDCAN_IsTxBufferMessagePending(&CAN1_struct, TxIndex));   //waits while message is pending 
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
    static uint8_t i = 0;
    uart_rx_buffer[i] = uart_rx_byte;
    i++;
    if(uart_rx_buffer[i-1u] == (uint8_t)'\r')
    {
        uart_rx_cplt_flag = SET;
        i = 0;
    }
    (void)HAL_UART_Receive_IT(&UART_struct, &uart_rx_byte, 1);
}

void CUBA_init(void)
{
    CUBA_Handle.UARTHandler = &UART_struct;
    CUBA_Handle.CANHandler = &CAN2_struct;
    CUBA_Handle.CANRxHeader = &RxHeader2;
    CUBA_Handle.CANFilterHeader = &Filter2_struct;
    CUBA_Handle.DMAHandler = &hdma_usart2_tx;
    MOD_CUBA_Init(&CUBA_Handle);
}