#include <stdio.h>
#include "main.h"
#include "PORT.H"
#include "Directory_Functions_struct.h"
#include "file_system.h"
#include "LED_Control.h"
#include "SPI.h"
#include "SDCard.h"
#include "print_bytes.h"
#include "Timer0_hardware_delay_1ms.h"
#include "UART.h"
#include "LCD_Routines.h"
//#include "Long_Serial_In.h"
#include "Outputs.h"
#include "player.h"
#include "STA013_config.h"
#include "I2C.h"
#include "song_navigator.h"

xdata uint8_t buf1[512];
xdata uint8_t buf2[512];
uint8_t code LCD_str_start[]="Starting...";

main()
{
   uint8_t idata  error_flag;
   // uint32_t idata entry_num, cwd_entries;
   // uint32_t idata cwd, clus;

   AUXR=0x0c;   // make all of XRAM available, ALE always on
   if(OSC_PER_INST==6)
   {
       CKCON0=0x01;  // set X2 clock mode
   }
   else if(OSC_PER_INST==12)
   {
       CKCON0=0x00;  // set standard clock mode
   }

   LEDS_ON(Red_LED);
   P2_set_bit(0x0F);

   Timer0_DELAY_1ms(300);

   LCD_Init();
   LCD_Print(0x00, sizeof(LCD_str_start)-1, LCD_str_start);
   
   uart_init(9600);
   printf("starting program...\n");

   if(STA013_init() != no_errors) while(1);

   // My SD card won't init at 400kHz, so using 200kHz
   error_flag=SPI_Master_Init(200000UL);
   if(error_flag!=no_errors)
   {
      LEDS_ON(Red_LED);  // An error causes the program to stop
	    UART_Transmit('S');
      while(1);
   }
   LEDS_ON(Amber_LED);
   // SD Card Initialization
   error_flag=SD_card_init();
   if(error_flag!=no_errors)
   {
      LEDS_ON(Red_LED);  // An error causes the program to stop
	  UART_Transmit('C');
      while(1);
   }
   LEDS_OFF(Amber_LED);
   error_flag=SPI_Master_Init(20000000UL);
   if(error_flag!=no_errors)
   {
      LEDS_ON(Red_LED);  // An error causes the program to stop
	    UART_Transmit('P');
      while(1);
   }

   printf("Mounting SD card\n");
   if(mount_drive(buf1) == MOUNT_SUCCESS)
   {
     printf("SD card mounted\n");
     LEDS_OFF(Red_LED);
     LEDS_ON(Green_LED);
   }
   else
   {
     printf("SD card failed to mount\n");
     while(1);
   }

   //printf("Root directory:\n\n");
   //cwd = Export_Drive_values()->FirstRootDirSec; // start out at root directory

   printf("Refer to LCD for navigation\n");
   song_navigator_init(Export_Drive_values()->FirstRootDirSec);
   song_navigator_runner();

   while(1);

   // while(1)
   // {
   //   cwd_entries = Print_Directory(cwd, buf1);
   //   printf("Enter an entry number: \n");
   //   entry_num = long_serial_input();
   //   printf("%lu  ", entry_num);
   //   printf("%lu  \n", cwd);
   //   // check to make sure entry is within the cwd
   //   if(entry_num <= cwd_entries)
   //   {
   //     clus = Read_Dir_Entry(cwd, entry_num, buf1);
   //     if(clus & directory_bit)
   //     {
   //       clus &= 0x0FFFFFFF; // mask off upper four bits to print another directory
   //       cwd = first_sector(clus);
   //     }
   //     else
   //     {
   //       init_player(clus);
   //       while(player_state_machine_runner() == PLAYER_RUNNING);
   //     }
   //   }
   //   else
   //   {
   //     printf("Error: invalid entry choice\n");
   //   }
   // }
}
