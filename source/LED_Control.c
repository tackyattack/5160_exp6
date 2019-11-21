#include "main.h"
#include "PORT.H"
#include "LED_Control.h"


/***********************************************************************
DESC:  Switches LEDs connected to port 2 on (active low)
INPUT: uint8 LED_mask ('1' bits for LEDs to switch on), uint8 * port (not used)
RETURNS: nothing
CAUTION: Only works if LEDs are connected to port 2
************************************************************************/

void LEDS_ON (uint8_t LED_mask)
{
  P2&=(~LED_mask);
}

/***********************************************************************
DESC:  Switches LEDs connected to port 2 off (inactive high)
INPUT: uint8 LED_mask ('1' bits for LEDs to switch off), uint8 * port (not used)
RETURNS: nothing
CAUTION: Only works if LEDs are connected to port 2
************************************************************************/


void LEDS_OFF (uint8_t LED_mask)
{
  P2|=(LED_mask);
}

void LED_number(uint8_t num)
{
  LEDS_OFF(Green_LED);
  LEDS_OFF(Amber_LED);
  LEDS_OFF(Yellow_LED);
  LEDS_OFF(Red_LED);
  if((num&(1<<0)) != 0) LEDS_ON(Green_LED);
  if((num&(1<<1)) != 0) LEDS_ON(Amber_LED);
  if((num&(1<<2)) != 0) LEDS_ON(Yellow_LED);
  if((num&(1<<3)) != 0) LEDS_ON(Red_LED);
}
