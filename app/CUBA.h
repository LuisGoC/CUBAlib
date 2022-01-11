#ifndef SERIAL_CAN_H_
    #define SERIAL_CAN_H_

    #include "app_bsp.h"
    #include "stm32g0xx.h"
    #include <stdint.h>

    typedef struct
    {
        UART_HandleTypeDef      *UARTHandler;
        FDCAN_HandleTypeDef     *CANHandler;
        FDCAN_TxHeaderTypeDef   *CANTxHeader;
        FDCAN_RxHeaderTypeDef   *CANRxHeader;
        FDCAN_FilterTypeDef     *CANFilterHeader;
        uint8_t                 pRxMsg[8];
        uint8_t                 pRxFlag;
    }CUBA_HandleTypeDef;

    HAL_StatusTypeDef MOD_CUBA_Init( CUBA_HandleTypeDef *hcuba );
    HAL_StatusTypeDef MOD_CUBA_PeriodicTask( CUBA_HandleTypeDef *hcuba );
    void dump( CUBA_HandleTypeDef *hcuba, uint8_t *str);
    uint8_t intToHex(uint32_t val, uint8_t* str);
    void integerToString(uint32_t value, uint8_t *str);
    void stringCat(uint8_t *str1, const uint8_t *str2);

#endif