
/***********************************************
 * Header for pixyCam2 camera
 * 
 * 
 * **********************************************/

#include <msp430.h>
#include "usciI2C.h"

#ifndef PIXYCAM2_H
#define PIXYCAM2_H

  // MACROS
    /******************   PixyCam Registers commands ******************/
    #define PIXY_ADDR                       0x54
    #define GET_VERSION_REQ_NUM_BYTES       4  
    #define GET_VERSION_RESP_NUM_BYTES      13
    #define GET_VERSION_VER_NUM_BYTES       7
    #define VECTOR_NUM_BYTES                4

    /************************   Constants   **************************/
    #define PI  3.1415926535
    #define FPS_COORD  16666 //  ~ 60 Hz == 60 FPS

  // global variables

  // funcion's prototypes
    void dispCnsl(signed int computedAngle);

    char pixyGetVersion (unsigned char slvAddr, unsigned char *rxBuffer);
    
    char pixyGetVector (unsigned char *rxBuffer);

    double computePixyAngle (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);

    void dispVersion(long long int version);
#endif
