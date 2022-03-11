#include "CUBA.h"
#include "queue.h"

static     void              CUBA_string                ( CUBA_HandleTypeDef *hcuba, CUBA_RxMsgTypeDef *hrxmsg );
static     HAL_StatusTypeDef cmd_process                (uint8_t *cmd);
static     uint8_t           intToHex                   ( uint32_t val, uint8_t* str );
static     void              integerToString            ( uint32_t value, uint8_t *str );
static     uint64_t          hexToInt                   (uint8_t* str);
static     void              MOD_CUBA_GetUartData       ( UART_HandleTypeDef *huart, uint8_t data );
static     void              MOD_CUBA_GetUartTxCpltFlag ( UART_HandleTypeDef *huart );

static QUEUE_HandleTypeDef fdcan_queue_struct     =   {0};
static QUEUE_HandleTypeDef uart_queue_struct      =   {0};
static CUBA_HandleTypeDef  *CUBA_HandlePtr        =   NULL;
static uint8_t             uart_rx_byte;        //UART reception variable

/**
  * @brief Initializes the CUBA library necessary peripherals.
  * @param hcuba pointer to a CUBA_HandleTypeDef structure that contains 
  *              the configuration information for the CUBA library
  * @retval HAL Status
  **/
HAL_StatusTypeDef MOD_CUBA_Init( CUBA_HandleTypeDef *hcuba )
{
    /* Check the CUBA handle */
    if(hcuba == NULL)
    {
        return HAL_ERROR; 
    }

    (void)memset(hcuba->CUBA_buffer, 0, sizeof(hcuba->CUBA_buffer));

    GPIO_InitTypeDef gpio_struct;

    /* GPIOA and USART2 Clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA2 and PA3 GPIOs Init as USART2 AF */
    gpio_struct.Alternate = GPIO_AF1_USART2;
    gpio_struct.Mode = GPIO_MODE_AF_PP;
    gpio_struct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_struct.Pull = GPIO_PULLUP;
    gpio_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio_struct);
    
    /* UART Init */
    hcuba->UARTHandler.Instance            = USART2;
    hcuba->UARTHandler.Init.BaudRate       = 115200;
    hcuba->UARTHandler.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    hcuba->UARTHandler.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
    hcuba->UARTHandler.Init.Mode           = UART_MODE_TX_RX;
    hcuba->UARTHandler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLED;
    hcuba->UARTHandler.Init.OverSampling   = UART_OVERSAMPLING_16;
    hcuba->UARTHandler.Init.Parity         = UART_PARITY_NONE;
    hcuba->UARTHandler.Init.StopBits       = UART_STOPBITS_1;
    hcuba->UARTHandler.Init.WordLength     = UART_WORDLENGTH_8B;
    (void)HAL_UART_Init(&hcuba->UARTHandler);

    /* USART2 interrupt init */
    /* USART2_LPUART2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(USART2_LPUART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_LPUART2_IRQn);

    /* DMA peripheral clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* USART2 DMA Init */
    /* USART2_TX Init */
    hcuba->DMAHandler.Instance                     =   DMA1_Channel1;
    hcuba->DMAHandler.Init.Request                 =   DMA_REQUEST_USART2_TX;
    hcuba->DMAHandler.Init.Direction               =   DMA_MEMORY_TO_PERIPH;
    hcuba->DMAHandler.Init.PeriphInc               =   DMA_PINC_DISABLE;
    hcuba->DMAHandler.Init.MemInc                  =   DMA_MINC_ENABLE;
    hcuba->DMAHandler.Init.PeriphDataAlignment     =   DMA_PDATAALIGN_BYTE;
    hcuba->DMAHandler.Init.MemDataAlignment        =   DMA_MDATAALIGN_BYTE;
    hcuba->DMAHandler.Init.Mode                    =   DMA_NORMAL;
    hcuba->DMAHandler.Init.Priority                =   DMA_PRIORITY_LOW;

    /* DMA init */
    HAL_DMA_Init(&(hcuba->DMAHandler));

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    /* Linking UART Tx DMA Handle parameters */
    hcuba->UARTHandler.hdmatx = &(hcuba->DMAHandler); 
    hcuba->DMAHandler.Parent  = &(hcuba->UARTHandler);

    GPIO_InitTypeDef GpioCanStruct;

    /* GPIO and CAN Clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* pin 0(rx) and pin 1(tx) set as AF for FDCAN2*/
    GpioCanStruct.Mode      = GPIO_MODE_AF_PP;
    GpioCanStruct.Alternate = GPIO_AF3_FDCAN2;
    GpioCanStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
    GpioCanStruct.Pull      = GPIO_NOPULL;
    GpioCanStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GpioCanStruct);

    /* FDCAN2 Init */
    hcuba->CANHandler.Instance                     =   FDCAN2;
    hcuba->CANHandler.Init.Mode                    =   FDCAN_MODE_NORMAL;
    hcuba->CANHandler.Init.AutoRetransmission      =   DISABLE;
    hcuba->CANHandler.Init.ClockDivider            =   FDCAN_CLOCK_DIV1;
    hcuba->CANHandler.Init.TxFifoQueueMode         =   FDCAN_TX_FIFO_OPERATION;
    hcuba->CANHandler.Init.TransmitPause           =   DISABLE;
    hcuba->CANHandler.Init.ProtocolException       =   DISABLE;
    hcuba->CANHandler.Init.ExtFiltersNbr           =   0;
    hcuba->CANHandler.Init.StdFiltersNbr           =   0;
    hcuba->CANHandler.Init.FrameFormat             =   FDCAN_FRAME_CLASSIC;
    hcuba->CANHandler.Init.NominalPrescaler        =   30;
    hcuba->CANHandler.Init.NominalSyncJumpWidth    =   1;
    hcuba->CANHandler.Init.NominalTimeSeg1         =   13;
    hcuba->CANHandler.Init.NominalTimeSeg2         =   2;
    HAL_FDCAN_Init(&(hcuba->CANHandler));
        
    /* FDCAN interrupt init */
    /* TIM17_FDCAN_IT1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(TIM17_FDCAN_IT1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM17_FDCAN_IT1_IRQn);

    /* FDCAN2 Rx Filter init, disable */
    hcuba->CANFilterHeader.FilterConfig = FDCAN_FILTER_DISABLE;
    hcuba->CANFilterHeader.IdType = FDCAN_STANDARD_ID;
    HAL_FDCAN_ConfigFilter(&(hcuba->CANHandler), &(hcuba->CANFilterHeader));

    /* FDCAN2 Global Rx filter accepts all non-matching frames in FIFO1 */  
    HAL_FDCAN_ConfigGlobalFilter(&(hcuba->CANHandler), FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_REJECT_REMOTE);
    
    
    /* Assign FIFO1 interrupt group to interrupt Line 1 */
    /* Enable interrupt notifications when FIFO1 is full */
    (void)HAL_FDCAN_ConfigInterruptLines(&(hcuba->CANHandler), FDCAN_IT_GROUP_RX_FIFO1, FDCAN_INTERRUPT_LINE1);
    (void)HAL_FDCAN_ActivateNotification(&(hcuba->CANHandler), FDCAN_IT_RX_FIFO1_FULL, FDCAN_RX_FIFO1);
    (void)HAL_FDCAN_ActivateNotification(&(hcuba->CANHandler), FDCAN_IT_RX_FIFO1_NEW_MESSAGE, FDCAN_RX_FIFO1);

    /* FDCAN2 Rx handle parameters initialization */
    hcuba->CANRxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    hcuba->CANRxHeader.DataLength = FDCAN_DLC_BYTES_8;
    hcuba->CANRxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    hcuba->CANRxHeader.FDFormat = FDCAN_CLASSIC_CAN;

    /* FDCAN2 Tx handle parameters initialization */
    hcuba->CANTxHeader.DataLength = FDCAN_DLC_BYTES_8;
    hcuba->CANTxHeader.Identifier = 0x0FF;
    hcuba->CANTxHeader.IdType = FDCAN_STANDARD_ID;
    hcuba->CANTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    hcuba->CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    hcuba->CANTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    hcuba->CANTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;

    //FDCAN queue initialization
    fdcan_queue_struct.Buffer = hcuba->RxMsgBuffer;
    fdcan_queue_struct.Elements = 10;
    fdcan_queue_struct.Size = sizeof(CUBA_RxMsgTypeDef);
    HIL_QUEUE_Init(&fdcan_queue_struct);

    //UART queue initialization
    uart_queue_struct.Buffer = hcuba->uartBuffer;
    uart_queue_struct.Elements = 116;
    uart_queue_struct.Size = sizeof(uint8_t);
    HIL_QUEUE_Init(&uart_queue_struct);

    hcuba->uartCpltFlag = SET;

    CUBA_HandlePtr = hcuba;

    /* FDCAN2 Start */
    HAL_FDCAN_Start(&(hcuba->CANHandler));

    /* Receive an amount of data in interrupt mode. */
    (void)HAL_UART_Receive_IT(&hcuba->UARTHandler, &uart_rx_byte, 1);

    return HAL_OK;
}

