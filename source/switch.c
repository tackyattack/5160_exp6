#include "main.h"
#include "PORT.H"
#include "switch.h"

#define INPUT_PORT (P2)

void switch_init(SW_values_t *SW_input_p, uint8_t mask)
{
  SW_input_p->SW_mask=mask;
  SW_input_p->debounce_time=0;
  SW_input_p->SW_state=not_pressed;
}

void read_switch(SW_values_t *SW_input_p)
{
  if(SW_input_p->SW_state == not_pressed)
  {
    if((INPUT_PORT&SW_input_p->SW_mask)==0)
    {
      SW_input_p->debounce_time=0;
      SW_input_p->SW_state=debounce_p;
    }
  }
  else if(SW_input_p->SW_state == debounce_p)
  {
    SW_input_p->debounce_time++;
    if(SW_input_p->debounce_time>=INTERVAL_TICKS)
    {
      if((INPUT_PORT&SW_input_p->SW_mask)==0)
      {
        SW_input_p->SW_state=pressed;
        // task execute
      }
      else
      {
        SW_input_p->SW_state=not_pressed;
      }
    }
  }
  else if(SW_input_p->SW_state==held)
  {
    if((INPUT_PORT&SW_input_p->SW_mask)!=0)
    {
      SW_input_p->SW_state=debounce_r;
    }
  }
  else if(SW_input_p->SW_state==debounce_r)
  {
    SW_input_p->debounce_time++;
    if(SW_input_p->debounce_time>=INTERVAL_TICKS)
    {
      if((INPUT_PORT&SW_input_p->SW_mask)!=0)
      {
        SW_input_p->SW_state=not_pressed;
      }
      else
      {
        SW_input_p->SW_state=held;
      }
    }
  }
}
