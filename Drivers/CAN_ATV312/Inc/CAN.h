#ifndef CAN_H_
#define CAN_H_
#include "stm32f407xx.h"


#define GPIOD_EN           (1U<<3)  // Bat GPIOD
#define CAN1_EN            (1U<<25) // Bat CAN1 
#define CAN_RX_PIN         (2U<<0) // PD0
#define CAN_TX_PIN         (2U<<2) // PD1

typedef struct
{
    uint32_t id;
    uint8_t dlc;
    uint8_t rx_data[8];
    volatile uint8_t is_new_data;
}CAN_RxFrame_t;

extern volatile CAN_RxFrame_t g_rx_frame;

void CAN_Init(void);
uint8_t CAN_Send_Frame(uint16_t id, uint8_t *data, uint8_t dlc);

#endif
