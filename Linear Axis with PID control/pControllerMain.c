

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
#include "incSensor.h"
#include "adc12.h"
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
	volatile signed long int angleAvg;

    // structures 
    CMD cmdList[MAX_CMDS];
    PID pidGains;

    /**************************************/

    P4SEL |= BIT1 | BIT2 | BIT3;      // Port 4.1 [SIMO], 4.2 [SOMI], 4.3 [CLK I/O]
    P7DIR |= BIT0;      //  P7.0 -> nSS (ls7366)0

    // initialise UART
    usciA1UartInit();

    // initialise TIMER1_A0
    timerA0Init();

    // initialise ADC12
    adc12Cfg("2V5", 1, 1, 0);

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

            posCounts = (counts[0] << 24)  |
                        (counts[1] << 16)  |
                        (counts[2] << 8)   |
                        (counts[3]);

            // compute the average angle from the accelerometer
            // and convert it to counts
            angleAvg = angleAvgCompute(mem0Result, mem1Result) * 87;
            // for safety reasons constrain the angleAvg output (in counts)
     //       angleAvg = constrainl(angleAvg, -12000, 12000);

            // mtrDir -> activate;
            if (abs(angleAvg - posCounts) > 25){  // updates the duty cycle until the difference between
                                                  // angleAvg[count] and posCounts[count] is 13 counts or less

                // based on the angle received update the PWM 
                mtrDirUpdate(motorDirection, posCtrl(posCounts, angleAvg, &pidGains, &motorDirection));    

                sprintf(rxString, "Set point: %ld    PID: %d   Error: %ld   \r\n",
                                    angleAvg, 
                                            posCtrl(posCounts, angleAvg, &pidGains, &motorDirection), 
                                                                (angleAvg - posCounts));
                usciA1UartTxString(rxString);
            }
            else {
                sprintf(rxString, "Brake is set. Final Error: %ld               \r\n", 
                                                        (angleAvg - posCounts));
                usciA1UartTxString(rxString);
                executeCmd(cmdList, 2);
            }
            memset(rxString, 0, BUF_SIZE);
            posCountUpdate = 0;

            // enable interrupts
            UCA1IE |= UCRXIE;

        }

        // set a new setPoint; is not used in this program
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


/***************************************************************
    
    Wheel's diameter: 2.726 in => 6.924 cm;
    Motor's CPR => 200
    Motor's gear ratio => 19.658
    Quadrature mode is on the encoder;
    # of counts: 200 counts * 19.658 * 4 => 15726 counts / rev
    or 43.7 counts / 1 deg;


 * ************************************************************/
