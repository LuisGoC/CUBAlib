#ifndef SERIAL_CAN_H_
    #define SERIAL_CAN_H_

    #include "stm32g0xx.h"
    #include <stdint.h>
    #include <string.h>

    /* CUBA_string length */
    #define STRING_LENGTH   68

    /* CUBA_string defines */
    #define S_SDCAN         "CAN_SD"
    #define S_FDCAN         "CAN_FD"
    #define S_DATA          "Data"
    #define S_REMOTE        "Remote"
    #define S_RX            "Rx"
    #define S_TX            "Tx"
    #define S_SPACE         " "
    #define S_DSPACE        "  "

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
        uint8_t                 pRxFlag;                //Rx Msg Flag
    }CUBA_HandleTypeDef;

    /* CUBA Prototype Functions */
    HAL_StatusTypeDef MOD_CUBA_Init         ( CUBA_HandleTypeDef *hcuba );
    HAL_StatusTypeDef MOD_CUBA_PeriodicTask ( CUBA_HandleTypeDef *hcuba );
    void              CUBA_string           ( CUBA_HandleTypeDef *hcuba, uint8_t *str);
    uint8_t           intToHex              ( uint32_t val, uint8_t* str);
    void              integerToString       ( uint32_t value, uint8_t *str);
    void              stringCat             ( uint8_t *str1, const uint8_t *str2);

#endif