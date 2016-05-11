/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * 2016.05.07 Created
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>

#define TITLE_STR1          "PSoC4 DCO"
#define TITLE_STR2          "20160511"

#define SAMPLING_CLOCK      12000000ul

#define WAVESHAPE_SQUARE    0
#define WAVESHAPE_SAW       1
#define WAVESHAPE_N         2

#define SPIS_RX_PACKET_SIZE  3
#define SPIS_TX_PACKET_SIZE  3

volatile uint8 count;

uint8 waveShape = WAVESHAPE_SAW;
int32 frequency10;
uint8 squareDuty;

CY_ISR(ISR_Saw_handler)
{
    Pin_Check2_Write(1);
    
    count++;
    IDAC8_SetValue(count);
 
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);
    
    Pin_Check2_Write(0);
}

CY_ISR(ISR_Square_handler)
{
    Pin_Check2_Write(1);
    
    count++;
    IDAC8_SetValue((count / squareDuty) ? 255 : 0);    
    
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);

    Pin_Check2_Write(0);    
}

int main()
{
    uint16 timerPeriod;
    //int32 frequency = 1000;
    int i;
    //int txData;
    uint8 rxBuffer[SPIS_RX_PACKET_SIZE];
    uint8 txBuffer[SPIS_TX_PACKET_SIZE];
    char strBuffer[80];

    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
//    UART_Start();
//    UART_UartPutString("Hello\r\n");
//    UART_UartPutString("Put Note Number\r\n");
//    
    LCD_Char_Start();
    LCD_Char_PrintString(TITLE_STR1);
    LCD_Char_Position(1, 0);
    LCD_Char_PrintString(TITLE_STR2);
    CyDelay(2000);

    IDAC8_Start();
    Opamp_IV_Conv_Start();
    
    LCD_Char_ClearDisplay();
    LCD_Char_PrintString("Initialize OK.");
    //CyDelay(2000);
    
    SPIS_Start();

    Timer_Sampling_Start();
    ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
    
    frequency10 = 4400;
    squareDuty = 127;
    for(;;)
    {
        Pin_Check1_Write(1);

        if (Pin_SW3_Read() == 0u) {
            waveShape++;
            if (waveShape >= WAVESHAPE_N) {
                waveShape = WAVESHAPE_SQUARE;
            }
            switch (waveShape) {
            case WAVESHAPE_SQUARE:
                ISR_Timer_Sampling_StartEx(ISR_Square_handler);
                break;
            case WAVESHAPE_SAW:
                ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
                break;
            }
        }
        
        if (Pin_SW4_Read() == 0u) {
            squareDuty -= 10;
        }
        if (Pin_SW5_Read() == 0u) {
            squareDuty += 10;
        }
        
        if (SPIS_RX_PACKET_SIZE <= SPIS_SpiUartGetRxBufferSize()) {
            // RX
            for (i = 0; i < SPIS_RX_PACKET_SIZE; i++) {
                rxBuffer[i] = SPIS_SpiUartReadRxData();
            }
            frequency10 = ((uint16)rxBuffer[1] << 8) | rxBuffer[2];

            // TX
            txBuffer[0] = rxBuffer[0];
            txBuffer[1] = rxBuffer[1];
            txBuffer[2] = rxBuffer[2];
            SPIS_SpiUartPutArray(txBuffer, SPIS_TX_PACKET_SIZE);
        
            // コマンドの実行
            timerPeriod = SAMPLING_CLOCK * 10 / (frequency10 * 256);
            Timer_Sampling_WritePeriod(timerPeriod);
            
            // LCDに表示
            sprintf(strBuffer, "RX: %03u %03u %03u   \r\n", rxBuffer[0], rxBuffer[1], rxBuffer[2]);
            UART_UartPutString(strBuffer);
                
            LCD_Char_ClearDisplay();
            strBuffer[15] = 0;
            LCD_Char_Position(0, 0);
            LCD_Char_PrintString(strBuffer);

            sprintf(strBuffer, "%ld %d %d     ", frequency10, timerPeriod, squareDuty);
            UART_UartPutString(strBuffer);

            LCD_Char_Position(1, 0);
            LCD_Char_PrintString(strBuffer);
        }    
        Pin_Check1_Write(0);
        //CyDelay(200);
    }
}

/* [] END OF FILE */
