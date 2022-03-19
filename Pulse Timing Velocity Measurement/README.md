#PTVM system

Pulse timing velocity measurement using MPS430F5529LP.

TIMER A0 is used to update the velocity of a motor at a frequency of ~71 kHz.

It is more efficient that way, and the measurement error is less than 10%.

TIMER A1 is used to capture the rising edge from the channel A of the motor's encoder. Capturing occures on two consecutive rising edges.

UART protocol is used to display the current velocity of the motor in rpm in the Code Composer Studio's (CCS) terminal.

Also, it shows the direction of the motor's rotation.
