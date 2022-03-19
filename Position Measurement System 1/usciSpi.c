/*
 * usciSpi.c
 *
 * create a proper file header for the C module
 */

#include <msp430.h>
#include "usciB1Spi.h"
#include "nok5110LCD.h"
#include "usciUart.h"

#define nSS P6OUT

unsigned char rxByte;   // byte to transmit to SPI RxBUFF in through ISR
// create a function header that describes the function and how to use it.
// Provide an example function call. (see usciB1Spi.h)

/************************************************************************************
* Function: usciB1SpiInit
* -
* argument:
* Arguments:
*
* return:
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
void usciB1SpiInit(unsigned char spiMST, unsigned int sclkDiv, unsigned char sclkMode, unsigned char spiLoopBack){

// call usciB1SpiClkDiv
    usciB1SpiClkDiv(sclkDiv);

    // **Put state machine in USCI reset while you initialise it**
    UCB1CTL1 |= UCSWRST;


/************************************************************
 * Configure the control registers using the input arguments.
 * See user manual, lecture notes and programmers model
 ************************************************************/

// USCI source clock select
    UCB1CTL1 |= UCSSEL__SMCLK;

// spiMST configuration
    switch(spiMST) {
    case 0:     // slave mode; to receive data
        UCB1CTL0 &= ~UCMST;
        break;
    case 1:     // master mode; to transmit data
        UCB1CTL0 |= UCMST;
        break;
    }
    UCB1CTL0 |= UCMSB;

// nSS configuration
    P6DIR |= BIT0;    //   P6.0 is an output
    nSS |= BIT0 ;     //   high for an inactive state
// sclkMode configuration
    // UCCKPH<7>, UCCKPL<6>
    switch(sclkMode) {
    case 0:
        //  Mode 0: UCCKPH = 0 (captured on falling edge);
        //          UCCKPL = 0 (inactive state - low);
        UCB1CTL0 &= ~UCCKPH & ~UCCKPL;
        break;
    case 1:
        // Mode 1: UCCKPH = 0 (captured on falling edge);
        //         UCCKPL = 1 (inactive state - high);
        UCB1CTL0 &= ~UCCKPH;
        UCB1CTL0 |= UCCKPL;
        break;
    case 2:
        // Mode 2: UCCKPH = 1 (captured on rising edge);
        //         UCCKPL = 0 (inactive state - low);
        UCB1CTL0 |= UCCKPH;
        UCB1CTL0 &= ~UCCKPL;
        break;
    case 3:
        // Mode 3: UCCKPH = 1 (captured on rising edge);
        //         UCCKPL = 1 (inactive state - high);
        UCB1CTL0 = UCCKPH | UCCKPL;
        break;
    }

// sclkLoopBack configuration
    switch(spiLoopBack) {
    case 0:     // Disable loopback mode
        UCB1STAT &= ~UCLISTEN;
        break;
    case 1:     // Enable loopback mode; The transmitter output is internally fed back to the receiver
        UCB1STAT |= UCLISTEN;
    }

// configure the SPI B1 pins with PxSEL register

    UCB1CTL1 &= ~UCSWRST;                       // **Initialise USCI state machine**  take it out of reset
}


// provide function header
// this function is complete. Understand what it is doing.  Call it when SCLKDIV needs to be changed in Lab.
void usciB1SpiClkDiv(unsigned int sclkDiv){

    UCB1CTL1 |= UCSWRST;                        // you always need to put state machine into reset when configuring USC module

    UCB1BR0 = (sclkDiv&0xFF);                   // 2
    UCB1BR1 = (sclkDiv>>8);                     //

    UCB1CTL1 &= ~UCSWRST;                       // **Initialise USCI state machine**
}


// when the TXBUFFER is ready load it.    txByte-->TXBUFFER
// provide a function header
void usciB1SpiPutChar(char txByte) {

    if (UCB1IFG & UCTXIFG) {      // TXBUF is ready to accept another byte
        //rxByte = txByte;
        UCB1TXBUF = txByte;
    }
}

/************************************************************************************
* Function: usciB1SpiTXBuffer
* -
* argument:
* Arguments:
*
* return:
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/

void usciB1SpiTXBuffer (unsigned volatile int *buffer, int buffLen) {
    nSS = 0;    // transmission began
     for (buffLen; buffLen >= 0; buffLen--) {
         usciB1SpiPutChar (buffer[buffLen]);
     }
         nSS = 1;   // transmission ended
}
/************************************************************************************
* Function: numStringToInt
* -
* argument:
* Arguments:
*
* return:
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/

void numStringToInt(char *source, unsigned volatile int *destination) {
    unsigned volatile srcCount = 0;
    unsigned volatile char intermedChar;

    do {
        intermedChar = source[srcCount];
        destination[srcCount] = atoi(&intermedChar);
        srcCount++;

    }while (source[srcCount] != '\r');

}
/************************************************************************************
* Function: bufferLen
* -
* argument:
* Arguments:
*
* return:
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int bufferLen(char *buffer) {

    unsigned volatile int buffCount = 0;
     do {
        buffCount++;
    }while (buffer[buffCount] != '\r');

        return buffCount;
}

#pragma vector=USCI_B1_VECTOR
__interrupt void usciB1SpiIsr(void) {
static unsigned volatile rxBuffCount = 0;
// UCB1IV interrupt handler. __even_in_range will optimize the C code so efficient jumps are implemented.
  switch(__even_in_range(UCB1IV,4)) // this will clear the current highest priority flag. TXIFG or RXIFG.
  {
      case 0: break;                            // Vector 0 - no interrupt
      case 2:                                   // Vector 2 - RXIFG. Highest priority
        // process RXIFG
        if (UCLISTEN) {
            spiRxBuff[rxBuffCount] = UCB1RXBUF;
            //usciB1SpiPutChar(UCB1RXBUF);
            rxBuffCount++;
            if (rxBuffCount > BUF_SIZE) {
                rxBuffCount = 0;
            }
        }
          //UCB1RXBUF = rxByte;
          break;

      case 4:                                       // Vector 4 - TXIFG

        // process TXIFG (careful)
        if (UCLISTEN) {        // loopback connection is ENABLED
            usciB1SpiPutChar(UCB1RXBUF);
        }
          break;

      default: break;
  }
}

