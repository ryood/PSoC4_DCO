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
#include "scaleTable10.h"

#define TITLE_STR1          "PSoC4 DCO"
#define TITLE_STR2          "20160511"

#define SAMPLING_CLOCK      24000000ul

#define WAVESHAPE_SQUARE    0
#define WAVESHAPE_SAW       1
#define WAVESHAPE_N         2

#define SPIS_RX_PACKET_SIZE  3
#define SPIS_TX_PACKET_SIZE  3

volatile uint8 count;
volatile uint8 flag;

uint8 waveShape = WAVESHAPE_SAW;
uint8 noteNumber;
int32 frequency10;
uint8 squareDuty = 128;

int cnt1, cnt2 = 69, cnt3;

CY_ISR(ISR_Saw_handler)
{
    //Pin_Check1_Write(flag);
    //flag = flag ? 0 : 1;
    
    count++;
    IDAC8_SetValue(count);
 
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);
}

CY_ISR(ISR_Square_handler)
{
    //Pin_Check2_Write(flag);
    //flag = flag ? 0 : 1;
    
    count++;
    IDAC8_SetValue((count / squareDuty) ? 255 : 0);    
    
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);
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
    for(;;)
    {
        Pin_Check1_Write(1);
        
        /* Place your application code here. */
        /*        
        if (Pin_SW1_Read() == 0u) {
            cnt1++;
        }
        if (Pin_SW2_Read() == 0u) {
            cnt2++;
        }
        noteNumber = cnt2 - cnt1;
        
        if (noteNumber >= 128) {
            noteNumber = 127;
            cnt1 = 0;
            cnt2 = 127;
        }
        */
        if (Pin_SW3_Read() == 0u) {
            cnt3++;
            if (cnt3 % 2) {
                ISR_Timer_Sampling_StartEx(ISR_Square_handler);
            } else {
                ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
            }
        }
        
        if (Pin_SW4_Read() == 0u) {
            squareDuty -= 10;
        }
        if (Pin_SW5_Read() == 0u) {
            squareDuty += 10;
        }
                
        /*
        frequency10 = scaleTable10[noteNumber];
        timerPeriod = SAMPLING_CLOCK * 10 / (frequency10 * 256);
        */
        
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
            
            //sprintf(strBuffer, "RX: %03u %03u %03u\r\n", rxBuffer[0], rxBuffer[1], rxBuffer[2]);
            sprintf(strBuffer, "RX: %03u %4ld   \r\n", rxBuffer[0], frequency10);
            UART_UartPutString(strBuffer);
            
            LCD_Char_ClearDisplay();
            strBuffer[15] = 0;
            LCD_Char_Position(0, 0);
            LCD_Char_PrintString(strBuffer);
            
            sprintf(strBuffer, "TX: %03u %03u %03u\r\n", txBuffer[0], txBuffer[1], txBuffer[2]);
            //sprintf(strBuffer, "txData:%d    \r\n", txData);
            UART_UartPutString(strBuffer);

            strBuffer[15] = 0;
            LCD_Char_Position(1, 0);
            LCD_Char_PrintString(strBuffer);
        }
        
        timerPeriod = SAMPLING_CLOCK * 10 / (frequency10 * 256);
        Timer_Sampling_WritePeriod(timerPeriod);
        
        //sprintf(strBuffer, "RX: %03u %03u %03u\r\n", rxBuffer[0], rxBuffer[1], rxBuffer[2]);
        sprintf(strBuffer, "RX: %03u %4d   \r\n", rxBuffer[0], ((uint16)rxBuffer[1] << 8) | rxBuffer[2]);
        UART_UartPutString(strBuffer);
            
        LCD_Char_ClearDisplay();
        strBuffer[15] = 0;
        LCD_Char_Position(0, 0);
        LCD_Char_PrintString(strBuffer);

        sprintf(strBuffer, "P:%d D:%d", timerPeriod, squareDuty);
        UART_UartPutString(strBuffer);

        LCD_Char_Position(1, 0);
        LCD_Char_PrintString(strBuffer);

        /*
        LCD_Char_ClearDisplay();
        LCD_Char_PrintString("N");
        LCD_Char_PrintNumber(noteNumber);
        LCD_Char_PrintString(" F");
        LCD_Char_PrintNumber(frequency10);
        LCD_Char_PrintString(" P");
        LCD_Char_PrintNumber(timerPeriod);
        LCD_Char_Position(1, 0);
        LCD_Char_PrintNumber(cnt1);
        LCD_Char_PrintString(":");
        LCD_Char_PrintNumber(cnt2);
        LCD_Char_PrintString(":");
        LCD_Char_PrintNumber(cnt3);
        LCD_Char_PrintString(" D:");
        LCD_Char_PrintNumber(squareDuty);
        */
        
        //CyDelay(200);
        Pin_Check1_Write(0);
    }
}

/* [] END OF FILE */
