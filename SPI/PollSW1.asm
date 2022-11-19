;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
; 
; Description: Software based SPI protocol implemented in Assembler 
; 
; Author: Iakov Umrikhin
; Date Created: 
; Date Modified: 04.11.2021 (dd:mm:yyyy)
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer

	; constants
ramS0Base	.equ 0x2400				; base addresses of data to serialize
OneKB		.equ 0x1024				; 1KB definition

LED1 		.equ BIT0
LED2 		.equ BIT7
SW1			.equ BIT1
LOOP_10ms 	.equ 3333

			.asg R4, counter

	; variables
			.define BIT0, SOUT		; P3OUT<0>
			.define BIT1, SCLK		; P3OUT<1>
			.define R7, byteCtr		; byte counter
			.define R9, byte		; index
			.define R13, numBytes	; numBytes of data stoared in R13
			.define R14, delay		; to delay SPI for 1 ms
			.define R15, ramS0ptr	; R12 points to the array of data


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------


INIT:
			bic.b #LED1, &P1OUT		; set output to 0
			bis.b #LED1, &P1DIR
			bic.b #LED2, &P4OUT		; set output to 0
			bis.b #LED2, &P4DIR

									; COMPLETE: configure the PORT so that SW1 can be read as INPUT. 3 registers. Use SW1 defn above.
			bis.b #SW1, &P2REN		; enable the internal resistor
			bic.b #SW1, &P2DIR		; set P2<1> as input
			bis.b #SW1, &P2OUT 		; set the resistor as pull-up


PORT3_INIT:										; initialize the required PORTS
			bic #SOUT, &P3OUT					; clear P3OUT<0>
			bic #SCLK, &P3OUT					; clear P3OUT<1>

			bis #SOUT, &P3DIR					; set P3DIR<0> to 1
			bis #SCLK, &P3DIR					; set P3DIR<1> to 1

			clr counter
VAR_BYTE:
			mov.b #4, numBytes					; 3  load counter with number of bytes to serialize

MAIN_LOOP:
			clr byteCtr
			mov.w #ramS0Base, ramS0ptr			; 3  provide address of bytes to serialize
			mov.w #0x9554, &0x2400
			mov.w #0xFF81, &0x2402
												; show the number of cycles for each instruction in the adjacent comment
												; jumps of any kind are always 2, NOPs are 1
SW_HIGH:
			bit.b #SW1, &P2IN			;  COMPLETE: fill in correct &PxIN
			jnz SW_HIGH

SW_LOW:
			bit.b #SW1, &P2IN 			;  COMPLETE fill in correct &PxIN
			jz SW_LOW
			call #DELAY_10m

LED_T0GGGLE:
			xor #LED1,&P1OUT
			inc.b counter
BYTE_LOOP:										; the outer loop to go through all the bytes
			cmp byteCtr, numBytes				; numBytes - byteCtr
			jz DONE								; jumps to DONE lable if there is no more data
			inc byteCtr

			push byte
			mov.b @ramS0ptr+, byte
			call #piso1
			pop byte

			jmp SW_HIGH
DONE:
			bic #SOUT, &P3OUT					; clears bit P3OUT<0>
			bic #SCLK, &P3OUT					; clears bit P3OUT<1>

			clr byteCtr
			clr ramS0ptr
			clr &ramS0Base

			jmp MAIN_LOOP



;--------------------------------------------------------------------------------------------------------
;Subroutine Delay_10m: provides a scalable N*10ms delay.N is an integer passed to the subroutine
;
; Input Parameters:
; R12 - Input parameter used to multiply 10ms. Total delay = (R12)*10ms.
;
; Registers Used In Subroutine: R10
; Registers Destroyed: R12. User is reqd to save R12 before passing parameter with it
;
; Local Variables:
; R10 - inner loop counter.

; Output Parameters: none.
;---------------------------------------------------------------------------------------------------------
DELAY_10m:
			; local variable declaration
			.asg R10, innerLoopCtr			;  the .asn statement is like .define except it can be re-defined at any time. So it's used in subroutines to name the local vars.
			.asg R12, outerLoopCtr


											; the outer loop
			push innerLoopCtr				; save R10 on the stack. [3 MCLK cycles]
			jmp LOOP_TEST					; jump to test innerLoopCtr in case it is zero [2 MCLK cycles]
DELAY_LOOP1:
			mov.w #LOOP_10ms, innerLoopCtr	; load the inner delay count 3333 into register innerLoopCtr.   Set to 1 for single stepping. Be sure to change it back!!! [2 MCLK cycles]
DELAY_LOOP2:								; the inner loop
			dec.w innerLoopCtr				; dec is emulated with a subtraction sub #1,R5 !!  so it is not a single operand instruction.  [1 MCLK cycles]
			jnz DELAY_LOOP2					; all jumps have [2 MCLK cycles]

			dec.w outerLoopCtr				; [1 MCLK cycles]
LOOP_TEST:	cmp #0, outerLoopCtr			; check if R8 is zero
			jnz DELAY_LOOP1					; [2 MCLK cycles]

			pop innerLoopCtr				; restore R10 [2 MCLK cycles]
			ret								; return to calling subroutine [2 MCLK cycles]
;-----------------------------------------------------------------------------------------------------------

;  fill in subroutine header
;--------------------------------------------------------------------------------------------------------
;Subroutine piso1:
;
; Input Parameters:

;
; Registers Used In Subroutine:
; Registers Destroyed:
;
; Local Variables:
;

; Output Parameters: none.
;---------------------------------------------------------------------------------------------------------
piso1:
			.asg R11, shiftBit
			.asg R6, bitCtr
			push shiftBit
			mov.b #BIT7, shiftBit
			push bitCtr
		    mov.b #8, bitCtr					; load 8 1 byte to bitCtr
BIT_LOOP: 										; the inner loop to go serialize all the bits within a byte and output them with SCLK on P3OUT
			SETC								; prepare the carry bit for MSB
												; look up the rotate (rra, rrc, rla, rlc) intructions in 6.6.2 of user manual
IF_HIGH:	bit.b byte, shiftBit ; test MSB
			jz IF_LOW							; if bit is HIGH, jump to IF_LOW statement
												; writing data HIGH
			bis #SOUT, &P3OUT					; sets P3OUT<0> to HIGH
			bic #SCLK, &P3OUT					; sets P3OUT<1> to LOW
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			jmp WHILE_CHECK
IF_LOW:											; writing data LOW
			bic #SOUT, &P3OUT					; sets P3OUT<0> to LOW
			bic #SCLK, &P3OUT					; sets P3OUT<1> to LOW
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			NOP
			jmp WHILE_CHECK

WHILE_CHECK:
			bis #SCLK, &P3OUT					; reading data; P3OUT<1> is HIGH


			rrc shiftBit						; BIT7 == BIT7 >> 1
			cmp #8, bitCtr						; check if there are bits remaning
			dec bitCtr							; decrements bitCtr by 1
			jnz	BIT_LOOP						; compute how long it takes to serialize one byte in the inner loop. show here ..
			bic #SOUT, &P3OUT					; clear P3OUT<0>
			bic #SCLK, &P3OUT					; clear P3OUT<1>
			pop bitCtr
			pop shiftBit

			ret
;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
