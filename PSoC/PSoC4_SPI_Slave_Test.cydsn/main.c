/* ========================================
 *
 * SPI Slaveの送受信テスト
 * 
 * 2016.05.09
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>

#define RX_PACKET_SIZE  3
#define TX_PACKET_SIZE  3

// Prototype
int readRE();

int main()
{
    int i;
    int txData;
    uint8 rxBuffer[RX_PACKET_SIZE];
    uint8 txBuffer[TX_PACKET_SIZE];
    
    char strBuffer[80];
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    UART_Start();
    UART_UartPutString("SPI Slave test.\r\n");
    
    LCD_Char_Start();
    LCD_Char_PrintString("SPI Slave Test");

    SPIS_Start();

    txData = 0;
    for(;;)
    {
        txData += readRE();
        
        /*
        while (RX_PACKET_SIZE > SPIS_SpiUartGetRxBufferSize())
        {
            txData += readRE();
        }
        */
        if (RX_PACKET_SIZE <= SPIS_SpiUartGetRxBufferSize()) {
            // RX
            for (i = 0; i < RX_PACKET_SIZE; i++) {
                rxBuffer[i] = SPIS_SpiUartReadRxData();
            }

            // TX
            txBuffer[0] = rxBuffer[0];
            txBuffer[1] = txData >> 8;
            txBuffer[2] = txData & 0xff;
            SPIS_SpiUartPutArray(txBuffer, TX_PACKET_SIZE);
            
            //sprintf(strBuffer, "RX: %03u %03u %03u\r\n", rxBuffer[0], rxBuffer[1], rxBuffer[2]);
            sprintf(strBuffer, "RX: %03u %4d   \r\n", rxBuffer[0], ((uint16)rxBuffer[1] << 8) | rxBuffer[2]);
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
        //CyDelay(1);
    }
}

//-------------------------------------------------
// ロータリーエンコーダの読み取り
// return: ロータリーエンコーダーの回転方向
//         0:変化なし 1:時計回り -1:反時計回り
//
int readRE()
{
    static uint8_t index;
    uint8_t rd = 0;
    int retval = 0;
    
    rd = Pin_RE_Read();
    index = (index << 2) | rd;
	index &= 0b1111;

	switch (index) {
	// 時計回り
	case 0b0001:	// 00 -> 01
	case 0b1110:	// 11 -> 10
	    retval = 1;
	    break;
	// 反時計回り
	case 0b0010:	// 00 -> 10
	case 0b1101:	// 11 -> 01
	    retval = -1;
	    break;
    }
    return retval;
}

/* [] END OF FILE */
