/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include "scaleTable10.h"

#define TITLE_STR1          "PSoC4 DCO"
#define TITLE_STR2          "20160507"

#define SAMPLING_CLOCK      12000000ul

#define WAVESHAPE_SQUARE    0
#define WAVESHAPE_SAW       1
#define WAVESHAPE_N         2

volatile uint8 count;
volatile uint8 flag;

uint8 waveShape = WAVESHAPE_SAW;
uint8 noteNumber;
int32 frequency10;
uint8 squareDuty = 128;

int cnt1, cnt2 = 69, cnt3;

CY_ISR(ISR_Saw_handler)
{
    Pin_Check1_Write(flag);
    flag = flag ? 0 : 1;
    
    count++;
    IDAC8_SetValue(count);
 
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);
}

CY_ISR(ISR_Square_handler)
{
    Pin_Check2_Write(flag);
    flag = flag ? 0 : 1;
    
    count++;
    IDAC8_SetValue((count / squareDuty) ? 255 : 0);    
    
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);
}

int main()
{
    uint16 timerPeriod;
    //int32 frequency = 1000;

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
    
    Timer_Sampling_Start();
    ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
    
    LCD_Char_ClearDisplay();
    LCD_Char_PrintString("Initialize OK.");
    
    for(;;)
    {
        /* Place your application code here. */
                
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
        
        if (Pin_SW3_Read() == 0u) {
            cnt3++;
            if (cnt3 % 2) {
                ISR_Timer_Sampling_StartEx(ISR_Square_handler);
            } else {
                ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
            }
        }
        
        frequency10 = scaleTable10[noteNumber];
        timerPeriod = SAMPLING_CLOCK * 10 / (frequency10 * 256);
        
        Timer_Sampling_WritePeriod(timerPeriod);

        LCD_Char_ClearDisplay();
        LCD_Char_PrintString("Note:");
        LCD_Char_PrintNumber(noteNumber);
        LCD_Char_PrintString(" F:");
        LCD_Char_PrintNumber(frequency10);
        LCD_Char_Position(1, 0);
        LCD_Char_PrintString("P:");
        LCD_Char_PrintNumber(timerPeriod);
        LCD_Char_PrintString(" ");
        LCD_Char_PrintNumber(cnt1);
        LCD_Char_PrintString(" :");
        LCD_Char_PrintNumber(cnt2);
        LCD_Char_PrintString(" :");
        LCD_Char_PrintNumber(cnt3);

        CyDelay(200);
    }
}

/* [] END OF FILE */
