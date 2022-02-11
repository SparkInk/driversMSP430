/*
 * usciB1Spi.h
 *
 *  Created on: 22 Jan 2022
 *      Author: emine
 */
#include <msp430.h>

#ifndef USCIB1SPI_H_
#define USCIB1SPI_H_

#define BUF_SIZE 50

// global variable

    unsigned volatile char spiRxBuff [BUF_SIZE];
// functions' prototypes
    /***************************************
     * EXAMPLE for usciB1SpiInit
     *
     *  usciB1SpiInit(1, 16, 3, 0);
     *
     *  Meaning: Master, (f_clk/16), (UCCKPH = 1, UCCKPL = 1), UCLISTEN = 0;
     ***************************************/
    void usciB1SpiInit(unsigned char spiMST, unsigned int sclkDiv, unsigned char sclkMode, unsigned char spiLoopBack);

    /***************************************
     * EXAMPLE for usciB1SpiClkDiv
     *
     *  usciB1SpiClkDiv(16);
     *  Meaning: f_clk / 16
     ***************************************/
    void usciB1SpiClkDiv(unsigned int sclkDiv);

    /***************************************
     * EXAMPLE for usciB1SpiPutChar
     *
     *  usciB1SpiPutChar('0xAA');
     *  Meaning: Transmits '0xAA' onto console
     ***************************************/
    void usciB1SpiPutChar(unsigned char txByte);
    /***************************************
         * EXAMPLE for usciB1SpiTXBuffer
         *
         *  usciB1SpiTXBuffer
         *  Meaning:
         ***************************************/
    void usciB1SpiTXBuffer (unsigned volatile int *buffer, int buffLen);
    /***************************************
         * EXAMPLE for numStringToInt
         *
         *  numStringToInt(
         *  Meaning:
         ***************************************/
    void numStringToInt(char *source, unsigned volatile int *destination);
    /***************************************
         * EXAMPLE for bufferLen
         *
         *  bufferLen();
         *  Meaning:
         ***************************************/
    int bufferLen(char *buffer);
#endif /* USCIB1SPI_H_ */
