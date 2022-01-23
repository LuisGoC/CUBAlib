#include "CUBA.h"

static     void      CUBA_string       ( CUBA_HandleTypeDef *hcuba);
static     uint8_t   intToHex          ( uint32_t val, uint8_t* str);
static     void      integerToString   ( uint32_t value, uint8_t *str);
static     uint64_t  hexToInt          (uint8_t* str);

static CUBA_HandleTypeDef *CUBA_struct;

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
    
    /* check the USART instance equals USART2 and 115200 baudrate */
    if(hcuba->UARTHandler->Instance != USART2 || hcuba->UARTHandler->Init.BaudRate != 115200)
    {
        return HAL_ERROR;
    }

    /* DMA peripheral clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* USART2 DMA Init */
    /* USART2_TX Init */
    hcuba->DMAHandler->Instance                     =   DMA1_Channel1;
    hcuba->DMAHandler->Init.Request                 =   DMA_REQUEST_USART2_TX;
    hcuba->DMAHandler->Init.Direction               =   DMA_MEMORY_TO_PERIPH;
    hcuba->DMAHandler->Init.PeriphInc               =   DMA_PINC_DISABLE;
    hcuba->DMAHandler->Init.MemInc                  =   DMA_MINC_ENABLE;
    hcuba->DMAHandler->Init.PeriphDataAlignment     =   DMA_PDATAALIGN_BYTE;
    hcuba->DMAHandler->Init.MemDataAlignment        =   DMA_MDATAALIGN_BYTE;
    hcuba->DMAHandler->Init.Mode                    =   DMA_NORMAL;
    hcuba->DMAHandler->Init.Priority                =   DMA_PRIORITY_LOW;

    /* DMA init */
    HAL_DMA_Init(hcuba->DMAHandler);

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    /* Linking UART Tx DMA Handle parameters */
    hcuba->UARTHandler->hdmatx = hcuba->DMAHandler; 
    hcuba->DMAHandler->Parent = hcuba->UARTHandler;

    /* check the FDCAN handle */
    if(hcuba->CANHandler == NULL)
    {
        return HAL_ERROR;
    }
    else
    {
        /* FDCAN2 Init */
        hcuba->CANHandler->Instance                     =   FDCAN2;
        hcuba->CANHandler->Init.Mode                    =   FDCAN_MODE_NORMAL;
        hcuba->CANHandler->Init.AutoRetransmission      =   ENABLE;
        hcuba->CANHandler->Init.ClockDivider            =   FDCAN_CLOCK_DIV1;
        hcuba->CANHandler->Init.TxFifoQueueMode         =   FDCAN_TX_FIFO_OPERATION;
        hcuba->CANHandler->Init.TransmitPause           =   DISABLE;
        hcuba->CANHandler->Init.ProtocolException       =   DISABLE;
        hcuba->CANHandler->Init.ExtFiltersNbr           =   0;
        hcuba->CANHandler->Init.StdFiltersNbr           =   0;
        hcuba->CANHandler->Init.FrameFormat             =   FDCAN_FRAME_CLASSIC;
        hcuba->CANHandler->Init.NominalPrescaler        =   30;
        hcuba->CANHandler->Init.NominalSyncJumpWidth    =   1;
        hcuba->CANHandler->Init.NominalTimeSeg1         =   13;
        hcuba->CANHandler->Init.NominalTimeSeg2         =   2;
        HAL_FDCAN_Init(hcuba->CANHandler);
        
        /* FDCAN interrupt init */
        /* TIM17_FDCAN_IT1_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(TIM17_FDCAN_IT1_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(TIM17_FDCAN_IT1_IRQn);
    }

    /* Check FDCAN2 Filter Handle */
    if(hcuba->CANFilterHeader == NULL)
    {
        return HAL_ERROR;
    }
    else
    {
        /* FDCAN2 Rx Filter init, disable */
        hcuba->CANFilterHeader->FilterConfig = FDCAN_FILTER_DISABLE;
        hcuba->CANFilterHeader->IdType = FDCAN_STANDARD_ID;
        HAL_FDCAN_ConfigFilter(hcuba->CANHandler, hcuba->CANFilterHeader);
    
        /* FDCAN2 Global Rx filter accepts all non-matching frames in FIFO1 */  
        HAL_FDCAN_ConfigGlobalFilter(hcuba->CANHandler, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_REJECT_REMOTE);
    }
    
    /* Assign FIFO1 interrupt group to interrupt Line 1 */
    /* Enable interrupt notifications when FIFO1 is full */
    (void)HAL_FDCAN_ConfigInterruptLines(hcuba->CANHandler, FDCAN_IT_GROUP_RX_FIFO1, FDCAN_INTERRUPT_LINE1);
    (void)HAL_FDCAN_ActivateNotification(hcuba->CANHandler, FDCAN_IT_RX_FIFO1_FULL, FDCAN_RX_FIFO1);
    (void)HAL_FDCAN_ActivateNotification(hcuba->CANHandler, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, FDCAN_RX_FIFO1);

    /* Check FDCAN2 Rx Handle */
    if(hcuba->CANRxHeader == NULL)
    {
        return HAL_ERROR;
    }
    else
    {
        /* FDCAN2 Rx handle parameters initialization */
        hcuba->CANRxHeader->BitRateSwitch = FDCAN_BRS_OFF;
        hcuba->CANRxHeader->DataLength = FDCAN_DLC_BYTES_8;
        hcuba->CANRxHeader->ErrorStateIndicator = FDCAN_ESI_PASSIVE;
        hcuba->CANRxHeader->FDFormat = FDCAN_CLASSIC_CAN;
    }

    hcuba->CANTxHeader->DataLength = FDCAN_DLC_BYTES_8; //Data frame size of 8 bytes
    hcuba->CANTxHeader->Identifier = 0x0FF;             // 11 bit identifier  (Lower value, higher priority)
    hcuba->CANTxHeader->IdType = FDCAN_STANDARD_ID;     // standar Id (11 bits)
    hcuba->CANTxHeader->FDFormat = FDCAN_CLASSIC_CAN;   
    hcuba->CANTxHeader->TxFrameType = FDCAN_DATA_FRAME; //DATA FRAME (It also can be remote frame RTR = 1)
    hcuba->CANTxHeader->BitRateSwitch = FDCAN_BRS_OFF;  
    hcuba->CANTxHeader->TxEventFifoControl = FDCAN_NO_TX_EVENTS; 

    hcuba->pRxFlag = 0;

    memset(hcuba->pTxMsg, 0, sizeof(hcuba->pTxMsg));

    CUBA_struct = hcuba;

    return HAL_OK;
}

