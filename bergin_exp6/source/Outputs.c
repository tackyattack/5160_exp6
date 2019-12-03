#include "main.h"
#include "PORT.H"

/*********************************************************************
*    Port Output Functions
********************************************************************/

/***********************************************************************
DESC:  Writes the value passed as a parameter to the specified port
INPUT: uint8_t value to be written to a port
RETURNS: Nothing
CAUTION: Only uncomment the functions that are to be used
************************************************************************/
void P0_output_bits(uint8_t Port_Data)
{
  P0=Port_Data;
}

/* void P1_output_bits(uint8_t Port_Data)
{
  P1=Port_Data;
}  */

/* void P2_output_bits(uint8_t Port_Data)
{
  P2=Port_Data;
}  */

/* void P3_output_bits(uint8_t Port_Data)
{
  P3=Port_Data;
} */

/*********************************************************************
*    Bit Output Functions
********************************************************************/


/***********************************************************************
DESC:  Clears the specified bits on a specified port
INPUT: 8-bit pattern with '1' bits for the bits to be cleared
RETURNS: Nothing
CAUTION: Only uncomment the functions that are to be used
************************************************************************/
/* void P0_clear_bit(uint8_t Bit_Data)
{
  P0&=(~Bit_Data);
} */

void P1_clear_bit(uint8_t Bit_Data)
{
  P1&=(~Bit_Data);
}

/* void P2_clear_bit(uint8_t Bit_Data)
{
  P2&=(~Bit_Data);
} */

void P3_clear_bit(uint8_t Bit_Data)
{
  P3&=(~Bit_Data);
}

/***********************************************************************
DESC:  Set the specified bits on a specified port
INPUT: 8-bit pattern with '1' bits for the bits to be set
RETURNS: Nothing
CAUTION: Only uncomment the functions that are to be used
************************************************************************/
/* void P0_set_bit(uint8_t Bit_Data)
{
  P0|=Bit_Data;
} */

void P1_set_bit(uint8_t Bit_Data)
{
  P1|=Bit_Data;
}

 void P2_set_bit(uint8_t Bit_Data)
{
  P2|=Bit_Data;
}

void P3_set_bit(uint8_t Bit_Data)
{
  P3|=Bit_Data;
}
