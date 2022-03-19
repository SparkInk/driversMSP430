/*****
 * TA0.1 (P1.2) ->  TA0.CCI1A -> TA0CCTL1;
 * 
 * 
 * 
 * */


#include <msp430.h>
#include "usciUart.h"
#include "timerA0.h"



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
    // for now we use ID_0; needs to be changed in the lab to save power
    TA0CTL |= TASSEL_2 | ID_1 | MC_2 | TACLR;   // SMCLK; Input divider 1; Continuous mode

    // TA0CCTL1 initialisation
    TA0CCTL1 |= CM_1 | CCIS_0 | SCS | CAP;
    TA0EX0 |= TAIDEX_7;
    // enable interrupt on CCR1
    TA0CCTL1 |= CCIE;

//    TA0EX0 |= TAIDEX_7; // divide by 8
    // clear all pending interrupt flags
    TA0CCTL1 &= ~CCIFG;
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

void timerA1Init() {
    TA1CTL |= TASSEL_2 | MC_1 |  TACLR;
    // update: 100 Hz
    TA1CCR0 = DISP_UPDATE_RATE - 1; // the main channel
    TA1CCTL0 |= CCIE;

    // clear interrupt flags
    TA1CTL &= ~TAIFG;


}
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

/************************************************************************************
* Function: dispCnsl
* Description: displays computedAngle on a console;
*              Serves as a replacement to sprintf(), because the latter refuses to
*              fulfill its duties :(
*
* Arguments: computedAngle  -   a number of counts
*
* return: none
* Author: Iakov Umrikhin
* Date Created: 06.02.2022 (dd:mm:yyyy)
* Date Modified: 06.03.2022 (dd::mm::yyyy)
************************************************************************************/
void dispCnsl(unsigned int computedSpeed) {
    char buffer[8] = 0;
    unsigned char i = 0;
    unsigned char j = 0;

    if (computedSpeed >= 0) {    // positive counts
        while (computedSpeed > 0) {
            volatile int mod;
            mod = computedSpeed % 10;
            buffer[7-j] = mod;    //0th element equals to the last digit of a number
            //usciA1UartTxChar('0' + mod);    // shift a digit by 40 in ASCII table
            computedSpeed /= 10;
            j++;
        }
        for (i = (8-j); i < 8; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
//        usciA1UartTxChar('\r');
//        usciA1UartTxChar('\n');
    }
    if (computedSpeed  < 0) {   // negative counts
        usciA1UartTxChar('-');  // display a negative sign
        computedSpeed *= -1;    // convert from negative to positive
        while (computedSpeed > 0) {
            volatile int mod;
            mod = computedSpeed % 10;   // chunk off the last digit
            buffer[7-j] = mod;    // 0th element equals to the last digit of a number
            //usciA1UartTxChar(mod + '0');
            computedSpeed /= 10;      // remove the last digit from the current number
            j++;
        }
        for (i = (8-j); i < 3; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
//        usciA1UartTxChar('\r');
//        usciA1UartTxChar('\n');
    }

    memset(buffer, 0, 8);   // clear buffer from previous data
}

