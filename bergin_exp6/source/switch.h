#ifndef _SWITCH_H
#define _SWITCH_H

#include "main.h"
#include "PORT.H"

#define MS_PER_TICK (10)
#define DEBOUNCE_INTERVAL_MS (25)
#define INTERVAL_TICKS (DEBOUNCE_INTERVAL_MS/MS_PER_TICK)

typedef enum {not_pressed, debounce_p, pressed, held, debounce_r} switch_state_t;
typedef struct
{
  uint8_t SW_mask;
  uint8_t debounce_time;
  switch_state_t SW_state;
} SW_values_t;

void switch_init(SW_values_t *SW_input_p, uint8_t mask);
void read_switch(SW_values_t *SW_input_p);

#endif
