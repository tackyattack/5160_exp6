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
