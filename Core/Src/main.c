#include "stm32f407xx.h"
#include "CAN.h"
#include "timer.h"
#include "uart.h"

int main(void)
{
    Timer2_Init();
    UART1_Init();
    CAN_Init();

    Send_String("\r\n==========================================\r\n");
    Send_String("HE THONG DIEU KHIEN CANOPEN ATV312 SAN SANG\r\n");
    Send_String("==========================================\r\n");

    ATV312_SDO_Power_Init(ATV312_NODE_ID);
    Delay_ms_TIM2(100);

    // Toc do muc tieu ban dau
    ATV312_SDO_Set_Speed_Hz(ATV312_NODE_ID, 4.0f);
    Send_String("-> Da dat toc do: 30.0 Hz\r\n");

    float actual_hz = 0.0f;
    float current_a = 0.0f;
    char str_buf[16];

    while (1)
    {
        /* ================= 1. Chay thuan chieu 5 GIÂY ================= */
        Send_String("\r\n>>> CHAY THUAN (0x000F) <<<\r\n");
        for (uint8_t i = 0; i < 20; i++)
        {
            ATV312_SDO_Run(ATV312_NODE_ID, DIR_FORWARD);

            if (ATV312_SDO_Get_Speed_Hz(ATV312_NODE_ID, &actual_hz) == 1)
            {
                Send_String("[FWD] Speed: ");
                Float_To_String_1Dec(actual_hz, str_buf);
                Send_String(str_buf);
                Send_String(" Hz | ");
            }

            if (ATV312_SDO_Get_Current_A(ATV312_NODE_ID, &current_a) == 1)
            {
                Send_String("Current: ");
                Float_To_String_1Dec(current_a, str_buf);
                Send_String(str_buf);
                Send_String(" A\r\n");
            }

            Delay_ms_TIM2(250);
        }

        /* ================= 2. Dao chieu nghich 5 GIÂY ================= */
        Send_String("\r\n>>> DAO CHIEU NGHICH (0x080F) <<<\r\n");
        for (uint8_t i = 0; i < 20; i++)
        {
            ATV312_SDO_Run(ATV312_NODE_ID, DIR_REVERSE);

            if (ATV312_SDO_Get_Speed_Hz(ATV312_NODE_ID, &actual_hz) == 1)
            {
                Send_String("[REV] Speed: ");
                Float_To_String_1Dec(actual_hz, str_buf);
                Send_String(str_buf);
                Send_String(" Hz | ");
            }

            if (ATV312_SDO_Get_Current_A(ATV312_NODE_ID, &current_a) == 1)
            {
                Send_String("Current: ");
                Float_To_String_1Dec(current_a, str_buf);
                Send_String(str_buf);
                Send_String(" A\r\n");
            }

            Delay_ms_TIM2(250);
        }
    }
}