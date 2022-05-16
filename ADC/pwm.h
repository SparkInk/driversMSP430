/**************************************
 * The header file for PWM signal
 * 
 * Author: Iakov Umrikhin 
 * 
 * Date Created: 13.03.2022 (dd:mm:yyyy)
 * Date Modified: 13.03.2022 (dd:mm:yyyy)
 * ***********************************/

#ifndef PWM_H
#define PWM_H

    // constants
    #define MCU_FRQ     1048000
    // macros

    // global variables

    // functions definitions

    void initPWM(void);

    void setPWM(unsigned int dutyCycle, unsigned int frequency);

#endif


