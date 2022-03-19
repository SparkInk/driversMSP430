/*************************************************************************************************
 * ucsiUart.h
 * - - C interface file for MSP430 UCSI UART A1, A0
 *
 *  Author: Greg Scutt
 *  Created on: March 1, 2017
 *  Modified: Feb 19, 2018
 **************************************************************************************************/

#ifndef USCIUART_H_
#define USCIUART_H_

    #define BUF_SIZE 100
    #define MAX_ARGS 4
    #define MAX_CMDS 11
    #define CMD0_NARG 2
    #define CMD1_NARG 0
    #define CMD2_NARG 4
    #define CMD3_NARG 2
    #define CMD4_NARG 0
    #define CMD5_NARG 0
    #define CMD6_NARG 1
    #define CMD7_NARG 0
// command name
    #define CMD0 "nokLcdDrawScrnLine"
    #define CMD1 "nokLcdClear"
    #define CMD2 "nokLcdDrawLine"
    #define CMD3 "fediHome"
    #define CMD4 "fediClr"
    #define CMD5 "fediRead"
    #define CMD6 "fediDisp"
    #define CMD7 "fediFw"
// global variables
    char rxBuffer[BUF_SIZE];
    volatile signed long int rxString[50];  // store commands from the terminal
    typedef struct CMD {
        const char* cmdName;
        int nArgs; // number of input arguments for a command
        signed long int args[MAX_ARGS]; // arguments
        char *encRegName;
    }CMD;

// functions
    void usciA1UartInit();

    void usciA1UartTxChar(signed long int txChar);

    void usciA1UartTxString(unsigned char *txChar);

    char *usciA1UartGets(char *rxString);

    int parseCmd(CMD * cmdList , char * cmdLine);

    int validateCmd(CMD * cmdList ,char * cmdName);

    int executeCmd(CMD * cmdList, int cmdIndex);

    void initCmdList(CMD* cmdList);
#endif /* USCIUART_H_ */
