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

    //filter config
    //hcuba->CANFilterHeader->FilterConfig = FDCAN_FILTER_DISABLE;
    //HAL_FDCAN_ConfigFilter(hcuba->CANHandler, hcuba->CANFilterHeader);

    //Reception initial setting
    hcuba->CANRxHeader->BitRateSwitch = FDCAN_BRS_OFF;
    hcuba->CANRxHeader->DataLength = FDCAN_DLC_BYTES_8;
    hcuba->CANRxHeader->ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    hcuba->CANRxHeader->FDFormat = FDCAN_CLASSIC_CAN;
    hcuba->CANRxHeader->FilterIndex = 0;

    hcuba->pRxFlag = 0;
    
    HAL_FDCAN_Start(hcuba->CANHandler);

    return HAL_OK;
}

 HAL_StatusTypeDef MOD_CUBA_CyclicalBusAnalysis( CUBA_HandleTypeDef *hcuba )
{
    uint8_t pDumpBuff[100] = {0};

    if(hcuba == NULL)
    {
        return HAL_ERROR;
    }

    //CAN RECEPTION
    if(HAL_FDCAN_GetRxFifoFillLevel(hcuba->CANHandler, FDCAN_RX_FIFO0) != 0)
    {
        if(HAL_FDCAN_GetRxMessage(hcuba->CANHandler, FDCAN_RX_FIFO0, hcuba->CANRxHeader, hcuba->pRxMsg) == HAL_OK)
        {
            hcuba->pRxFlag = 1;
            //Funcion de dump
            dump(hcuba, pDumpBuff);
            HAL_UART_Transmit(hcuba->UARTHandler, pDumpBuff, 100, HAL_MAX_DELAY);
        }
        else
        {
            return HAL_ERROR;
        }
    }
    else if(HAL_FDCAN_GetRxFifoFillLevel(hcuba->CANHandler, FDCAN_RX_FIFO1) != 0)
    {
        if(HAL_FDCAN_GetRxMessage(hcuba->CANHandler, FDCAN_RX_FIFO0, hcuba->CANRxHeader, hcuba->pRxMsg) == HAL_OK)
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
    else
    {
        //Nothing
    }

    return HAL_OK;
}

void dump( CUBA_HandleTypeDef *hcuba, uint8_t *str)
{
    uint8_t word[10] = {0};
    uint8_t *pIDType[2] = {(uint8_t *)"CAN_SD", (uint8_t *)"CAN_FD"};
    uint8_t *pFType[2] = {(uint8_t *)"Data", (uint8_t *)"Remote"};
    uint8_t *pDir[2] = {(uint8_t *)"Rx", (uint8_t *)"Tx"};
    uint8_t *pSpace[2] = {(uint8_t *)" ", (uint8_t *)"  "};
    
    intToHex(hcuba->CANRxHeader->Identifier, word);
    stringCat(str, word+4);
    stringCat(str, pSpace[1]);

    if(hcuba->CANRxHeader->IdType == FDCAN_STANDARD_ID)
    {
        stringCat(str, pIDType[0]);
        stringCat(str, pSpace[1]);
    }
    else
    {
        stringCat(str, pIDType[1]);
        stringCat(str, pSpace[1]);
    }
    
    if(hcuba->CANRxHeader->RxFrameType == FDCAN_DATA_FRAME)
    {
        stringCat(str, pFType[0]);
        stringCat(str, pSpace[1]);
    }
    else
    {
        stringCat(str, pFType[1]);
        stringCat(str, pSpace[1]);
    }

    if(hcuba->pRxFlag == 1)
    {
        hcuba->pRxFlag = 0;
        stringCat(str, pDir[0]);
        stringCat(str, pSpace[1]);
    }
    else
    {
        stringCat(str, pDir[1]);
        stringCat(str, pSpace[1]);
    }

    integerToString(hcuba->CANRxHeader->DataLength >> 16, word);
    stringCat(str, word);
    stringCat(str, pSpace[1]);

    for(uint8_t i = 0; i < (hcuba->CANRxHeader->DataLength >> 16); i++)
    {
        intToHex(hcuba->pRxMsg[i], word);
        stringCat(str, word+6);
        stringCat(str, pSpace[1]);
    }

    /*for(uint8_t i = 0; i < (hcuba->CANRxHeader->DataLength >> 16); i++)
    {
        if(hcuba->pRxMsg[i] 32 - 126) //ascii
        {

        }
    }*/

}

uint8_t intToHex(uint32_t val, uint8_t* str)
{
    uint8_t index = 0;
    uint32_t mask = 0x0000000F;
    uint8_t pos = 0;
    
    uint8_t cHex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', \
                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    if(val != 0)
    {
        for(uint8_t rsv = 28; rsv != 0; rsv -= 4)
        {
            pos = (val>>rsv)&mask;
            str[index] = cHex[pos];
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
    uint8_t *numPtr = NULL;
    if(value != 0)
    {
        temp[index] = '\n';
        while(value != 0UL)
        {
            index--;
            temp[index] = (value%10UL)+48UL;
            value /= 10;
        }
        numPtr = temp+index;
        for(uint8_t i = 0; numPtr[i] != '\0' ; i++)
        {
            str[i] = numPtr[i];
        }   
    }
    else
    {
        str[0] = '0';
        str[1] = '\0';
    }
}

void stringCat(uint8_t *str1, const uint8_t *str2)
{
    uint8_t i;

    for (i = 0; str1[i] != '\0'; i++)
    {
        str1[i] = str1[i];
    }
    for (uint8_t j = 0; str2[j] != '\0'; i++)
    {
        str1[i] = str2[j];
        j++;
    }
    str1[i + 1u] = 0u;
}
