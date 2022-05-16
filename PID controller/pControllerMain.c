

                            /************************************************************
                             *
                             *                      PID-controller main file
                             *
                             * Author: Iakov (aka Iasha) Umrikhin
                             *
                             * Date Created: 09.05.2022 (dd.mm.yyyy)
                             * Date Modified: 09.05.2022 (dd.mm.yyyy)
                             ***********************************************************/

#include <msp430.h>.
#include <stdio.h>
#include "LS7366R.h"
#include "positionControl.h"
#include "usciUart.h"
#include "pwm.h"
#include "usciB1Spi.h"
#include "commands.h"

// global variables
//volatile static signed int setAngleGlobal;

//                                                                  MAIN STARTS HERE

int main(void) {

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    /***************************************/
    /*              VARIABLES              */
	volatile unsigned char counts[4];
    volatile int cmdIndex;
    volatile signed long int rxString[50];  // store commands from the terminal
    volatile signed long int posCounts = 0; 
    volatile unsigned char motorDirection;
    CMD cmdList[MAX_CMDS];
    PID pidGains;
    /**************************************/

    P4SEL |= BIT1 | BIT2 | BIT3;      // Port 4.1 [SIMO], 4.2 [SOMI], 4.3 [CLK I/O]
    P7DIR |= BIT0;      //  P7.0 -> nSS (ls7366)0

    // initialise UART
    usciA1UartInit();

    // initialise TIMER1_A0
    timerA0Init();

    // initialise TIMER1_A0; don't know what currently to use it for

    // initialise SPI
    usciB1SpiInit(1, 1, 2, 0);  // SPI initialisation

    // initialise FEDI
    LS7366Rinit();
    memset(counts, 0, 4);   // empty counts array before loading it
    LS7366Rclear(CNTR);     // clear the FEDI counter

    // initialise PWM
    initPWM();

    // initialise CMD list 
    initCmdList(cmdList);

    // enable UART interrupt
    UCA1IE |= UCRXIE;

    // enable interrupts 
    __enable_interrupt();

    while (1) {

        if (posCountUpdate) {  // update posCount

            // read counts from FEDI
            LS7366Rread(CNTR, counts);
            posCounts = (counts[0] << 24) |
                        (counts[1] << 16) |
                        (counts[2] << 8)  |
                         counts[3];
//            posCounts = constrain(posCounts, -4600, 4600);

            // mtrDir -> activate
            if (abs(setPointGlobal - posCounts) > 13){
                mtrDirUpdate(motorDirection, posCtrl(posCounts, setPointGlobal, &pidGains, &motorDirection));    
                sprintf(rxString, "Set point: %ld    PID: %d   Error: %ld   \r\n", setPointGlobal, posCtrl(posCounts, setPointGlobal, &pidGains, &motorDirection), (setPointGlobal - posCounts));
                usciA1UartTxString(rxString);
            }
            else {
                sprintf(rxString, "Break is set. Final Error: %ld           \r\n", (setPointGlobal - posCounts));
                usciA1UartTxString(rxString);
//                executeCmd(cmdList, 2);
            }
            memset(rxString, 0, BUF_SIZE);
            posCountUpdate = 0;

            // enable interrupts
            UCA1IE |= UCRXIE;

        }

        // set a new setPoint
        if (executeCmdUpdate) {
            
            usciA1UartTxString("Enter a commands \n");
            newLine();

            memset(rxString, 0, BUF_SIZE);
            usciA1UartGets(rxString);
            cmdIndex = parseCmd(cmdList, rxString);
            executeCmd(cmdList, cmdIndex);  
            
            executeCmdUpdate = 0;

            // enable interrupts
            UCA1IE |= UCRXIE;
        }
        
    }
    
	return 0;
}
//                                                                  MAIN ENDS HERE



/***************************************************
 * ISR for the CCR0 of TimerA0
 * ************************************************/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timer0A0ISR (void) {

    // update counts
    posCountUpdate = 1;

    //TA0CTL &= ~TAIFG;   //automatically clears interrupt flags;
}