/**
  * @brief Periodic task of CUBA library that get FDCAN2 Rx Messages, 
  *         processes its data and sends it through UART to the terminal computer using DMA. 
  * @param hcuba pointer to a CUBA_HandleTypeDef structure that contains 
  *              the configuration information for the CUBA library
  * @retval HAL Status
  **/
 HAL_StatusTypeDef MOD_CUBA_PeriodicTask( CUBA_HandleTypeDef *hcuba )
{
    CUBA_RxMsgTypeDef RxMsgToRead = {0};
    uint8_t uart_cmd_array[30] = {0};
    uint8_t pData;
    uint8_t pIndex;

    pIndex = 0;

    /* Check the CUBA handle */
    if(hcuba == NULL)
    {
        return HAL_ERROR;  
    }

    /* UART RECEPTION - CAN TRANSMISSION */
    while(HIL_QUEUE_IsEmpty(&uart_queue_struct) == 0u) //If circular buffer isn't empty
    {
        //desactivar interrupcion
        (void)HIL_QUEUE_Read(&uart_queue_struct, &pData); 
        //habilitar interrupcion
        if(pData == (uint8_t)'\r') 
        {
            if(cmd_process(uart_cmd_array) == HAL_OK)
            {
                (void)HAL_FDCAN_AddMessageToTxFifoQ(&(hcuba->CANHandler), &(hcuba->CANTxHeader), hcuba->pTxMsg);   
            }
            else
            {
                //transmit error
                return HAL_ERROR;   
            }
            break; 
        }
        else
        {
            uart_cmd_array[pIndex] = pData; 
            pIndex++;
        }    
    }

    /* CAN RECEPTION - UART TRANMISSION*/
    if((HIL_QUEUE_IsEmpty(&fdcan_queue_struct) == 0u) && (hcuba->uartCpltFlag == SET))
    {
        if(HIL_QUEUE_Read(&fdcan_queue_struct, &RxMsgToRead) == 1u)
        {
            /* create character string with Rx FDCAN message data */ 
            CUBA_string(hcuba, &RxMsgToRead);

            /* DMA UART Transmit of Rx FDCAN analyzed data */
            HAL_UART_Transmit_DMA(&hcuba->UARTHandler, hcuba->CUBA_buffer, STRING_LENGTH);
            hcuba->uartCpltFlag = RESET;
        }
        else
        {
            return HAL_ERROR;   
        }
    }
    
    return HAL_OK;
}

