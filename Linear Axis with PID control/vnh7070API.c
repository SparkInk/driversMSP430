/*******************************************************
 * This is a module for a VNH7070AS evaluation board
 * 
 *
 * MSP430F5529LP     VNH7070AS
 *
 *      P2.0     ->     PWM
 *      P3.0     ->     INa
 *      P3.1     ->     INb
 *      P3.2     ->     SEL
 *      5V       ->     Vcc
 *
 * Author: Iakov Umrikhin
 * 
 * Date Created: 20.03.2022
 * Date Modified: 20.03.2022
 *******************************************************/

#include <msp430.h>
#include "vnh7070API.h"

/*******************************************************
 * Name: vnh7070InputCtrl
 * 
 * Description: Control the state of the INa, INb, SEL bits
 *              Ranges from 0x0 -> 0x7
 * 
 * Arguments: ctrl  -   a variable contating the states of 
 *                      INa, INb, and SEL bits
 * 
 * Return: 0    -   ctrl is within the range of 0x0 - 0x7;
 *        -1    -   otherwise
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 20.03.2022
 * Date Modified: 20.03.2022
 *******************************************************/
char vnh7070InputCtrl(unsigned char ctrl) {

    signed char result;

    if (ctrl > 7) { // out of the range
        result = -1;
    }
    else {  // within the range
		result = 0;
        // P3DIR &= 0xFF & ctrl;
		P3DIR = 0;
		P3DIR = 0xFF & ctrl;
		
        // P3OUT initialisation
		P3OUT = 0;
		P3OUT = 0xFF & ctrl;
    }

    return result;
}

/*******************************************************
 * Name: vnh7070CW
 * 
 * Description: Executes clockwise rotation with a given 
 *              value of the duty cycle
 * 
 * Arguments: dutyCycle   -   the duty cycle for the 
 *                            CW rotation
 * 
 * Return: 0    -   successful
 *        -1    -   unsuccessful
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 20.03.2022
 * Date Modified: 20.03.2022
 *******************************************************/
char vnh7070CW(unsigned char dutyCycle) {

    signed char result = -1;

    if (dutyCycle >= 1 && dutyCycle <= 100) {
        // call vnh7070InputCtrl; for CW with PWM on and CS Hi-Z
        // Sets SEL: CS is High-Z
        result = vnh7070InputCtrl(1);

        // call pwmFunction (use your own)
        setPWM(dutyCycle, PWM_FRQ);
    }
    else {
        result = -1;
    }

    return result;
}

/*******************************************************
 * Name: vnh7070CCW
 * 
 * Description: Executes counter-clockwise rotation
 *              with a given value of the duty cycle
 * 
 * Arguments: dutyCycle   -   the duty cycle for the 
 *                            CCW rotation
 * 
 * Return: 0    -   successful
 *        -1    -   unsuccessful
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 20.03.2022
 * Date Modified: 20.03.2022
 *******************************************************/
char vnh7070CCW(unsigned char dutyCycle) {

    signed char result = -1;
    if (dutyCycle >= 1 && dutyCycle <= 100) {
        // call vnh7070InputCtrl; for CCW with PWM on and CS Hi-Z
        result = vnh7070InputCtrl(6);

        // call pwmFunction (use your own)
        setPWM(dutyCycle, PWM_FRQ);
    }
    else {
        result = -1;
    }

    return result;
}

/*******************************************************
 * Name: vnh7070Brake
 * 
 * Description: Executes a brake signal to VNH707AS
 *              to implement dynamic braking
 * 
 * Arguments: none
 * 
 * Return: 0    -   successful
 *        -1    -   unsuccessful
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 20.03.2022
 * Date Modified: 20.03.2022
 *******************************************************/
char vnh7070Brake(void) {

    signed char result;

    // calls vnh7070InputCtrl
    result = vnh7070InputCtrl(3);

    // pwm set HIGH
    setPWM(10, PWM_FRQ);

    // SEL set LOW
    
    return result;
}
