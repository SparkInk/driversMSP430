/*
 * commands.h
 *
 *
 *  Author: Iakov (aka Iasha) Umrikhin
 *  Date Created: 15.05.2022 (dd.mm.yyyy)
 *  Date Modified: 15.05.2022 (dd.mm.yyyy)
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_
        
    // commands definitions
    #define BUF_SIZE    100
    #define MAX_ARGS    2
    #define MAX_CMDS    11
    #define CNT_TO_ANG  13
    #define ONE_REV     360
    // commands' number of arguments
    #define CMD0_NARG   1
    #define CMD1_NARG   2
    #define CMD2_NARG   0
    #define CMD3_NARG   2
    #define CMD4_NARG   1
    #define CMD5_NARG   2
    #define CMD6_NARG   0
    #define CMD7_NARG   0
    #define CMD8_NARG   1
    #define CMD9_NARG   0
    #define CMD10_NARG  2

    // command name
    #define CMD0    "dutyC"
    #define CMD1    "mtrDir"   // implements both mtrCW and mtrCCW
    #define CMD2    "brake"
    #define CMD3    "pwmSet"
    #define CMD4    "inputCtrl"
    #define CMD5    "fediHome"
    #define CMD6    "fediClr"
    #define CMD7    "fediRead"
    #define CMD8    "fediDisp"
    #define CMD9    "fediFw"
    #define CMD10   "updateSetPoint"

    #define MAX_ARGS 4

    // global variables
    typedef struct CMD {
        const char* cmdName;
        int nArgs; // number of input arguments for a command
        unsigned int args[MAX_ARGS]; // arguments
        char *encRegName;
    }CMD;

    volatile long signed int setPointGlobal;
    unsigned char dispMode;     // display mode: 0 -> single line mode; 1 -> horizontal bar mode
    volatile signed int angleOffset;

    unsigned int posCtrl(signed long int posCount, signed long int setPoint, PID *pidGain, signed char *motorDirection);

    void changePID(PID *pid);

    void clearArrowKeys(ARROWS *arrow);
    
    void detectArrowKey(ARROWS *arrow);

    signed int constrain(signed int modifiedVar, signed int lowBoundary, signed int highBoundary);

    int parseCmd(CMD * cmdList , char * cmdLine);

    int validateCmd(CMD * cmdList ,char * cmdName);

    int executeCmd(CMD * cmdList, int cmdIndex);

    void initCmdList(CMD* cmdList);
#endif /* COMMANDS_H_ */
