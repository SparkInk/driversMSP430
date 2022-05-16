/*****
 * TA0.1 (P1.2) ->  TA0.CCI1A -> TA0CCTL1;
 * 
 * 
 * 
 * */


#include <msp430.h>
#include "usciUart.h"
#include "timer.h"


/******************************************************************
 * Name: timerA0Init    
 * 
 * Description:
 * 
 * 
 * Arguments:
 * 
 * Return: none
 * 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 06.03.2022 (dd:mm:yyyy)
 * Date Modified: 06.03.2022 (dd:mm:yyyy)
 ******************************************************************/ 

void timerA0Init() {

    TA0CTL |= TASSEL_2 | ID_0 | MC_1 | TACLR;   // SMCLK; Input divider 0; Up mode

    // TA0CCTL1 initialisation
    TA0CCR0 = 100001;   // update every 100 ms ~ 10 kHz
    
    // enable interrupt on CCR1
    TA0CCTL0 |= CCIE;

    // clear all pending interrupt flags
    TA0CCTL0 &= ~CCIFG;
    TA0CTL &= ~TAIFG;

}

/******************************************************************
 * Name: timerA1Init    
 * 
 * Description:
 * 
 * 
 * Arguments:
 * 
 * Return: none
 * 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 06.03.2022 (dd:mm:yyyy)
 * Date Modified: 06.03.2022 (dd:mm:yyyy)
 ******************************************************************/ 

/*void timerA1Init() {
//    TA1CTL |= TASSEL_2 | MC_1 |  TACLR;
//    // update: 100 Hz
//    TA1CCR0 = DISP_UPDATE_RATE - 1; // the main channel
    TA1CCTL0 |= CCIE;

    // clear interrupt flags
    TA1CTL &= ~TAIFG;


}*/
/************************************************************************************
* Function: fwDirection
* Description: 
*
* Arguments: encState   -    structure containing values of CHA and CHB
*
* return: 0 -> ccw; 1 -> cw;
* Author: Iakov Umrikhin
* Date Created: 06.02.2022 (dd:mm:yyyy)
* Date Modified: 06.03.2022 (dd::mm::yyyy)
************************************************************************************/
unsigned char fwDirection(ENC_STATE encState) {

    volatile unsigned char direction;

    direction = encState.chAState ^ encState.chBState; // 0

    return direction;
}
