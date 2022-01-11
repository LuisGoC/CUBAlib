#include "app_bsp.h"
#include "CUBA.h"

HAL_StatusTypeDef MOD_CUBA_Init( CUBA_HandleTypeDef *hcuba )
{
    if(hcuba == NULL)
    {
        return HAL_ERROR;
    }
    
    if(hcuba->UARTHandler->Instance != USART2)
    {
        return HAL_ERROR;
    }

    //FDCAN init
    if(hcuba->CANHandler == NULL)
    {
        return HAL_ERROR;
    }
    else
    {
        hcuba->CANHandler->Instance = FDCAN2;
        hcuba->CANHandler->Init.Mode = FDCAN_MODE_NORMAL;
        hcuba->CANHandler->Init.AutoRetransmission = ENABLE;
        hcuba->CANHandler->Init.ClockDivider = FDCAN_CLOCK_DIV1;
        hcuba->CANHandler->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
        hcuba->CANHandler->Init.TransmitPause = DISABLE;
        hcuba->CANHandler->Init.ProtocolException = DISABLE;
        hcuba->CANHandler->Init.ExtFiltersNbr = 0;
        hcuba->CANHandler->Init.StdFiltersNbr = 0;
        hcuba->CANHandler->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
        hcuba->CANHandler->Init.NominalPrescaler = 10;
        hcuba->CANHandler->Init.NominalSyncJumpWidth = 1;
        hcuba->CANHandler->Init.NominalTimeSeg1 = 13;
        hcuba->CANHandler->Init.NominalTimeSeg2 = 2;
        HAL_FDCAN_Init(hcuba->CANHandler);
    }

    //filter config
    if(hcuba->CANFilterHeader == NULL)
    {
        return HAL_ERROR;
    }
    else
    {
        //Filter config disable
        hcuba->CANFilterHeader->FilterConfig = FDCAN_FILTER_DISABLE;
        hcuba->CANFilterHeader->IdType = FDCAN_STANDARD_ID;
        HAL_FDCAN_ConfigFilter(hcuba->CANHandler, hcuba->CANFilterHeader);
        //All standard CAN messages are received in FIFO1
        HAL_FDCAN_ConfigGlobalFilter(hcuba->CANHandler, FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_REJECT_REMOTE);
    }

    (void)HAL_FDCAN_ConfigInterruptLines(hcuba->CANHandler, FDCAN_IT_GROUP_RX_FIFO1, FDCAN_INTERRUPT_LINE1);
    (void)HAL_FDCAN_ActivateNotification(hcuba->CANHandler, FDCAN_IT_RX_FIFO1_FULL, FDCAN_RX_FIFO1);

    //Reception initial setting
    if(hcuba->CANRxHeader == NULL)
    {
        return HAL_ERROR;
    }
    else
    {
        hcuba->CANRxHeader->BitRateSwitch = FDCAN_BRS_OFF;
        hcuba->CANRxHeader->DataLength = FDCAN_DLC_BYTES_8;
        hcuba->CANRxHeader->ErrorStateIndicator = FDCAN_ESI_PASSIVE;
        hcuba->CANRxHeader->FDFormat = FDCAN_CLASSIC_CAN;
    }

    hcuba->pRxFlag = 0;
    
    HAL_FDCAN_Start(hcuba->CANHandler);

    return HAL_OK;
}

 HAL_StatusTypeDef MOD_CUBA_PeriodicTask( CUBA_HandleTypeDef *hcuba )
{
    uint8_t pDumpBuff[100] = {0};

    if(hcuba == NULL)
    {
        return HAL_ERROR;
    }

    //CAN RECEPTION
    if(HAL_FDCAN_GetRxFifoFillLevel(hcuba->CANHandler, FDCAN_RX_FIFO1) != 0)
    {
        if(HAL_FDCAN_GetRxMessage(hcuba->CANHandler, FDCAN_RX_FIFO1, hcuba->CANRxHeader, hcuba->pRxMsg) == HAL_OK)
        {
            hcuba->pRxFlag = 1;
            //Funcion de dump
            dump(hcuba, pDumpBuff);
            HAL_UART_Transmit(hcuba->UARTHandler, pDumpBuff, sizeof(pDumpBuff), HAL_MAX_DELAY);
        }
        else
        {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

void dump( CUBA_HandleTypeDef *hcuba, uint8_t *str)
{
    char word[10] = {0};
    char *pIDType[2] = {"CAN_SD", "CAN_FD"};
    char *pFType[2] = {"Data", "Remote"};
    char *pDir[2] = {"Rx", "Tx"};
    char *pSpace[2] = {" ", "  "};
    
    intToHex(hcuba->CANRxHeader->Identifier, (uint8_t*)word);
    strcat((char*)str, word+4);
    strcat((char*)str, pSpace[1]);

    if(hcuba->CANRxHeader->IdType == FDCAN_STANDARD_ID)
    {
        strcat((char*)str, pIDType[0]);
        strcat((char*)str, pSpace[1]);
    }
    else
    {
        strcat((char*)str, pIDType[1]);
        strcat((char*)str, pSpace[1]);
    }

    integerToString(hcuba->CANRxHeader->DataLength >> 16, (uint8_t*)word);
    strcat((char*)str, word);
    strcat((char*)str, pSpace[1]);

    if(hcuba->pRxFlag == 1)
    {
        hcuba->pRxFlag = 0;
        strcat((char*)str, pDir[0]);
        strcat((char*)str, pSpace[1]);
    }
    else
    {
        strcat((char*)str, pDir[1]);
        strcat((char*)str, pSpace[1]);
    }

    if(hcuba->CANRxHeader->RxFrameType == FDCAN_DATA_FRAME)
    {
        strcat((char*)str, pFType[0]);
        strcat((char*)str, pSpace[1]);
    }
    else
    {
        strcat((char*)str, pFType[1]);
        strcat((char*)str, pSpace[1]);
    }

    for(uint8_t i = 0; i < (hcuba->CANRxHeader->DataLength >> 16); i++)
    {
        intToHex(hcuba->pRxMsg[i], (uint8_t*)word);
        strcat((char*)str, word+6);
        strcat((char*)str, pSpace[1]);
    }

    for(uint8_t i = 0; i < (hcuba->CANRxHeader->DataLength >> 16); i++)
    {
        char data[2] = {0};
        if((hcuba->pRxMsg[i] >= 32) && (hcuba->pRxMsg[i] <= 126)) //ascii
        {
            data[0] = hcuba->pRxMsg[i];
            strcat((char*)str, data);
        }
        else
        {
            strcat((char*)str, ".");
        }
    }

    strcat((char*)str, "\n");

}

uint8_t intToHex(uint32_t val, uint8_t* str)
{
    uint8_t index = 0;
    uint32_t mask = 0x0000000F;
    uint8_t pos = 0;
    uint8_t rsv = 28; //Right shift value
    
    uint8_t cHex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', \
                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    if(val != 0)
    {
        for(uint8_t i = 0; i < 8; i++)
        {
            pos = (val>>rsv)&mask;
            str[index] = cHex[pos];
            rsv -= 4;
            index++;
        }
        str[index] = '\0';
    }
    else
    {
         for(uint8_t i = 0; i < 8; i++)
         {
             str[i] = '0';
         }
         index = 8;
         str[index] = '\0';
    }

    return index;
}

void integerToString(uint32_t value, uint8_t *str)
{
    uint8_t temp[12] = {0};
    uint8_t index = 10;
    if(value != 0)
    {
        temp[index] = '\0';
        while(value != 0UL)
        {
            index--;
            temp[index] = (value%10UL)+48UL;
            value /= 10;
        }
        for(uint8_t i = 0; i < (11 - index); i++)
        {
            str[i] = temp[index+i];
        }   
    }
    else
    {
        str[0] = '0';
        str[1] = '\0';
    }
}

__weak void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    if((hfdcan->Instance == FDCAN2) && (RxFifo1ITs == FDCAN_IT_RX_FIFO1_FULL))
    {
        //coming soon...
    }
}


