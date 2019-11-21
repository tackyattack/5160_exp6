#include "sEOS.h"

extern uint8_t xdata buf1[BUF_SIZE];
extern uint8_t xdata buf2[BUF_SIZE];

// void sEOS_ISR(void) interrupt Timer_2_overflow using 1
// {
//   TF2 = 0; // clear timer flag that caused this interrupt
// }
//
// void sEOS_init(uint8_t interval_ms)
// {
//   T2CON=0;
//   uint16_t PRELOAD=(65536–((OSC_FREQ*interval_ms)/(OSC_PER_INST*1000UL))));
//   RCAP2H=(uint8_t)PRELOAD/256;
//   RCAP2L=(uint8_t)PRELOAD%256;
//   TF2=0; // Clear overflow
//   ET2=1; // Enable Timer 2 interrupt
//   EA=1;  // Set Global Interrupt Enable
//   TR2=1; // Start Timer Running
// }