/**
  * @brief Periodic task of CUBA library that get FDCAN2 Rx Messages, 
  *         processes its data and sends it through UART to the terminal computer using DMA. 
  * @param hcuba pointer to a CUBA_HandleTypeDef structure that contains 
  *              the configuration information for the CUBA library
  * @retval HAL Status
  **/
 HAL_StatusTypeDef MOD_CUBA_Start( CUBA_HandleTypeDef *hcuba )
{
    /* Check the CUBA handle */
    if(hcuba == NULL)
    {
        return HAL_ERROR;
    }

    /* FDCAN2 Start */
    HAL_FDCAN_Start(hcuba->CANHandler);

    return HAL_OK;
}

HAL_StatusTypeDef MOD_CUBA_GetData( CUBA_HandleTypeDef *hcuba, uint8_t data)
{
    static uint8_t i = 0;
    uint64_t value;

    if(data != '\r')
    {
        hcuba->pTxMsg[i] = data;
        i++;
    }
    else
    {
        i = 0;
        value = hexToInt(hcuba->pTxMsg);
        HAL_FDCAN_AddMessageToTxFifoQ(hcuba->CANHandler, hcuba->CANTxHeader, (uint8_t*)&value);
    }

    return HAL_OK;
}

/**
  * @brief Gets the Rx FDCAN2 structure parameters to create a string of characters 
  *         with the information about the received message.
  * @param hcuba pointer to a CUBA_HandleTypeDef structure that contains 
  *              the configuration information for the CUBA library
  * @param str   Pointer to data buffer (u8 data elements).
  * @retval none
  **/
