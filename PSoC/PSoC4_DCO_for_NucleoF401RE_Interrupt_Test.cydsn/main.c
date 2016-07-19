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
#define LCD_DISPLAY (0)

#define TITLE_STR1  ("PSoC 4 DCO")
#define TITLE_STR2  ("20160609")

#define SAMPLING_CLOCK           (24000000)
#define SPIS_RX_PACKET_SIZE      (5)
#define SPIS_RX_PACKET_HEADER    (0x55)

#define WAVESHAPE_SQUARE    0
#define WAVESHAPE_SAW       1
#define WAVESHAPE_N         2

volatile uint8 count = 0;
volatile uint16 timerPeriod = 0;
volatile int toChangePeriod = 0;

uint8 waveShape   = WAVESHAPE_N;
uint8 squareDuty  = 127;
int32 frequency10 = 4400;

#if(LCD_DISPLAY)
void printLCD(uint8* rxBuffer)
{
    char strBuffer[20];
    sprintf(strBuffer, "%3u %3u %3u %3u",
    rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3]);
    LCD_Char_Position(0, 0);
    LCD_Char_PrintString(strBuffer);
    sprintf(strBuffer, "%3u %5ld", rxBuffer[4], frequency10);
    LCD_Char_Position(1, 0);
    LCD_Char_PrintString(strBuffer);
}
#endif

CY_ISR(ISR_Saw_handler)
{
    Pin_Check2_Write(1);
    
    count++;
        if (count == 0 && toChangePeriod) {
        Timer_Sampling_WritePeriod(timerPeriod);
        toChangePeriod = 0;
    }
    IDAC8_SetValue(count);
 
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);
    
    Pin_Check2_Write(0);
}

CY_ISR(ISR_Square_handler)
{
    Pin_Check2_Write(1);
    
    count++;
    if (count == 0 && toChangePeriod) {
        Timer_Sampling_WritePeriod(timerPeriod);
        toChangePeriod = 0;
    }
    IDAC8_SetValue((count / squareDuty) ? 255 : 0);    
    
    Timer_Sampling_ClearInterrupt(Timer_Sampling_INTR_MASK_TC);

    Pin_Check2_Write(0);    
}

void doCommand(uint8 *rxBuffer)
{
    //uint16 timerPeriod;
    
    if (waveShape != rxBuffer[1]) {
        waveShape = rxBuffer[1];
        switch (waveShape) {
        case WAVESHAPE_SQUARE:
            ISR_Timer_Sampling_SetVector(ISR_Square_handler);
            break;
        case WAVESHAPE_SAW:
            ISR_Timer_Sampling_SetVector(ISR_Saw_handler);
            break;
        }
    }

    squareDuty = rxBuffer[2];
    frequency10 = ((uint16)rxBuffer[3] << 8) | rxBuffer[4];
    
    timerPeriod = (uint64)SAMPLING_CLOCK * 10 / (frequency10 * 256);
    toChangePeriod = 1;
    //Timer_Sampling_WritePeriod(timerPeriod);
}

void SPI_RX_handler()
{
    uint8 rxBuffer[SPIS_RX_PACKET_SIZE];
    int i;
    
    Pin_Check1_Write(1);
    
    rxBuffer[0] = SPIS_SpiUartReadRxData();
    if (SPIS_RX_PACKET_HEADER == rxBuffer[0]) {
        for (i = 1; i < SPIS_RX_PACKET_SIZE; i++) {
            rxBuffer[i] = SPIS_SpiUartReadRxData();
        }
        doCommand(rxBuffer);
    }
    
    SPIS_ClearRxInterruptSource(SPIS_INTR_RX_FIFO_LEVEL);
    
    Pin_Check1_Write(0);
}

int main()
{
    // Initial parameter
    uint8 dummyRxBuffer[SPIS_RX_PACKET_SIZE] = {
        SPIS_RX_PACKET_HEADER,
        WAVESHAPE_SAW,
        squareDuty,
        (frequency10 >> 8),
        (frequency10 & 0xff)
    };
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    #if(UART_TRACE)
    UART_Start();
    UART_UartPutString("PSoC 4 SPI Slave Test\r\n");
    #endif
    
    #if(LCD_DISPLAY)
    LCD_Char_Start();
    LCD_Char_ClearDisplay();
    LCD_Char_PrintString(TITLE_STR1);
    LCD_Char_Position(1, 0);
    LCD_Char_PrintString(TITLE_STR2);
    CyDelay(1000);
    #endif
    
    IDAC8_Start();
    Opamp_IV_Conv_Start();

    Timer_Sampling_Start();
    ISR_Timer_Sampling_StartEx(ISR_Saw_handler);
    doCommand(dummyRxBuffer);
    
    SPIS_SetCustomInterruptHandler(SPI_RX_handler);
    SPIS_Start();

    for(;;)
    {
    }
}

/* [] END OF FILE */
