
/*************************************************
 * I2C module for MSP4305529LP
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 23.02.2022 (dd::mm::yyyy)
 * Date Modified: 23.02.2022 (dd::mm::yyyy)
 * 
 * **********************************************
 * 
 *              MSP4305529LP          PixyCam               Big Connector /PixyCam/
 *             ---------------       ---------               ----------
 *            |            5V | ->  | P2      |             |   1   2  |        1 <- SPI MISO, UART Rx      6 <- GND
 *            |               |     |         |             |   3   4  |        2 <- 5V (IN or OUT)         7 <- SPI SS
 *            |     3.1 (SCL) | ->  | P5      |             |   5   6  |        3 <- SPI SCK                8 <- Analog Out
 *            |     3.0 (SDA) | ->  | P9      |             |   7   8  |        4 <- SPI MOSI, UART Tx      9 <- I2C SDA
 *            |               |     |         |             |   9   10 |        5 <- I2C SCL               10 <- Vin (6 - 10V)
 *            |           GND | ->  | P6      |              ----------          
 *            |               |      ---------              
 *             ---------------                               Small Connector /PixyCam/
 *                                                        ---------------------------
 *                                                       |   PWM0 (pan)  PWM1 (tilt) |
 *                                                       |   Vout        Vout        |
 *                                                       |   GND         GND         |
 *                                                        ---------------------------
 * NB: Looks nice :)
 * 
 * NOTE:
 * From MSP4305529LP, two pull-down resistors are connected to P3.1 and P3.0. 
 * They are used as voltage dividers, since an input voltage to MSP's pins 
 * cannot exceed 3.3V.
 * Also, PixyCam already has two pull-up resistors coonected to the SCL and SDA pins.
 * They are 82 kOhm.
 * 
 * NOTE on interrupts:
 *      UCB0CTL1 |= UCSWRST resets UCB0IE as well
 *
 * ************************************************/

#include <msp430.h>
#include "usciI2C.h"
#include "pixyCam2.h"
unsigned char stopBit = 0;

/*********************************************************************
 * Name: usciB0I2CInit 
 * 
 * Description: usciB0I2cInit initialises I2C protocol on MSP4305529LP
 * 
 * Arguments: sclkDiv   -  sub-muster clock deviser
 * 
 * Return: none, for it owes you nothing 
 * 
 * Author: Iakov Umrikhin
 * Date Created: 23.02.2022 (dd::mm::yyyy)
 * Date Modified: 28.02.2022 (dd::mm::yyyy)
 * ******************************************************************/

void usciB0I2CInit (unsigned int sclkDiv) {
    UCB0CTL1 |= UCSSEL_2 | UCSWRST;     // SMCLK; reset ON

    // Configure Registers
    // Control registers
    UCB0CTL0 |= UCMST | UCMODE_3 | UCSYNC;  // Master Mode; I2C mode; synchronous mode
    UCB0CTL0 &= ~(UCA10 | UCSLA10 | UCMM);   // Own address: 7 bit; Slave address: 7 bit; Single-master mode

    // I2C own address register
    UCB0I2COA = 0;  // do not respond to general call; own address = 0;

    // Use only P3.1 (SCL) and P3.0 (SDA)
    P3SEL |= BIT0 | BIT1;

    // Configure Ports
    UCB0CTL1 &= ~UCSWRST;
    // Leave reset condition

    // Clock divider
    UCB0BR0 = sclkDiv & 0xff;
    UCB0BR1 = sclkDiv >> 8;
}
/*********************************************************************
 * Name: usciB0MstTx 
 * 
 * Description: usciB0MstTx sends a string of bytes to PixyCam v2.
 *              It is little-endian byte ordering, i.e. LSB first;
 * 
 * Arguments: nBytes    -   a number of bytes in a string to be sent
 *            txBuffer  -   a string of bytes
 *            slaveAddr -   the address of the slave; not a register address
 * 
 * Return: 0 if nBytes have been sent successfully; -1 if it is all in vain
 * 
 * Author: Iakov Umrikhin
 * Date Created: 23.02.2022 (dd::mm::yyyy)
 * Date Modified: 28.02.2022 (dd::mm::yyyy)
 * ******************************************************************/

signed char usciB0MstTx (signed char nBytes, char slaveAddr, char *txBuffer) {

    signed char result = -1;
    unsigned char txCounter = 0;

    // Configure as a transmitter
    SEND_SA(slaveAddr);
    MST_TX_MODE;
    // Initialise START bit
    START_BIT;

    // send Addresss
    // ACK is issued
    while(txCounter < nBytes && !stopBit) {
        WAIT_TX_IFG;

        UCB0TXBUF = txBuffer[txCounter];

        txCounter++;
        result = 0;
    }

    if (!stopBit) {
        WAIT_TX_IFG;
        STOP_BIT;
    }
    stopBit = 0;

    return result;
}
/*********************************************************************
 * Name: usciB0MstRx 
 * 
 * Description: 
 * 
 * Arguments: nBytes    -   # of bytes to receive
 *            slaveAddr -   slave address
 *            regAddr   -   address of a register to read from
 *            rxBuffer  -   buffer to store received data
 * 
 * Return: 
 * 
 * Author: Iakov Umrikhin
 * Date Created: 23.02.2022 (dd::mm::yyyy)
 * Date Modified: 28.02.2022 (dd::mm::yyyy)
 * ******************************************************************/

signed char usciB0MstRx (unsigned char nBytes, unsigned char slaveAddr, signed char *rxBuffer) {

    volatile signed char result = -1;
    volatile unsigned char rxCounter = 0;

    // Receive mode
    SEND_SA(slaveAddr);
    MST_RX_MODE;

    // START Bit
    START_BIT;
    WAIT_STT;
    // dummy read; to clear the UCRXIFG;
    rxBuffer[0] = UCB0RXBUF;

    while (rxCounter < nBytes && !stopBit) {
        WAIT_RX_IFG;
        rxBuffer[rxCounter] = UCB0RXBUF;
        rxCounter++;

        if (rxCounter == (nBytes - 1)) {
            WAIT_RX_IFG;
            STOP_BIT;
        }
    }

    return result;
}

#pragma vector = USCI_B0_VECTOR
__interrupt void usciB0ISR (void) {

    switch(__even_in_range(UCB0IV, 4)) {
    case 0:
        break;
    case 2:
        break;
    case 4:     // NACK
        UCB0CTL1 |= UCTXSTP;
        stopBit = 1;
        break;
    }
}
