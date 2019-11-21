#ifndef _S_EOS_H
#define _S_EOS_H

#include "main.h"

#define Timer_2_overflow (5)


// ------ Public function prototypes -------------------------------
void sEOS_init(uint8_t interval_ms);
//void sEOS_ISR(void) interrupt Timer_2_overflow using 1;
void os_loop();

#endif
