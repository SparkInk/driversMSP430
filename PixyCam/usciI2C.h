
/***********************************************
 * Header for I2C protocol
 * 
 * 
 * **********************************************/

#include <msp430.h>

#ifndef USCII2C_H
#define USCII2C_H

    // constants
    #define SMCLK_DIV_10 10     // SMCLK deviser by 10
    /******************   MPU6050 Registers ******************/
    // MPU6050's address
    #define MPU6050_ADDR 0x68
    //      X-axis
    #define GYRO_XOUT_H     0x43
    #define GYRO_XOUT_L     0x44
    // Register for the slave's ID, i.e. it's register where a slave's address is stored
    #define WHO_AM_I        0x75


    // MACROS
   
    #define SEND_SA(slaveAddr)   UCB0I2CSA = slaveAddr

    #define I2C_RST_ON          UCB0CTL1 |= UCSWRST;
    #define I2C_RST_OFF         UCB0CTL1 &= ~UCSWRST;

    #define MST_TX_MODE         UCB0CTL1 |= UCTR     // Reset is ON; Transmitter Mode; Reset is OFF
    #define MST_RX_MODE         UCB0CTL1 &= ~UCTR   // Reset is ON; Receiver Mode; Reset is OFF
    #define START_BIT           UCB0CTL1 |= UCTXSTT    // Transmit START bit
    #define STOP_BIT            UCB0CTL1 |= UCTXSTP    // Transmit STOP bit
    #define WAIT_STT            while (UCB0CTL1 & UCTXSTT)  // Wait for START bit to go low -> ACK is received
    #define WAIT_TX_IFG         while (!(UCB0IFG & UCTXIFG))  // Wait for TXBUFF to be empty; when it is empty TXIFG == 1;
    #define WAIT_RX_IFG         while (!(UCB0IFG & UCRXIFG))   // Wait RXBUFF to receive a character; when receive a character RXIFG == 1;

    // global variables

    // functions' prototypes

    void usciB0I2CInit (unsigned int sclkDiv);

    signed char usciB0MstTx (signed char nBytes, char slaveAddr, char *txBuffer);

    signed char usciB0MstRx (unsigned char nBytes, unsigned char slaveAddr, signed char *rxBuffer);

#endif