/**
  * @brief This function will let the CUBA library store the data received by the UART in a queue 
  *         to avoid the loss of information until the execution of the periodic task function.
            Use this function in the UART Rx Transfer completed callback. HAL_UART_RxCpltCallback.
  * @param huart pointer to a UART_HandleTypeDef structure that contains 
  *              the configuration information for the specified UART.
  * @retval none
  **/
void MOD_CUBA_GetUartData( UART_HandleTypeDef *huart, uint8_t data )
{
    if(huart->Instance == USART2)
    {
        (void)HIL_QUEUE_Write(&uart_queue_struct, &data);
    }
}

/**
  * @brief This function will let the CUBA library know when it has finished transmitting data over the UART using DMA.
            Use this function in the UART Tx Transfer completed callback. HAL_UART_TxCpltCallback.
  * @param huart pointer to a UART_HandleTypeDef structure that contains 
  *              the configuration information for the specified UART.
  * @retval none
  **/
void MOD_CUBA_GetUartTxCpltFlag( UART_HandleTypeDef *huart )
{
    if(huart->Instance == USART2)
    {
        CUBA_HandlePtr->uartCpltFlag = SET;
    }
}

/**
  * @brief Gets the Rx FDCAN2 structure parameters to create a string of characters 
  *         with the information about the received message.
  * @param hcuba pointer to a CUBA_HandleTypeDef structure that contains 
  *              the configuration information for the CUBA library
  * @param str   Pointer to data buffer (u8 data elements).
  * @retval none
  **/
