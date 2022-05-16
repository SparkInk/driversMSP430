/*
 * posCtrl.h
 *
 *
 *  Author: Iakov (aka Iasha) Umrikhin
 *  Date Created: 09.05.2022 (dd.mm.yyyy)
 *  Date Modified: 09.05.2022 (dd.mm.yyyy)
 */

#ifndef POSCTRL_H_
#define POSCTRL_H_

    // arrow keys structure
    typedef struct ARROWS {

        volatile unsigned char up, down, left, right, bitPID;

    } ARROWS;

    // PID gains 
    typedef struct PID {
        volatile float Kp, Ki, Kd;
    } PID;

    // global variables 
    ARROWS arrowKey;

    volatile unsigned char executeCmdUpdate;
    volatile unsigned char enterConsole;
    volatile unsigned char posCountUpdate;

    // functions definitions    
    unsigned int posCtrl(signed long int posCount, signed long int setPoint, PID *pidGain, signed char *motorDirection);

    void changePID(PID *pid);

    void clearArrowKeys(ARROWS *arrow);
    
    void detectArrowKey(ARROWS *arrow);

    signed int constrain(signed int modifiedVar, signed int lowBoundary, signed int highBoundary);

#endif /* POSCTRL_H_ */
