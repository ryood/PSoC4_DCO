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

#define UART_TRACE  (1)
#define TITLE_STR1  ("PSoC 4 DCO")
#define TITLE_STR2  ("20160603")

#define SPIS_RX_PACKET_SIZE      (5)
#define SPIS_RX_PACKET_HEADER    (0x55)

int main()
{
    uint8 rxBuffer[SPIS_RX_PACKET_SIZE];
    char strBuffer[80];
    int i;
    uint16 frequency10;
    
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

    SPIS_Start();
        
    for(;;)
    {
        Pin_Check1_Write(1);
        
        // Check Packet Header
        if (SPIS_RX_PACKET_SIZE <= SPIS_SpiUartGetRxBufferSize()) {
            rxBuffer[0] = SPIS_SpiUartReadRxData();
            if (SPIS_RX_PACKET_HEADER != rxBuffer[0]) {
                break;
            }
            for (i = 1; i < SPIS_RX_PACKET_SIZE; i++) {
                rxBuffer[i] = SPIS_SpiUartReadRxData();
            }
            frequency10 = ((uint16)rxBuffer[3] << 8) | rxBuffer[4];
            
            sprintf(strBuffer, "%3u %3u %3u %3u",
                rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3]);
            LCD_Char_Position(0, 0);
            LCD_Char_PrintString(strBuffer);
            sprintf(strBuffer, "%3u %u", rxBuffer[4], frequency10);
            LCD_Char_Position(1, 0);
            LCD_Char_PrintString(strBuffer);
            
            #if(UART_TRACE)
            sprintf(strBuffer, "%d\t%d\t%d\t%d\t%d\t%d\r\n",
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
