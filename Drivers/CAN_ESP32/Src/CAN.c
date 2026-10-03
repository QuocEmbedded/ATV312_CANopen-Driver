#include "CAN.h"

volatile CAN_RxFrame_t g_rx_frame;

void CAN_Init(void)
{
    /**************GPIO configuration***********/
    RCC->AHB1ENR   |= (GPIOD_EN);
    RCC->APB1ENR   |= (CAN1_EN);

    // Set GPIOD PD0 (RX), PD1 (TX)
    GPIOD->MODER   &= ~(3U << 0);
    GPIOD->MODER   |=  (CAN_RX_PIN);
    GPIOD->MODER   &= ~(3U << 2);
    GPIOD->MODER   |=  (CAN_TX_PIN);

    GPIOD->AFR[0] &= ~(0xFU << 0);
    GPIOD->AFR[0] &= ~(0xFU << 4);
    GPIOD->AFR[0] |=  (9U << 0);
    GPIOD->AFR[0] |=  (9U << 4);    

    GPIOD->OSPEEDR |= (3U << 0);
    GPIOD->OSPEEDR |= (3U << 2);

    GPIOD->PUPDR   &= ~(3U << 0);
    GPIOD->PUPDR   |=  (1U << 0);

    /*********CAN1 Configuration********/
    CAN1->MCR |= (CAN_MCR_INRQ);
    CAN1->MCR &= ~(CAN_MCR_SLEEP);
    while (!(CAN1->MSR & CAN_MSR_INAK));

    CAN1->MCR |= (CAN_MCR_ABOM);
    
    // Baudrate 250 kbps
    CAN1->BTR = (0U  << 24) | 
                (1U  << 20) | 
                (10U << 16) | 
                (11U << 0);   

    CAN1->MCR &= ~(CAN_MCR_INRQ);
    while (CAN1->MSR & CAN_MSR_INAK);

    /*****************Bộ lọc Filter 0************/
    CAN1->FMR   |=  (CAN_FMR_FINIT); 
    CAN1->FA1R  &= ~(CAN_FA1R_FACT0);
    CAN1->FS1R  |=  (CAN_FS1R_FSC0); 
    CAN1->FM1R  &= ~(CAN_FM1R_FBM0); 
    CAN1->sFilterRegister[0].FR1 = 0x00000000;
    CAN1->sFilterRegister[0].FR2 = 0x00000000;
    CAN1->FFA1R &= ~(CAN_FFA1R_FFA0); // FIFO 0
    CAN1->FA1R  |=  (CAN_FA1R_FACT0);
    CAN1->FMR   &= ~(CAN_FMR_FINIT);

    /*************Cấu hình ngắt CAN**************/
    // Chỉ bật khi hàm IRQHandler bên dưới được định nghĩa!
    CAN1->IER |= (CAN_IER_FMPIE0);
    NVIC_SetPriority(CAN1_RX0_IRQn, 1);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);     
}

uint8_t CAN_Send_Frame(uint16_t stid, uint8_t *data, uint8_t dlc)
{
    // Tìm mailbox rảnh chính xác
    uint8_t mbx;
    if (CAN1->TSR & CAN_TSR_TME0)      mbx = 0;
    else if (CAN1->TSR & CAN_TSR_TME1) mbx = 1;
    else if (CAN1->TSR & CAN_TSR_TME2) mbx = 2;
    else return 0; // Hết mailbox trống

    CAN1->sTxMailBox[mbx].TIR  = ((uint32_t)stid << 21);
    CAN1->sTxMailBox[mbx].TDTR = (dlc & 0x0F);
    
    uint8_t payload[8] = {0};
    for (uint8_t i = 0; i < dlc && i < 8; i++)
    {
        payload[i] = data[i];
    }
    CAN1->sTxMailBox[mbx].TDLR = ((uint32_t)payload[3] << 24) |
                                 ((uint32_t)payload[2] << 16) |
                                 ((uint32_t)payload[1] << 8)  |
                                 ((uint32_t)payload[0]);

    CAN1->sTxMailBox[mbx].TDHR = ((uint32_t)payload[7] << 24) |
                                 ((uint32_t)payload[6] << 16) |
                                 ((uint32_t)payload[5] << 8)  |
                                 ((uint32_t)payload[4]);

    CAN1->sTxMailBox[mbx].TIR |= CAN_TI0R_TXRQ;
    return 1;
}

// BẮT BUỘC PHẢI MỞ HÀM NÀY NẾU ĐÃ BẬT NVIC_EnableIRQ
void CAN1_RX0_IRQHandler(void)
{
    if ((CAN1->RF0R & CAN_RF0R_FMP0) != 0)
    {
        g_rx_frame.id          = (CAN1->sFIFOMailBox[0].RIR >> 21) & 0x7FF;
        g_rx_frame.dlc         = (CAN1->sFIFOMailBox[0].RDTR & 0x0F);

        uint32_t low_reg       = CAN1->sFIFOMailBox[0].RDLR;
        uint32_t high_reg      = CAN1->sFIFOMailBox[0].RDHR;

        g_rx_frame.rx_data[0] = (uint8_t)(low_reg >> 0);
        g_rx_frame.rx_data[1] = (uint8_t)(low_reg >> 8);
        g_rx_frame.rx_data[2] = (uint8_t)(low_reg >> 16);
        g_rx_frame.rx_data[3] = (uint8_t)(low_reg >> 24);

        g_rx_frame.rx_data[4] = (uint8_t)(high_reg >> 0);
        g_rx_frame.rx_data[5] = (uint8_t)(high_reg >> 8);
        g_rx_frame.rx_data[6] = (uint8_t)(high_reg >> 16);
        g_rx_frame.rx_data[7] = (uint8_t)(high_reg >> 24);

        g_rx_frame.is_new_data = 1;

        CAN1->RF0R |= CAN_RF0R_RFOM0; // Xả mailbox hạ cờ ngắt
    }
}