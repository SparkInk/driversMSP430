
#include <msp430.h> 
#include "usciB1Spi.h"
#include "usciUart.h"
#include <string.h>

/***************************
 * usciB1SPITestClient.c
 *
 *
 * Author: Iakov Umrikhin
 * Date Created: 22.01.2022 (dd/mm/yyyy)
 * Date Modified: 22.01.2022 (dd/mm/yyyy)
 ***************************/
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	

	// variables
    volatile char rxString[BUF_SIZE];
    unsigned volatile int buffer[BUF_SIZE];    // string for TxBuffer
    unsigned volatile int buffLen;  // the length of TxBUFFER
    // initialise UART
    usciA1UartInit();

    // initialise SPI
    usciB1SpiInit(1, 1, 2, 1);

    // enable USCI A1 interrupts
    /*
     * UCA1IE = UCRXIE;
     * */
    UCB1IE = UCRXIE;
    __enable_interrupt();


    // main loop; runs through infinity and beyond
    while (1) {
       rxString[BUF_SIZE] = 0;
       usciA1UartGets(rxString);   // wait for a user to enter a command

       buffLen = bufferLen(rxString) - 1;    // compute the length of rxString

       // convert ASCII into integers
       numStringToInt(rxString, buffer);

       // transmit a string through SPI
       usciB1SpiTXBuffer(buffer, buffLen);
    }
	return 0;
}

