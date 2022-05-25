
/*************************************************
 * The source file for the PWM generator 
 * with the timer A1
 * 
 * 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 13.03.2022
 * Date Modified: 13.03.2022
 * ***********************************************/

#include <msp430.h>
#include "pwm.h"

/****************************************************************
 * Name: initPWM
 * 
 * Description:  initPWM initialises the timer A1 which is used to
 *              to control and configure PWM signal
 * 
 * Arguments: none
 *  
 * Return: none
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 13.03.2022
 * Date Modified: 13.03.2022
 *****************************************************************/
void initPWM (void) {

    TA1CTL |= TASSEL_2 | MC_1;  // SMCLK; Up-mode
    TA1CCTL1 |= OUTMOD_7;

    // Initialise P2.0; TA1.1
    P2SEL |= BIT0;
    P2DIR |= BIT0;
}

/****************************************************************
 * Name: setPWM
 * 
 * Description: setPWM configures the PWM's duty cycle
 *                    and frequency;
 *             Alas:
 *             The frequency should be entered in the
 *             form shown on the left: 10000 == 10 kHz;
 *                                     2234 == 2.234 kHz;
 *             The duty cycle is entered 
 *             as shown on the left: 50 == 50 % or 73 == 73%;
 *             Fail to conform will blow up your PC 
 *             
 * Arguments: dutyCycle     -   the duty cycle of the PWM signal;
 *            frequency     -   the frequency of the PWM signal;
 * 
 * Return: Nothing 
 * 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 13.03.2022 (dd:mm:yyyy)
 * Date Modified: 13.03.2022 (dd:mm:yyyy)
 ***************************************************************/
void setPWM(unsigned char dutyCycle, unsigned int frequency) {

    TA1CCR0 = (MCU_FRQ / frequency) + 1;    // The period of the main clock is (1 MHz / (frequency)) - 1
    TA1CCR1 = (TA1CCR0) * (double)dutyCycle * 0.01;  // The duty cycle is a percentage of the main timer's period

}
