

/**************************************************************************
 *                             mpu6050TestBench.c
 *
 * This test bench is for MPU6050 sensor.
 *
 * If the test is successful, then this bench will be used to test PixyCam
 *
 **************************************************************************/

#include <msp430.h>
#include "usciI2C.h"
#include "pixyCam2.h"
#include "pixyCamCmds.h"

int main(void)

{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	unsigned char verStr[GET_VERSION_VER_NUM_BYTES];
	unsigned char vectorStr[VECTOR_NUM_BYTES];
	volatile unsigned char rxString[50];  // store commands from the terminal

	PIXY_CMD cmdList[MAX_PIXY_CMDS];
	int cmdIndex;

	usciB0I2CInit(SMCLK_DIV_10);

	// initialise UART
	usciA1UartInit();
	// initialise CMD list
	initCmdList(cmdList);

	timerA0Init();

	UCB0IFG &= ~UCNACKIFG;
	UCB0IE |= UCNACKIE;
	__enable_interrupt();


	while (1) {
	    usciA1UartGets(rxString);
        cmdIndex = parseCmd(cmdList, rxString);
        executeCmd(cmdList, cmdIndex);
	}

	return 0;
}

/***************************************************
 * timerAInit()
 *
 *
 *
 *
 *
 **************************************************/
void timerA0Init(void) {
    TA0CTL |= TASSEL_2 | ID_0 | MC_1; // TASSEL_2 <- source select (SMCLK); ID_0 <- devider(1); MC_1 <- mode (up mode);

    TA0CCR0 = FPS_COORD - 1;    // 60 Hz (16.6 ms)
    TA0CCTL0 |= CCIE;   // enable interrupts on CCR0

    TA0CTL &= ~TAIFG; // clear interrupt flags; if the first one does not work

}


