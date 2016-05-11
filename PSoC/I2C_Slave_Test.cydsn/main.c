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


/***************************************
*         Function Prototypes
****************************************/

uint8 ExecuteCommand(uint32 cmd);


/***************************************
*            Constants
****************************************/

/* Buffer and packet size */
#define BUFFER_SIZE      (6u)
#define PACKET_SIZE      (BUFFER_SIZE / 2u)
#define READ_ONLY_OFFSET (3u)

/* Packet positions */
#define PACKET_SOP_POS  (0u)
#define PACKET_CMD_POS  (1u)
#define PACKET_STS_POS  (4u)
#define PACKET_EOP_POS  (2u)

/* Start and end of packet markers */
#define PACKET_SOP      (0x01u)
#define PACKET_EOP      (0x17u)

/* Command valid status */
#define STS_CMD_DONE    (0x00u)
#define STS_CMD_FAIL    (0xFFu)

/* Commands set */
#define CMD_SET_OFF     (0u)
#define CMD_SET_RED     (1u)
#define CMD_SET_GREEN   (2u)
#define CMD_SET_BLUE    (3u)


/***************************************
*               Macros
****************************************/

/* Set LED RED color */
#define RGB_LED_ON_RED  \
                do{     \
                    LED_RED_Write  (0u); \
                    LED_GREEN_Write(1u); \
                    LED_BLUE_Write (1u); \
                }while(0)

/* Set LED GREEN color */
#define RGB_LED_ON_GREEN \
                do{      \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(0u); \
                    LED_BLUE_Write (1u); \
                }while(0)

/* Set LED BLUE color */
#define RGB_LED_ON_BLUE \
                do{     \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(1u); \
                    LED_BLUE_Write (0u); \
                }while(0)

/* Set LED TURN OFF */
#define RGB_LED_OFF \
                do{ \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(1u); \
                    LED_BLUE_Write (1u); \
                }while(0)

/* EZI2C slave buffer for write and read */
uint8 ezI2cBuffer[BUFFER_SIZE] = {PACKET_SOP, STS_CMD_FAIL, PACKET_EOP, \
                                  PACKET_SOP, STS_CMD_FAIL, PACKET_EOP};

int main()
{
    uint8 status = STS_CMD_FAIL;

    RGB_LED_OFF;
    
    /* Setup buffer and start EZI2C slave (SCB mode) */
    EZI2C_EzI2CSetBuffer1(BUFFER_SIZE, READ_ONLY_OFFSET, ezI2cBuffer);
    EZI2C_Start();
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    for(;;)
    {
        /* Write complete: parse packet */
        if (0u != (EZI2C_EzI2CGetActivity() & EZI2C_EZI2C_STATUS_WRITE1))
        {
            /* Check start and end of packet markers */
            if ((ezI2cBuffer[PACKET_SOP_POS] == PACKET_SOP) &&
                (ezI2cBuffer[PACKET_EOP_POS] == PACKET_EOP))
            {
                status = ExecuteCommand(ezI2cBuffer[PACKET_CMD_POS]);
            }

            /* Update buffer with status */
            ezI2cBuffer[PACKET_STS_POS] = status;
            status = STS_CMD_FAIL;
        }

        /* Buffer is always available to be read */
    }
}

uint8 ExecuteCommand(uint32 cmd)
{
    uint8 status;

    status = STS_CMD_DONE;

    /* Execute received command */
    switch (cmd)
    {
        case CMD_SET_RED:
            RGB_LED_ON_RED;
            break;

        case CMD_SET_GREEN:
            RGB_LED_ON_GREEN;
            break;

        case CMD_SET_BLUE:
            RGB_LED_ON_BLUE;
            break;

        case CMD_SET_OFF:
            RGB_LED_OFF;
            break;

        default:
            status = STS_CMD_FAIL;
        break;
    }

    return(status);
}

/* [] END OF FILE */
