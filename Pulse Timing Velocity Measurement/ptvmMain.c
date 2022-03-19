

/*******************************************************************
 * 									ptvmMain.c
 * This system inmpelents Pulse Timing Velocity Measurement 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 06.03.2022 (dd:mm:yyyy)
 * Date Modified: 06.03.2022 (dd:mm:yyyy)
 * 
 * System specifications:
 * Maximum error at maximum velocity measurement, Emax <= 10%
 * Display speed at a rate of  60 Hz; different timer 
 ******************************************************************/

#include <msp430.h> 
#include "timerA0.h"

// global variables


ENC_STATE encState = {0, 0};

volatile unsigned int countDiff;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    volatile char fwVelocityStr[15] = "FW Velocity: \n";
    volatile char rpmStr[6] = " rpm\n";
    volatile char cwStr[7] = " CW \n";
    volatile char ccwStr[7] = " CCW \n";
    volatile unsigned int speed;

	// initialise UART
	usciA1UartInit();

    // ports initialisation; P1.2: Timer_A0 CCI1A;
    P1DIR = 0xFF & ~(BIT2 | BIT3);    // only P1.2 is 0; others 1
    P1SEL |= BIT2;          // select TA0.CCI1A
    //  P1.3: channel B input
    P1REN |= BIT3;  // pull down resistor

	// initialise timers
    timerA0Init();
    timerA1Init();

	// enable the global interrupt
	__enable_interrupt();

	while(1) {
		// live forever

	    if (displayRPM) {

	        speed = (6522) / (countDiff);

	        displayRPM = 0;
	        usciA1UartTxChar('\r');
//	        usciA1UartTxChar('\n');
	        usciA1UartTxString(fwVelocityStr);
	        dispCnsl(speed);
	        usciA1UartTxString(rpmStr);

	        // display the direction
	        if (fwDirection(encState)) {  // cw
	            usciA1UartTxString(cwStr);
	        }
	        else {  // ccw
	            usciA1UartTxString(ccwStr);

	        }
	    }

	}
	return 0;
}

/********************************************
 *		 		TimerA0 ISR          		*
 * 											*
 * Updates the speed at 60 Hz	 			*
 *******************************************/

#pragma vector = TIMER1_A0_VECTOR
__interrupt void timerA0ISR(void) {


	displayRPM = 1;

    // clear interrupt flags
    TA1CTL &= ~TAIFG;
}
/************************************************
 *		 		TimerA1 ISR          			*
 * 												*
 * Used to compute the speed of the flywheel    *
 * with the capture mode						*
 ***********************************************/

#pragma vector = TIMER0_A1_VECTOR
__interrupt void timerA1ISR(void) {

    static volatile unsigned int oldCounts = 0;
    volatile unsigned int newCounts;

	//disable interrupt
	TA0CCTL1 &= ~CCIE;

	// record values of CHA and CHB
    encState.chAState = 1;
    encState.chBState = (P1IN & 0xFF) >> 3;     // shift the P1.3 bit by 3

    newCounts = TA0R;

    // record the current counts on TA0R
    countDiff = newCounts- oldCounts;

    oldCounts = newCounts;

	// clear interrupt flag; in case, the vector did not work
	TA0CCTL1 &= ~CCIFG;

	// enable interrupt
	TA0CCTL1 |= CCIE;
}

/*
Pseudo code
(global) oldCounts = 0; decleared outside of ISR
within ISR:
	desable interrupts;
	(static) int newCounts = TAR;
	diff = newCounts - oldCounts;
	Speed =  (2 *pi) / (pulses) * 1 / (diff);
	direction = Direction();
	print("FW velocity: ", speed, direction);
	oldCounts = TAR;
	enable interrupts;
*/
