#ifndef SERIAL_CAN_H_
    #define SERIAL_CAN_H_

    #include "stm32g0xx.h"
    #include <stdint.h>
    #include <string.h>

    /* CUBA_string length */
    #define STRING_LENGTH   65

    /* CUBA_string defines */
    #define S_SDCAN         "CAN_SD"
    #define S_FDCAN         "CAN_FD"
    #define S_DATA          "Data"
    #define S_REMOTE        "Remote"
    #define S_SPACE         " "
    #define S_DSPACE        "  "

    typedef struct 
    {
        FDCAN_RxHeaderTypeDef   RxHeaderMsg;
        uint8_t                 RxDataMsg[8];
    }CUBA_RxMsgTypeDef;
    
    /* CUBA_HandleTypeDef structure */
    typedef struct
    {
        UART_HandleTypeDef      *UARTHandler;           //pointer to UART_HandleTypeDef structure CUBA library shall use
        FDCAN_HandleTypeDef     *CANHandler;            //pointer to FDCAN_HandleTypeDef structure CUBA library shall use   
        FDCAN_TxHeaderTypeDef   *CANTxHeader;           //pointer to FDCAN_TxHeaderTypeDef structure CUBA library shall use
        FDCAN_RxHeaderTypeDef   *CANRxHeader;           //pointer to FDCAN_RxHeaderTypeDef structure CUBA library shall use
        FDCAN_FilterTypeDef     *CANFilterHeader;       //pointer to FDCAN_FilterTypeDef structure CUBA library shall use
        DMA_HandleTypeDef       *DMAHandler;            //pointer to DMA_HandleTypeDef structure CUBA library shall use
        uint8_t                 CUBA_buffer[100];       //Buffer of 100 elements to store CUBA analyzed data
        uint8_t                 pRxMsg[8];              //Buffer of 8 elements to store Rx FDCAN2 Msg 
        uint8_t                 pTxMsg[8];
        CUBA_RxMsgTypeDef       RxMsgBuffer[10];
        uint8_t                 uartBuffer[116];
        uint8_t                 uartCpltFlag;
    }CUBA_HandleTypeDef;

    /* CUBA Prototype Functions */
    HAL_StatusTypeDef MOD_CUBA_Init         ( CUBA_HandleTypeDef *hcuba );
    void HAL_CUBA_MspInit(CUBA_HandleTypeDef *hcuba);
    HAL_StatusTypeDef MOD_CUBA_PeriodicTask ( CUBA_HandleTypeDef *hcuba );
    void MOD_CUBA_GetUartData( UART_HandleTypeDef *huart, uint8_t data );
    void MOD_CUBA_GetUartTxCpltFlag( UART_HandleTypeDef *huart );

#endif