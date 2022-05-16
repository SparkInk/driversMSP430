/*******************************************************
 * This is a header file for a VNH7070AS 
 * evaluation board module
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 20.03.2022
 * Date Modified: 20.03.2022
 *******************************************************/

#ifndef VNH7070API_H
#define VNH7070API_H

    // macros

    // constants
    #define PWM_FRQ 1000
    // global variables

    // functions definition
    char vnh7070InputCtrl(unsigned char ctrl);

    char vnh7070CW(unsigned char dutyCycle);

    char vnh7070CCW(unsigned char dutyCycle);

    char vnh7070Brake(void);
    
#endif
