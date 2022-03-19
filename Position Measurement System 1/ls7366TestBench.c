
/**
 * ls7366TestBech.c
 *
 * main file
 *
 * Author: Iakov Umrikhin
 * Date Created: 07.02.2022 (dd:mm:yyyy)
 */

#include <msp430.h>
#include "LS7366R.h"
#include "nok5110LCD.h"
#include "usciB1Spi.h"
#include "usciUart.h"
#include "angleDisp.h"

#define HALF_DELAY  100000
#define FRQ_REF_RATE_LCD  250000  
#define FRQ_REF_RATE_POS  100000

#define SHIFT_8 256
#define SHIFT_16 65536
#define SHIFT_24 16777216

/********************************************         
 *              GLOBAL VARIABLES
 * *****************************************/
unsigned char dispMode;     // display mode: 0 -> single line mode; 1 -> horizontal bar mode
unsigned char lcdUpdate;    // update LCD bit
unsigned char posCountUpdate;   // update posCount bit
unsigned char executeCmdUpdate;     // update bit for executeCmd
volatile signed long int rxString[50];  // store commands from the terminal
volatile signed int angleOffset;
/******************************************
 *          MAIN MODULE FUNCTIONS
 * ***************************************/
void displayCounts(signed long int posCounts);

/**************************************************
	N.B.:
		When entering a command, press any key (preferably ENTER)
		There is a quick note below on why it is the way it is
	
*/
int main(void)
    {
    WDTCTL = WDTPW | WDTHOLD;  // stop watchdog timer

    //volatile unsigned char counts [50];
    volatile unsigned char counts[4];
    volatile signed long int posCounts = 0;
    volatile signed long int oldCounts = 0;
    CMD cmdList[MAX_CMDS];
    volatile int cmdIndex;
    volatile long int angleCounter;
    volatile unsigned char i;
    //volatile signed long int rxString[50];  // store commands from the terminal
    dispMode = 0;

    // setting up pins
    P6DIR |= BIT1 | BIT2 | BIT3 | BIT4;   // P6.1 -> Reset pin; P6.2 -> Power pin; P6.3 -> nSCE; P6.4 -> DnC;
    P7DIR |= BIT0;      //  P7.0 -> nSS (ls7366)0
    P6OUT = 0;  // set P6<5:1> to zero
    _NOK_LCD_PWR_OFF;   // turns off LCD

    P4SEL |= BIT1 | BIT2 | BIT3;      // Port 4.1 [SIMO], 4.2 [SOMI], 4.3 [CLK I/O]
    nSCE |= SCE;   // disable SCE

    // Initialise UART
    usciA1UartInit();
    //initCmdList(cmdList);   // initialise a list of commands

    // Initialise SPI
    usciB1SpiInit(1, 1, 2, 0);  // SPI initialisation

    //UCB1IE = UCRXIE;


    UCA1IFG &= ~UCRXIFG;    // clear interrupt flags
    UCA1IE |= UCRXIE;   // enable uartRx interrupt

    // initialise timers
    timerA0Init();

    __enable_interrupt();

    // initialise the LCD
    nokLcdInit();
    initCmdList(cmdList);

    // initialilse LS7366R
    LS7366Rinit();
    memset(counts, 0, 4);   // empty counts array before loading it

    // main loop
    LS7366Rclear(CNTR);
    while(1) {

       /*********************************************************************************
        *           QUICK NOTE on usciA1UartGets
        *   When using the terminal to enter a command,
        *   please press enter (or any key you want) to initialise the process
        *   The reason for it: When you press a key for the first time
        *                      usciA1UartGetsISR is activated, thus this key press
        *                      acts as a pass to enter IF statement for executeCmdUpdate
        *
        *********************************************************************************/
       if (executeCmdUpdate) {
            memset(rxString, 0, BUF_SIZE);
            usciA1UartGets(rxString);
            cmdIndex = parseCmd(cmdList, rxString);
            executeCmd(cmdList, cmdIndex);

            executeCmdUpdate = 0;
            UCA1IE |= UCRXIE;  // disable interrupts on uartRx
            TA0CCTL0 |= CCIE;   // enable interrupts on CCR0
            TA0CCTL1 |= CCIE;   // enalbe interrupts on CCR1
        }

        if (posCountUpdate) {  // update posCount
            LS7366Rread(CNTR, counts);
            posCounts = (counts[0] << 24) |
                        (counts[1] << 16) |
                        (counts[2] << 8)  |
                         counts[3];  // casting

//            posCounts /= 4;       // divide by 4 to ignore quadrature amplification

            posCountUpdate = 0;
        }
        // for the line mode
        if ((abs((posCounts - oldCounts)) > UPDATE_RATE_LINE) && (!dispMode) && lcdUpdate) {   // more than 2 degrees; for the line mode
            dispAngleLCDline(posCounts);
            oldCounts = posCounts;  // save the current position to use for comparison in the next run
            lcdUpdate = 0;
        }
        // for the bar mode
        if ((abs((posCounts - oldCounts)) > UPDATE_RATE_BAR) && dispMode && lcdUpdate) {
            dispAngleLCDbar(posCounts);
            oldCounts = posCounts;  // save the current position to use for comparison in the next run
            lcdUpdate = 0;
        }

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

    TA0CCR0 = FRQ_REF_RATE_LCD - 1;    // 4 Hz (250 ms)
    TA0CCR1 = FRQ_REF_RATE_POS;        // 10 Hz (100 ms)
    TA0CCR2 = 2 *FRQ_REF_RATE_POS;      // 20 Hz (200 ms)
    TA0CCTL0 |= CCIE;   // enable interrupts on CCR0
    TA0CCTL1 |= CCIE;   // enalbe interrupts on CCR1

    TA0CTL &= ~TAIFG; // clear interrupt flags; if the first one does not work

}

/***************************************************
 * ISR for the CCR0 of TimerA0
 * ************************************************/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timer0A0ISR (void) {

    // update LCD
    lcdUpdate = 1;

    //TA0CTL &= ~TAIFG;   //automatically clears interrupt flags;
}
/***************************************************
 * ISR for the CCR1 of TimerA0
 * ************************************************/
#pragma vector = TIMER0_A1_VECTOR
__interrupt void timer0A1ISR (void) {

    switch(__even_in_range(TA0IV, 4)) {
        case 0: break;
        case 2:     // Vector 2: CCIFG1
            // update posCounts
            posCountUpdate = 1;
        case 4:
            posCountUpdate = 1;
            break;
    }

    //TA0CTL &= ~TAIFG;   //automatically clears interrupt flags;
}
/***************************************************
 * ISR for the UART_A1_VECTOR
 * ************************************************/
#pragma vector = USCI_A1_VECTOR
__interrupt void usciA1UartGetsISR(void) {
    TA0CCTL0 &= ~CCIE;   // dinable interrupts on CCR0
    TA0CCTL1 &= ~CCIE;   // dinalbe interrupts on CCR1
    UCA1IE &= ~UCRXIE;  // disable interrupts on uartRx

    // store the command
    executeCmdUpdate = 1;

    UCA1IFG &= ~UCRXIFG;    // clear the interrupt flags
}
/*********************************************************
 * Author: Iakov Umrikhin
 * nokLcdDraw (4.2.2)
 * Purpose: to update the posCounts variable and LCD display
 * posCounts: updated every 100 ms (100 000 cycles)
 * LCD: updates every 250 ms (250 000 cycles) <- CCR0
 * Line mode: update every 2 degrees => 26 coutns
 * Bar mode: update every 55 counts;
 * 
 * Pseudo-code:
 * // global variable
 * unsigned char posCountUpdate = 0;
 * unsigned char lcdUpdate = 0;
 * 
 * int main () {
*       oldCounts = 0;
* 
*       initTimerA0();
*       __enable_interrupts();
*       while(1) {
             if (posCountUpdate) {  // update posCount
                LS7366Rread(CNTR, counts);
                posCounts = (signed long int) counts;  // casting 
                posCounts /= 4;       // devide by 4 to ignore quadrature amplification
                posCountUpdate = 0;
                __enable_interrutps();
             }
*            if (abs((posCounts - oldCounts)) > UPDATE_RATE_LINE && (!dispMode) && lcdUpdate) {   // more than 2 degrees; for the line mode
*                dispAngleLCDline(posCounts);
*                oldCounts = posCounts;
                 lcdUpdate = 0;
*            }
            if (abs((posCounts - oldCounts)) > UPDATE_RATE_BAR && dispMode && lcdUpdate) {
                dispAngleLCDbar(posCounts);
                oldCounts = posCounts;
                lcdUpdate = 0;
            }
*       }
 *  }
 *
 * within ISR:
 *      __disable_interrupts();
 *      if (timerA0Channel1 == 100 ms) {    // time to update posCount
 *              posCountUpdate = 1;
 *          }
 *      }
 *      else if (timerA0MainChannel == 250 ms) {    // time to update the display
 *          lcdUpdate = 1;
 *      }
 *      __clear_interrupt_flag();
 * 
 * 
 * -------------------- REQUIRED FUNCTIONS ------------------
 * LS7366Rread (unsigned char reg, unsigned char *dataIn)
 * 
 * dispAngleLCDline (signed long int posCounts)
 * 
 * dispAngleLCDbar (signed long int posCounts)
 * 
 * initTimerA0();
 * 
 * ******************************************************/
