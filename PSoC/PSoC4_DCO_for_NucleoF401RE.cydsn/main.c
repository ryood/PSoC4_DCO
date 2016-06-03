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
#include <stdio.h>

#define UART_TRACE  (0)
#define TITLE_STR1  ("PSoC 4 DCO")
#define TITLE_STR2  ("20160603")

#define SAMPLING_CLOCK           (24000000)
#define SPIS_RX_PACKET_SIZE      (5)
#define SPIS_RX_PACKET_HEADER    (0x55)

#define WAVESHAPE_SQUARE    0
#define WAVESHAPE_SAW       1
#define WAVESHAPE_N         2

volatile uint8 count;

uint8 waveShape   = WAVESHAPE_SQUARE;
uint8 squareDuty  = 127;
int32 frequency10 = 4400;

void printLCD(uint8* rxBuffer)
{
    char strBuffer[20];
    sprintf(strBuffer, "%3u %3u %3u %3u",
    rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3]);
    LCD_Char_Position(0, 0);
    LCD_Char_PrintString(strBuffer);
    sprintf(strBuffer, "%3u %ld", rxBuffer[4], frequency10);
    LCD_Char_Position(1, 0);
    LCD_Char_PrintString(strBuffer);
}

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

void doCommand(uint8 *rxBuffer)
{
    uint16 timerPeriod;
    
    //waveShape = rxBuffer[1];
    waveShape = WAVESHAPE_SAW;
    squareDuty = rxBuffer[2];
    frequency10 = ((uint16)rxBuffer[3] << 8) | rxBuffer[4];
    
    switch (waveShape) {
    case WAVESHAPE_SQUARE:
        ISR_Timer_Sampling_StartEx(ISR_Square_handler);
        break;
    case WAVESHAPE_SAW:
        ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
        break;
    }
    
    timerPeriod = (uint64)SAMPLING_CLOCK * 10 / (frequency10 * 256);
    Timer_Sampling_WritePeriod(timerPeriod);
}

int main()
{
    uint8 rxBuffer[SPIS_RX_PACKET_SIZE];
    int i;
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    #if(UART_TRACE)
    UART_Start();
    UART_UartPutString("PSoC 4 SPI Slave Test\r\n");
    #endif
    
    LCD_Char_Start();
    LCD_Char_ClearDisplay();
    LCD_Char_PrintString(TITLE_STR1);
    LCD_Char_Position(1, 0);
    LCD_Char_PrintString(TITLE_STR2);
    CyDelay(1000);
    
    IDAC8_Start();
    Opamp_IV_Conv_Start();

    Timer_Sampling_Start();
    ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
    
    SPIS_Start();

    for(;;)
    {
        Pin_Check1_Write(1);
        
        if (SPIS_RX_PACKET_SIZE <= SPIS_SpiUartGetRxBufferSize()) {
            // Check Packet Header
            rxBuffer[0] = SPIS_SpiUartReadRxData();
            if (SPIS_RX_PACKET_HEADER != rxBuffer[0]) {
                break;
            }
            for (i = 1; i < SPIS_RX_PACKET_SIZE; i++) {
                rxBuffer[i] = SPIS_SpiUartReadRxData();
            }

            doCommand(rxBuffer);
            printLCD(rxBuffer);
            
            #if(UART_TRACE)
            char strBuffer[80];
            sprintf(strBuffer, "%d\t%d\t%d\t%d\t%d\t%ld\r\n",
                rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3], rxBuffer[4],
                frequency10
            );
            UART_UartPutString(strBuffer);
            #endif     
        }
        
        Pin_Check1_Write(0);
    }
}

/* [] END OF FILE */
