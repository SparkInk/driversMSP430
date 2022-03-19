/*************************************************************************************************
 * pixyCamCmds.h
 * - - C interface file for MSP430 UCSI UART A1, A0
 *
 *  Author: Iakov Umrikhin
 *  Created on: somewhere in January
 *  Modified: 01.03.2022
 **************************************************************************************************/

#ifndef PIXYCAMCMDS_H_
#define PIXYCAMCMDS_H_

    #define BUF_PIXY_SIZE 100
    #define MAX_PIXY_ARGS 2
    #define MAX_PIXY_CMDS 2
    #define CMD0_PIXY_NARG 0
    #define CMD1_PIXY_NARG 2

// command name
    #define CMD0_PIXY "pixyVer"
    #define CMD1_PIXY "pixyVec"

// global variables
    char rxBuffer[BUF_PIXY_SIZE];
    volatile signed long int rxString[50];  // store commands from the terminal

    unsigned char enterConsole;

    typedef struct PIXY_CMD {
        const char* cmdName;
        int nArgs; // number of input arguments for a command
        signed long int args[MAX_PIXY_ARGS]; // arguments
        char *encRegName;
    }PIXY_CMD;

// functions
//    void usciA1UartInit();
//
//    void usciA1UartTxChar(signed long int txChar);
//
//    void usciA1UartTxString(unsigned char *txChar);
//
//    char *usciA1UartGets(char *rxString);

    int parseCmd(PIXY_CMD * cmdList , char * cmdLine);

    int validateCmd(PIXY_CMD * cmdList ,char * cmdName);

    int executeCmd(PIXY_CMD * cmdList, int cmdIndex);

    void initCmdList(PIXY_CMD* cmdList);
#endif /* USCIUART_H_ */

