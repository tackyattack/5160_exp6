#include <stdio.h>
#include "main.h"
#include "UART.h"


/***********************************************************************
DESC:    Initializes UART for mode 1
         Baudrate: 9600
INPUT: Nothing
RETURNS: Nothing
CAUTION: Uses Baudrate Generator instead of a standard timer
************************************************************************/
void uart_init(uint16_t BAUD_RATE)
{
  
  // configure UART
  // set or clear SMOD1 and clear SMOD0
  PCON &= 0x3F;  // Clears both SMOD0 and SMOD1
  PCON |= (SMOD1 << 7); // Sets SMOD1 to the defined value
   
  // serial interrupt is disabled
  ES=0;  

  SCON = UART_MODE1|RECEIVE_ENABLE|TRANSMIT_FLAG_SET;

  // Initialize Baud Rate Generator
  BDRCON=0;   //Stops and disables Baud Rate Gen.
  // BRL = BAUD_RATE_RELOAD   // Use this line for compile time baud rate set up.
  BRL= (uint8_t)(256-((1+(5*SPD))*(1+(1*SMOD1))*OSC_FREQ)/(32UL*OSC_PER_INST*BAUD_RATE));
  BDRCON= (0x1C | (SPD << 1));  // Enables Baud Rate Gen. for RxD and TxD

  

  // initially not busy
  TI=1;


} // uart_init


/***********************************************************************
DESC:    Sends one character through the UART
INPUT:   Character to send
RETURNS: Character sent or error flag
CAUTION: TI must be set during the initialization
************************************************************************/

uint8_t UART_Transmit(uint8_t send_value)
{
   uint8_t return_value;
   uint16_t timeout;
   timeout=0;
   do
   {
     timeout++;
   }while((TI==0)&&(timeout!=0));
   if(timeout!=0)
   {
     SBUF=send_value;
     return_value=send_value;
     TI=0;
   }
   else
   {
     return_value=UART_timeout;
   }
   return return_value;
}
   





/***********************************************************************
DESC:    Waits for a value to be received through the UART and returns 
         the value.
INPUT: Nothing
RETURNS: Received value
CAUTION: Will stop program execution until a character is received
************************************************************************/

uint8_t UART_Receive(void)
{
    uint8_t return_value;
    while(RI==0);
    return_value=SBUF;
    RI=0;
    return return_value;
}