static void CUBA_string( CUBA_HandleTypeDef *hcuba, CUBA_RxMsgTypeDef *hrxmsg)
{
    char word[10] = {0};

    (void)memset(hcuba->CUBA_buffer, 0, sizeof(hcuba->CUBA_buffer));
    
    /* Get identifier of Rx FDCAN2 msg and convert it to hex string */
    (void)intToHex(hrxmsg->RxHeaderMsg.Identifier, (uint8_t*)word);
    (void)strcat((char*)hcuba->CUBA_buffer, &word[4]);
    (void)strcat((char*)hcuba->CUBA_buffer, S_DSPACE);

    /* IdType equals FDCAN_STANDARD_ID */
    if(hrxmsg->RxHeaderMsg.IdType == FDCAN_STANDARD_ID)
    {
        (void)strcat((char*)hcuba->CUBA_buffer, S_SDCAN);
        (void)strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }
    else   /* FDCAN_EXTENDED_ID */
    {
        (void)strcat((char*)hcuba->CUBA_buffer, S_FDCAN);
        (void)strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }

    /* Get DLC of Rx FDCAN2 msg and convert it to string */
    (void)integerToString(hrxmsg->RxHeaderMsg.DataLength >> 16, (uint8_t*)word);
    (void)strcat((char*)hcuba->CUBA_buffer, word);
    (void)strcat((char*)hcuba->CUBA_buffer, S_DSPACE);

    /* RxFrameType equals FDCAN_DATA_FRAME */
    if(hrxmsg->RxHeaderMsg.RxFrameType == FDCAN_DATA_FRAME)
    {
        (void)strcat((char*)hcuba->CUBA_buffer, S_DATA);
        (void)strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }
    else  /* RxFrameType equals FDCAN_REMOTE_FRAME */
    {
        (void)strcat((char*)hcuba->CUBA_buffer, S_REMOTE);
        (void)strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }

    /* Data Payload */
    for(uint8_t i = 0; i < (hrxmsg->RxHeaderMsg.DataLength >> 16); i++)
    {
        (void)intToHex(hrxmsg->RxDataMsg[i], (uint8_t*)word);
        (void)strcat((char*)hcuba->CUBA_buffer, &word[6]);
        (void)strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }

    /* Data in ASCII */
    for(uint8_t i = 0; i < (hrxmsg->RxHeaderMsg.DataLength >> 16); i++)
    {
        char data[2] = {0};
        /* ASCII data */
        if((hrxmsg->RxDataMsg[i] >= 32u) && (hrxmsg->RxDataMsg[i] <= 126u)) 
        {
            data[0] = hrxmsg->RxDataMsg[i];
            (void)strcat((char*)hcuba->CUBA_buffer, data);
        }
        else /* No ASCII data */
        {
            (void)strcat((char*)hcuba->CUBA_buffer, ".");
        }
    }

    /* end of string */
    (void)strcat((char*)hcuba->CUBA_buffer, "\n");

}

