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

#define SAMPLING_CLOCK      12000000ul

#define WAVESHAPE_SQUARE    0
#define WAVESHAPE_SAW       1
#define WAVESHAPE_N         2

volatile uint8 count;
volatile uint8 flag;

uint8 waveShape = WAVESHAPE_SAW;
uint8 noteNumber;
int32 frequency10;

int cnt1, cnt2 = 69;

CY_ISR(ISR_Timer_Sampling_handler)
{
    Pin_Check1_Write(flag);
    flag = flag ? 0 : 1;
    
    count++;
    IDAC8_SetValue(count);
/*    
    switch (waveShape) {
    case WAVESHAPE_SQUARE:
        break;
    case WAVESHAPE_SAW:
        break;
    default:
        ;
    }
*/    
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
    LCD_Char_PrintString("Hello");
    CyDelay(500);

    LCD_Char_PrintString("before idac8");
    IDAC8_Start();
    Opamp_IV_Conv_Start();
    
    LCD_Char_Position(0, 0);
    LCD_Char_PrintString("before timer ");
    
    Timer_Sampling_Start();
    ISR_Timer_Sampling_StartEx(ISR_Timer_Sampling_handler);
    
    LCD_Char_Position(0, 0);
    LCD_Char_PrintString("init ok");
    
    for(;;)
    {
        /* Place your application code here. */
        /*

        */
                
        if (Pin_SW1_Read() == 0u) {
            cnt1++;
        }
        if (Pin_SW2_Read() == 0u) {
            cnt2++;
        }
        noteNumber = cnt2 - cnt1;
        
        if (noteNumber >= 128) {
            noteNumber = 127;
        }
        frequency10 = scaleTable10[noteNumber];
        timerPeriod = (int64)SAMPLING_CLOCK * 10 / (frequency10 * 256);
        
        Timer_Sampling_WritePeriod(timerPeriod);

        LCD_Char_ClearDisplay();
        LCD_Char_PrintString("Note:");
        LCD_Char_PrintNumber(noteNumber);
        LCD_Char_PrintString(" F:");
        LCD_Char_PrintNumber(frequency10);
        LCD_Char_Position(1, 0);
        LCD_Char_PrintString("P:");
        LCD_Char_PrintNumber(timerPeriod);
        LCD_Char_PrintString(" :");
        LCD_Char_PrintNumber(cnt1);
        LCD_Char_PrintString(" :");
        LCD_Char_PrintNumber(cnt2);

        CyDelay(100);
    }
}

/* [] END OF FILE */