static void CUBA_string( CUBA_HandleTypeDef *hcuba)
{
    char word[10] = {0};

    memset(hcuba->CUBA_buffer, 0, sizeof(hcuba->CUBA_buffer));
    
    /* Get identifier of Rx FDCAN2 msg and convert it to hex string */
    intToHex(hcuba->CANRxHeader->Identifier, (uint8_t*)word);
    strcat((char*)hcuba->CUBA_buffer, word+4);
    strcat((char*)hcuba->CUBA_buffer, S_DSPACE);

    /* IdType equals FDCAN_STANDARD_ID */
    if(hcuba->CANRxHeader->IdType == FDCAN_STANDARD_ID)
    {
        strcat((char*)hcuba->CUBA_buffer, S_SDCAN);
        strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }
    else   /* FDCAN_EXTENDED_ID */
    {
        strcat((char*)hcuba->CUBA_buffer, S_FDCAN);
        strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }

    /* Get DLC of Rx FDCAN2 msg and convert it to string */
    integerToString(hcuba->CANRxHeader->DataLength >> 16, (uint8_t*)word);
    strcat((char*)hcuba->CUBA_buffer, word);
    strcat((char*)hcuba->CUBA_buffer, S_DSPACE);

    /* Rx Msg */
    if(hcuba->pRxFlag == 1)
    {
        hcuba->pRxFlag = 0;
        strcat((char*)hcuba->CUBA_buffer, S_RX);
        strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }
    else /* Tx Msg */
    {
        strcat((char*)hcuba->CUBA_buffer, S_TX);
        strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }

    /* RxFrameType equals FDCAN_DATA_FRAME */
    if(hcuba->CANRxHeader->RxFrameType == FDCAN_DATA_FRAME)
    {
        strcat((char*)hcuba->CUBA_buffer, S_DATA);
        strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }
    else  /* RxFrameType equals FDCAN_REMOTE_FRAME */
    {
        strcat((char*)hcuba->CUBA_buffer, S_REMOTE);
        strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }

    /* Data Payload */
    for(uint8_t i = 0; i < (hcuba->CANRxHeader->DataLength >> 16); i++)
    {
        intToHex(hcuba->pRxMsg[i], (uint8_t*)word);
        strcat((char*)hcuba->CUBA_buffer, word+6);
        strcat((char*)hcuba->CUBA_buffer, S_DSPACE);
    }

    /* Data in ASCII */
    for(uint8_t i = 0; i < (hcuba->CANRxHeader->DataLength >> 16); i++)
    {
        char data[2] = {0};
        /* ASCII data */
        if((hcuba->pRxMsg[i] >= 32) && (hcuba->pRxMsg[i] <= 126)) 
        {
            data[0] = hcuba->pRxMsg[i];
            strcat((char*)hcuba->CUBA_buffer, data);
        }
        else /* No ASCII data */
        {
            strcat((char*)hcuba->CUBA_buffer, ".");
        }
    }

    /* end of string */
    strcat((char*)hcuba->CUBA_buffer, "\n");

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
    if(val != 0)
    {
        /* for Max. HEX characters value possible in 32 bits */
        for(uint8_t i = 0; i < 8; i++)
        {
            pos = (val>>rsv)&mask;  //Position gets HEX Character value
            str[index] = cHex[pos]; //str[index] gets HEX Character value
            rsv -= 4;               //Right Shift Value decrements 4 (4 bits per hex character)
            index++;                //index increments by 1 
        }
        str[index] = '\0'; //str last character get NULL character
    }
    else /* value equals zero*/
    {   
        /* All characters are zero */
         for(uint8_t i = 0; i < 8; i++)
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
    if(value != 0)
    {
        temp[index] = '\0'; //last position gets NULL character
        while(value != 0UL)
        {
            index--;
            temp[index] = (value%10UL)+48UL; //next position gets its ASCII value
            value /= 10;
        }
        for(uint8_t i = 0; i < (11 - index); i++)
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

static uint64_t hexToInt(uint8_t* str)
{
    /* Local variables */
    uint8_t index = 0;         
    uint64_t res = 0;

    if((str != NULL))
    {
        while(index < 16)
        {
            if((str[index]-48) < 10)
            {
                res |= (str[index] - 48);
                res <<= 4;
            }
            else
            {
                res |= (str[index] - 55);
                res <<= 4;
            }
            if(index != 15)
            {
                res <<= 4;
            }
            index++;
        }
    }

    return res;
}


void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    
    if((hfdcan->Instance == FDCAN2) && (RxFifo1ITs == FDCAN_IT_RX_FIFO1_FULL))
    {
        //If FIFO1 is full
    }
    if((hfdcan->Instance == FDCAN2) && (RxFifo1ITs == FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
    {
        //If there's a new msg in FIFO1
        /* Get FIFO1 Message */
        if(HAL_FDCAN_GetRxMessage(CUBA_struct->CANHandler, FDCAN_RX_FIFO1, CUBA_struct->CANRxHeader, CUBA_struct->pRxMsg) == HAL_OK)
        {
            CUBA_struct->pRxFlag = 1;

            /* create character string with Rx FDCAN message data */ 
            CUBA_string(CUBA_struct);

            /* DMA UART Transmit of Rx FDCAN analyzed data */
            HAL_UART_Transmit_DMA(CUBA_struct->UARTHandler, CUBA_struct->CUBA_buffer, STRING_LENGTH);
        }
    }
    
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

} 


