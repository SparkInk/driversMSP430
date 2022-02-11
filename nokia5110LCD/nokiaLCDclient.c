
#include <msp430.h> 
#include "nok5110LCD.h"
#include "usciB1Spi.h"
#include "usciUart.h"
/**
 * nokiaLCDclient.c
 * main file
 */

int main(void)
{
    char rxString[50];
    CMD cmdList[MAX_CMDS];
    int cmdIndex;

     WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    // setting up pins
    P6DIR |= BIT1 | BIT2;   // P6.1 -> Reset pin; P6.2 -> Power pin
    _NOK_LCD_PWR_OFF;   // turns off LCD

	P4DIR |= BIT0 | BIT2;		// P4.0 is output [nSCE]; P4.2 is output [DnC]
    P4SEL |= BIT1 | BIT3;      // Port 4.1 [SIMO], 4.2 [SOMI], 4.3 [CLK I/O]
    nSCE |= SCE;   // disable SCE

    // Initialise UART
    usciA1UartInit();
    initCmdList(cmdList);   // initialise a list of commands
    // Initialise SPI
	usciB1SpiInit(1, 1, 2, 0);	// SPI initialisation

	UCB1IE = UCRXIE;
	__enable_interrupt();

    nokLcdInit();

	while (1) {
	    memset(rxString, 0, BUF_SIZE);
	    // wait for the user to enter the command
	    usciA1UartGets(rxString);
	    cmdIndex = parseCmd(cmdList, rxString);
	    executeCmd(cmdList, cmdIndex);
	}

	return 0;
}
