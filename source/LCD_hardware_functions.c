#include "main.h"
#include "LCD_hardware_functions.h"
#include "PORT.H"
#include "Outputs.h"


/***********************************************************************
DESC:    Writes data to the LCD with proper order for RS and E
INPUT: Uint8 for RS and Data to be written
       RS=0 instruction, RS=1 data
RETURNS: Nothing
CAUTION: None
************************************************************************/
void LCD_Write(uint8_t RegSelect, uint8_t LCD_Data)
{
  if(RegSelect==0)     // Set or Clear RS before E is Set
  {
	P3_clear_bit(RS_bit);
  }
  else
  {
    P3_set_bit(RS_bit);
  }
  P3_set_bit(E_bit);  // Set Enable before data written
  P0_output_bits(LCD_Data);         
  P3_clear_bit(E_bit);   
  P0_output_bits(0xFF);
  P3_set_bit(RS_bit);
}



