#include "stm32f4xx.h"
#include "CAN.h"
#include "gpio.h"
#include "timer.h"

int main(void)
{
    led_init();
    Timer2_Init();
    CAN_Init();

    // Frame gửi sang ESP32: ID 0x123, 4 bytes [0x11, 0x22, 0x33, 0x44]
    uint8_t tx_payload[4] = {0x11, 0x22, 0x33, 0x44};

    while (1)
    {
        CAN_Send_Frame(0x123, tx_payload, 4);
        Delay_ms_TIM2(500);

        if (g_rx_frame.is_new_data)
        {
            g_rx_frame.is_new_data = 0;
            if (g_rx_frame.id == 0x321 && g_rx_frame.rx_data[0] == 0xDE)
            {
                Toggle_led7();
            }
        }
        Delay_ms_TIM2(500);
    }
}