/*************************************************************************************************
 * main_lab.c
 * - top level file with main{} for UART lab3
 *
 *  Author: Iakov Umrikhin
 *  Created on: March 1, 2018

 **************************************************************************************************/

#include <msp430.h> 
#include "usciUart.h"
/*
 * main_lab.c
 */


#define 	_UART_A1PSEL 	P4SEL |= PM_UCA1TXD | PM_UCA1RXD // use macros for short expressions.

// global variables
char rxBuffer[BUF_SIZE];

int main(void) {
	volatile int i = 0;
	const char testString1[20] = "BCIT MECHATRONICS\r\n";
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    char rxString[50];
    CMD cmdList[MAX_CMDS];
    int cmdIndex;


    usciA1UartInit();
    initCmdList(cmdList);

    // interrupts
   //UCA1IE |= UCRXIE;
   //__enable_interrupt();


    /*while (1){

       //usciA1UartTxChar(0x7f);
        //usciA1UartTxString (testString1);
        if (!usciA1UartGets(rxString)) {
            UCA1TXBUF = -1;
        }

    }*/
   while (1) { // this loop is for the command interpreter only

       usciA1UartGets(rxString);    // wait for the user to type in a command
       cmdIndex = parseCmd(cmdList, rxString); // self-explanatory
       executeCmd(cmdList, cmdIndex);

   }
	return 0;
}
