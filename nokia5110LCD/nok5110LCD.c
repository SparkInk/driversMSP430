/*************************************************************************************************
 * nok5110LCD.c
 * - C implementation or source file for NOKIA 5110 LCD.
 *
 *  Author: Greg Scutt
 *  Created on: Feb 20th, 2017
 **************************************************************************************************/



// nok5110LCD pin connectivity --> to MSP-EXP430F5529LP EVM.
//  8-LIGHT  	-->  	no connection necassary
//  7-SCLK  	-->  	MS430EVM  P4.3 or UCB1CLK
//  6-DN(MOSI)  -->  	MS430EVM  P4.1 or UCB1SIMO
//  5-D/C'  	-->  	MS430EVM  P4.2. 	Kept as I/O pin !!
//  4-RST'  	-->  	MS430EVM or supply VSS
//  3-SCE'  	-->  	MS430EVM  P4.0.  	Kept as I/O pin !!
//  2-GND  		-->  	MS430EVM or supply VSS
//  1-VCC  		-->  	MS430EVM or supply 3V3



#include <msp430.h>
#include "nok5110LCD.h" // remove _lab from file names
#include "usciB1Spi.h"
#include "usciUart.h"
// 2-D 84x6 array that stores the current pixelated state of the display.
// remember a byte (8 bits) sets 8 vertical pixels in a column allowing 8x6=48 rows
// note that this array is GLOBAL to this file only. In that way it is protected from access from other functions in files.
// said another way - it is a private global array with local scope to the file in which the defining declaration exists.
// we don't want other functions messing with the shadow RAM. This is the reason for static and for its dec/defn in the .c file
static unsigned char currentPixelDisplay[LCD_MAX_COL][LCD_MAX_ROW / LCD_ROW_IN_BANK];

