/*************************************************************************************************
 * ucsiUart.h
 * - - C interface file for MSP430 UCSI UART A1, A0
 *
 *  Author: Greg Scutt
 *  Created on: March 1, 2017
 *  Modified: Feb 19, 2018
 **************************************************************************************************/

#ifndef USCIUART_H_
#define USCIUART_H_

    #define BUF_SIZE 100


// global variables
    char rxBuffer[BUF_SIZE];
    volatile signed long int rxString[50];  // store commands from the terminal

// functions
    void usciA1UartInit();

    void usciA1UartTxChar(signed long int txChar);

    void usciA1UartTxString(unsigned char *txChar);

    char *usciA1UartGets(char *rxString);

#endif /* USCIUART_H_ */