/**
  * @brief Function to check the Rx UART command
  * @param cmd Pointer to a u8 data elements.
  * @retval HAL_StatusTypeDef
  **/
static HAL_StatusTypeDef cmd_process(uint8_t *cmd)
{
    uint16_t pID    = 0;
    uint64_t pValue = 0;
    uint8_t *cmdToken   = NULL;
    uint8_t *idToken    = NULL;
    uint8_t *valueToken = NULL;

    cmdToken    = (uint8_t*)strtok((char*)cmd, "=");
    idToken     = (uint8_t*)strtok(NULL, ",");
    valueToken  = (uint8_t*)strtok(NULL, "\r");

    if(strcmp((char *)cmdToken, "ATSMCAN") == 0)
    {
        if(((int)strlen((char*)valueToken) > 16) || ((int)strlen((char*)valueToken) == 0))
        {
            return HAL_ERROR;  
        }
    }
    else
    {
        return HAL_ERROR;  
    }

    pValue  = hexToInt(valueToken);
    pID     = hexToInt(idToken);

    for(uint8_t i = 8; i > 0u; i--)
    {
        CUBA_HandlePtr->pTxMsg[i-1u] = (uint8_t)(pValue&0xFFULL);
        pValue >>= 8ULL;
    }

    if(pID > 2047UL)
    {
        return HAL_ERROR;   
    }
    else
    {
        CUBA_HandlePtr->CANTxHeader.Identifier = pID;
    }

    return HAL_OK;
}

/**
  * @brief Converts integer values to a HEX string representation in ASCII.
  * @param val   integer value (u32 data element).
  * @param str   Pointer to data buffer (u8 data elements).
  * @retval string length
  **/
static uint8_t intToHex(uint32_t val, uint8_t* str)
{
    /* Local variables */
    uint8_t index = 0;          //string index
    uint32_t mask = 0xF;        //HEX character mask
    uint8_t pos = 0;            //HEX character position
    uint8_t rsv = 28;           //Right Shift Value

    /* HEX Characters Array */
    uint8_t cHex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', \
                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    /* Value is different to zero */
    if(val != 0UL)
    {
        /* for Max. HEX characters value possible in 32 bits */
        for(uint8_t i = 0; i < 8u; i++)
        {
            pos = (uint8_t)((val>>rsv)&mask);  //Position gets HEX Character value
            str[index] = cHex[pos]; //str[index] gets HEX Character value
            rsv -= 4u;               //Right Shift Value decrements 4 (4 bits per hex character)
            index++;                //index increments by 1 
        }
        str[index] = '\0'; //str last character get NULL character
    }
    else /* value equals zero*/
    {   
        /* All characters are zero */
         for(uint8_t i = 0; i < 8u; i++)
         {
             str[i] = '0';
         }
         index = 8;
         str[index] = '\0';
    }

    //return the lenght
    return index;
}

/**
  * @brief Converts integer values to a decimal string representation in ASCII.
  * @param val   integer value (u32 data element).
  * @param str   Pointer to data buffer (u8 data elements).
  * @retval none
  **/