/************************************************************************************
* Function: nokLcdWrite
* - performs write sequence to send data or command to nokLCD. Calls spiTxByte to transmit serially to nokLCD
* argument:
* Arguments: lcdByte - the 8 bit char (data or command) that is written to nokLCD.
* 			 cmdType - 0 - lcdByte is a cmd,   1 - lcdByte is data.
* return: none
* Author: Iakov Umrikhin
* Date: Feb 20th, 2017
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
void nokLcdWrite(char lcdByte, char cmdType) {
	// ********** complete this function. *************

	// check cmdType and output correct DAT_CMD signal to PORT4 based on it.
	switch(cmdType) {	
		case DC_CMD:		// lcdByte is a command
			DnC &= ~DAT_CMD;	
			break;
		case DC_DAT:		// lcdByte is data
			DnC |= DAT_CMD;	
			break;
	}
	// activate the SCE. the chip select
	nSCE &= ~SCE;	// P4.0 is LOW -> enabled
	// transmit lcdByte with spiTxByte
	spiTxByte(lcdByte);

	// wait for transmission  complete
	//while (UCB1IFG & UCRXIFG);		// RxBUFF should be empty => the transmission has ended
	while (UCB1STAT & UCBUSY);

	//DnC &= ~DAT_CMD;

	nSCE |= SCE;    //  P4.0 is HIGH -> disabled

}

/************************************************************************************
* Function: nokLcdInit
* -
* argument:
*	none
* return: none
* Author: Greg Scutt
* Date: Feb 20th, 2017
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
void nokLcdInit(void) {

	nSCE &= ~SCE;		// enable SCE (chip select; it is an active low pin)

	_NOK_LCD_PWR_ON;
    _NOK_LCD_RST;

	// gScutt.  do an SPI init with ucsiB1SpiInit  from ucsiSpi.h before this function call !!

	//P4OUT 	&= 	~(SCE |	DAT_CMD);	// Set DC and CE Low
    
	// send initialization sequence to LCD module
	nokLcdWrite(LCD_EXT_INSTR, DC_CMD);		// enable extended instruction set
	nokLcdWrite(LCD_SET_OPVOLT, DC_CMD);	// set operation voltage for contrast ctrl	
	nokLcdWrite(LCD_SET_TEMPCTRL, DC_CMD);	// set coeff 2
	nokLcdWrite(LCD_SET_SYSBIAS, DC_CMD);	// set system bias mode
	nokLcdWrite(LCD_BASIC_INSTR, DC_CMD);	// enable basic instruction set
	nokLcdWrite(LCD_NORMAL_DISP, DC_CMD);	// normal display control

	nSCE |= SCE;
 	nokLcdClear(); // clear the display
}

void nokLcdClear(void) {
    unsigned int addrCount;
    nokLcdSetPixel(0, 0);

    for (addrCount = 0; addrCount < (6 * LCD_MAX_COL); addrCount++) {
        nokLcdWrite(0x00, DC_DAT);
    }
    nokLcdSetPixel(0, 0);
}

/************************************************************************************
* Function: nokLcdSetPixel
* -
* argument:
*	xPos - The horizontal pixel location in the domain (0 to 83)
*	yPos - The vertical pixel location in the domain (0 to 47)
*
* return: 0 - pixel was valid and written.  1 - pixel not valid
* Author: Greg Scutt
* Date: Feb 20th, 2017
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
unsigned char  nokLcdSetPixel(unsigned char xPos, unsigned char yPos) {
	unsigned char bank; // a bank is a group of 8 rows, selected by 8 bits in a byte
	unsigned char result = 1;
	// verify pixel position is valid
	if ((xPos < LCD_MAX_COL) && (yPos < LCD_MAX_ROW)) {

		// if-else statement avoids costly division
		if (yPos<8) bank = 0;
		else if (yPos<16) bank = 1;
		else if (yPos<24) bank = 2;
		else if (yPos<32) bank = 3;
		else if (yPos<40) bank = 4;
		else if (yPos<48) bank = 5;

		// set the x and y RAM address  corresponding to the desired (x,bank) location. this is a command DC_CMD
		nokLcdWrite(LCD_SET_XRAM | xPos, DC_CMD);	// set X address of RAM
		nokLcdWrite(LCD_SET_YRAM | bank, DC_CMD);	// set Y address of RAM

		// update the pixel being set in currentPixelDisplay array
		currentPixelDisplay[xPos][bank] = BIT0 << (yPos % LCD_ROW_IN_BANK); // i.e if yPos = 7 then BIT0 is left shifted 7 positions to be 0x80. nice
		nokLcdWrite(currentPixelDisplay[xPos][bank], DC_DAT); // write the data. this is DATA DC_DAT
		result = 0;
	}
	return (!result) ? 0 : 1;
}

/************************************************************************************
* Function: nokLcdDrawScrnLine (coordValue, lineName)
* -         this function draw either a horizontal or a vertical line on the screen
*
* argument:
*          coorValue -> x coordinate <= 84 or y coordinate <= 48;
*          lineName  -> 0: horizontal; 1: vertical
*
* return: 0 - pixel was valid and written.  1 - pixel not valid
* Author: Iakov Umrikhin
* Date: Feb
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
unsigned char nokLcdDrawScrnLine (char coorValue, char lineName) {
    // coorValue : if x <= 84; if y <= 48;
    // lineName: 0 if Horizontal; 1 if Vertical;
    unsigned char result = -1;
    unsigned char xCount = 0;
    unsigned char yCount = 0;

    // vertical or horizontal addressing
    switch(lineName) {
    case 0:     // horizontal line => y coordinate matters
        if (coorValue < LCD_MAX_ROW) {  // pass for y coordinate
            result = 0;
            nokLcdSetPixel(0, coorValue);
            while (xCount  < LCD_MAX_COL - 1) {
                nokLcdWrite(BIT0 << (coorValue % LCD_ROW_IN_BANK), DC_DAT);
                xCount++;
            }
        }
        break;
    case 1:     // vertical line => x coordinate matters
        if (coorValue < LCD_MAX_COL) {  // pass for x coordinate
            result = 0;
            nokLcdWrite(LCD_BASIC_INSTR | VERT_HORZ_ADDR, DC_CMD);    // vertical addressing
            nokLcdWrite(LCD_SET_XRAM | coorValue, DC_CMD);   // set X address of RAM
            while (yCount  < (LCD_MAX_ROW) / 8) {
                nokLcdWrite(0xff, DC_DAT);
                yCount++;
                }
            nokLcdWrite(LCD_BASIC_INSTR & ~VERT_HORZ_ADDR, DC_CMD);
        }
        break;
    }   // switch end

    return result;
}


int nokLcdDrawLine (int xCoordZero, int yCoordZero, int xCoordF, int yCoordF) {
   // int xCoord = xCoordZero;
    //int yCoord = yCoordZero;
    //int yi, xi = 1;
    int dx = abs(xCoordF - xCoordZero);
    int dy = -abs(yCoordF - yCoordZero);
    int sx = xCoordZero < xCoordF ? 1 : -1;
    int sy = yCoordZero < yCoordF ? 1 : -1;
    int err = dx + dy;
    int e2;
    /*if (dy < 0) {
        yi = -1;
        dy = -dy;
    }*/

   // int D = 2 * dy - dx;

    signed char result = -1;

    nokLcdSetPixel(0, 0);   // set pixel to the start

    if (((xCoordZero || xCoordF) < LCD_MAX_COL) || ((yCoordZero || yCoordF) < LCD_MAX_ROW) ){
        result = 0;
        while (1) {
           nokLcdSetPixel(xCoordZero, yCoordZero);
           if (xCoordZero == xCoordF && yCoordZero == yCoordF) break;
           e2 = 2 * err;
           if (e2 >= dy){
               if (xCoordZero == xCoordF) break;
               err += dy;
               xCoordZero += sx;
           }
           if (e2 <= dx) {
               if (yCoordZero == yCoordF) break;
               err += dx;
               yCoordZero += sy;
           }
        }
    }
    nokLcdSetPixel(0, 0);   // set the pixel back to the start
    return (result == 0) ? 0 : -1;
}
/************************************************************************************
* Function: spiTxByte
* - if TXBUFFER is ready!!  then a byte is written to buffer and serialized on SPI UCB1. nothing else happens.
* argument:
*	txData - character to be serialized over SPI
*
* return: none
* Author: Iakov Umrikhin
* Date: Feb 20th, 2017
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
void spiTxByte(char txData)
{
	// transmit a byte on SPI if it is ready !!
	if (UCB1IFG & UCTXIFG) {      // TXBUF is ready to accept another byte
		usciB1SpiPutChar(txData);
    }
}