static void integerToString(uint32_t value, uint8_t *str)
{
    uint8_t temp[12] = {0}; //temporal array
    uint8_t index = 10;     //index equals last position
    uint32_t valueCpy;

    valueCpy = value;  

    if(valueCpy != 0UL)
    {
        temp[index] = '\0'; //last position gets NULL character
        while(valueCpy != 0UL)
        {
            index--;
            temp[index] = (valueCpy%10UL)+48UL; //next position gets its ASCII value
            valueCpy /= (uint32_t)10;
        }
        for(uint8_t i = 0; i < (11u - index); i++)
        {
            str[i] = temp[index+i]; //str gets temp appropriate values
        }   
    }
    else
    {
        str[0] = '0';
        str[1] = '\0';
    }
}

/**
  * @brief Converts ASCII hexadecimal strings to an integer values.
  * @param str   Pointer to data buffer (u8 data elements).
  * @retval 64 bits integer value
  **/
static uint64_t hexToInt(uint8_t* str)
{
    uint64_t decimal = 0;
    uint64_t base = 1;
    uint8_t length;
    int8_t i = 0;
    
    if(str == NULL)
    {
        return 0;  
    }

    length = strlen((char*)str);
    
    for(i = length-1u; i >= 0; i--)
    {
        if((str[i] >= (uint8_t)'0') && (str[i] <= (uint8_t)'9'))
        {
            decimal += (uint64_t)(str[i] - 48UL) * base;
            base *= 16UL;
        }
        else if((str[i] >= (uint8_t)'A') && (str[i] <= (uint8_t)'F'))
        {
            decimal += (uint64_t)(str[i] - 55UL) * base;
            base *= 16UL;
        }
        else if((str[i] >= (uint8_t)'a') && (str[i] <= (uint8_t)'f'))
        {
            decimal += (uint64_t)(str[i] - 87UL) * base;
            base *= 16UL;
        }
        else
        {
            //nothing
        }
    }
    return decimal;
}

/**
  * @brief  Rx FIFO 1 callback.
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  RxFifo1ITs indicates which Rx FIFO 1 interrupts are signalled.
  *         This parameter can be any combination of @arg FDCAN_Rx_Fifo1_Interrupts.
  * @retval None
  */
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)    
{
    CUBA_RxMsgTypeDef RxMsgToWrite = {0};

    if((hfdcan->Instance == FDCAN2) && (RxFifo1ITs == FDCAN_IT_RX_FIFO1_FULL))
    {
        //If FIFO1 overwrite
        
    }
    if((hfdcan->Instance == FDCAN2) && (RxFifo1ITs == FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
    {
        //If there's a new message in FIFO1
        /* Get FIFO1 Message */
        if(HAL_FDCAN_GetRxMessage(&(CUBA_HandlePtr->CANHandler), FDCAN_RX_FIFO1, &(CUBA_HandlePtr->CANRxHeader), &(CUBA_HandlePtr->pRxMsg[0])) == HAL_OK)
        {
            (void)memcpy(&RxMsgToWrite.RxHeaderMsg, &(CUBA_HandlePtr->CANRxHeader), sizeof(RxMsgToWrite.RxHeaderMsg));
            (void)memcpy(&(RxMsgToWrite.RxDataMsg[0]), &(CUBA_HandlePtr->pRxMsg[0]), sizeof(RxMsgToWrite.RxDataMsg));
            (void)HIL_QUEUE_Write(&fdcan_queue_struct, &RxMsgToWrite);
        }   
    }
    
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
    (void)HAL_UART_Receive_IT(&CUBA_HandlePtr->UARTHandler, &uart_rx_byte, 1);
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

/** INTERRUPT VECTORS **/

void USART2_LPUART2_IRQHandler(void)    /* USART2 interrupt vector */
{
    HAL_UART_IRQHandler(&CUBA_HandlePtr->UARTHandler);
}

void TIM17_FDCAN_IT1_IRQHandler(void)   /* FDCAN2 interrupt vector */
{
    HAL_FDCAN_IRQHandler(&CUBA_HandlePtr->CANHandler);
}

void DMA1_Channel1_IRQHandler(void)     /* DMA1 interrupt vector */
{
  HAL_DMA_IRQHandler(&CUBA_HandlePtr->DMAHandler);
}


