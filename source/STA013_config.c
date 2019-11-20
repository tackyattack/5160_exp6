#include "main.h"
#include "PORT.H"
#include "I2C.h"
#include "STA013_Config.h"
#include <stdio.h>

/************ Private Functions for STA013 Reset *********************/
#define STA013_RESET_bit (1<<3)

void Clear_P3_bit(uint8_t Bit_Data);
void Set_P3_bit(uint8_t Bit_Data);
/********************************************************************/

void Clear_P3_bit(uint8_t Bit_Data)
{
  P3&=(~Bit_Data);
}


void Set_P3_bit(uint8_t Bit_Data)
{
  P3|=Bit_Data;
}

/********************************************************************/


/***********************************************************************
DESC:    Initializes the STA013 with the following steps:
         Applies a hardware reset signal, checks the ID register for an 0xAC
		 response and then sends the configuration data.
RETURNS: error flag
CAUTION: 
          
************************************************************************/


uint8_t STA013_init(void)
{
   uint8_t error_flag, timeout, reg_addr;
   uint8_t idata send[3], rcv[3];
   uint8_t * config_data_p;
   uint16_t j;

/***** Set up for STA013, XTAL=14.7456MHz, 512 oversampling ratio (STA013 datasheet, Table 12), I2S compatible,
       DLA and DRA 12dB attenutation, Data_Req active low,  ****/
   uint8_t code STA_cfg_addr[18]={0x54, 0x55,  6, 11,  82,  81, 101, 100, 80, 97, 0x05, 0x0d, 0x18, 0x0c, 0x46, 0x48, 0x72};
   uint8_t code STA_cfg_data[18]={0x07, 0x10,  9,  2, 184,   0,   0,   0,  6,  5, 0xa1, 0x00, 0x04, 0x05, 0x0C, 0x0C, 0x01};

   extern uint8_t code CONFIG;
   extern uint8_t code CONFIG2;

   Clear_P3_bit(STA013_RESET_bit);  // Reset STA013

   send[0]=0x01;   // address of the ID reg.  
   timeout=50;

   Set_P3_bit(STA013_RESET_bit);    // make STA013 active

   do
   {
      //P3_4=0;          // Trigger for oscilloscope, if needed
      error_flag=I2C_Read(STA013,0,0,3,rcv);
      timeout--;
      //P3_4=1;         
   }while((error_flag!=no_errors)&&(timeout!=0));

   if(timeout!=0)
   {
       printf("Output from ID register: %2.2bX \n\r",rcv[2]);
   }

   do
   {
      //P3_4=0;          // Trigger for oscilloscope, if needed
      error_flag=I2C_Read(STA013,1,1,1,rcv);
      timeout--;
      //P3_4=1;         
   }while((error_flag!=no_errors)&&(timeout!=0));
   
   if(timeout!=0)
   {
       printf("Output from ID register: %2.2bX \n\r",rcv[0]);
   }



   if(rcv[0]==0xAC)
   {
	 error_flag=I2C_Read(STA013,0x71,1,1,rcv);
     printf("STA013 SoftVersion: %2.2bX \n\r",rcv[0]); //Not needed. Used to check if software version changed with config download
     j=0;
     config_data_p=&CONFIG;  // point to STA013 configuration file
     do
     {
        //reg_addr=*(config_data_p+j);   // Accessed using a pointer
        reg_addr=config_data_p[j];       // or accessed as an array
        j++;
        //send[0]=*(config_data_p+j);
        send[0]=config_data_p[j];
        j++;
		if((j&0x3F)==0) printf(".");    // prints a dot periodically to indicate configuration download progress
        if(reg_addr!=0xff)
        {
          
           timeout=50;
           do
           {
               error_flag=I2C_Write(STA013,reg_addr,1,1,send);
               timeout--;
               if(timeout==0) AMBERLED=0;
           }while((error_flag!=no_errors)&&(timeout!=0));
        }
     }while((reg_addr!=0xff) && (timeout!=0));
     printf("\nFirst configuration file sent...\n");
     printf("Number of values written: %d \n",j>>1);
     j=0;
     config_data_p=&CONFIG2;  // point to STA013 configuration file
     do
     {
        reg_addr=*(config_data_p+j);      // Accessed using a pointer
        //reg_addr=config_data_p[j];      // or accessed as an array
        j++;
        send[0]=*(config_data_p+j);
        //send[0]=config_data_p[j];
        j++;
		if((j&0x07)==0) printf(".");    // prints a dot periodically to indicate configuration download progress
        if(reg_addr!=0xff)
        {
           timeout=50;
           do
           {
               error_flag=I2C_Write(STA013,reg_addr,1,1,send);
               timeout--;
               if(timeout==0) AMBERLED=0;
           }while((error_flag!=no_errors)&&(timeout!=0));
        }
     }while((reg_addr!=0xff) && (timeout!=0));
	 printf("\nSecond configuration file sent...\n");
     printf("Number of values written: %d \n",j>>1);
	 error_flag=I2C_Read(STA013,0x71,1,1,rcv);
     printf("STA013 SoftVersion: %2.2bX \n\r",rcv[0]); // Determined that software version did not change            
        
     /***** Set up for STA013, XTAL=14.7456MHz, 512 oversampling ratio (STA013 datasheet, Table 12), I2S compatible,
             DLA and DRA 12dB attenutation, Data_Req active low,  ****/
      for(j=0;j<17;j++)
	  {
	    reg_addr=STA_cfg_addr[j];
	    send[0]=STA_cfg_data[j];
        timeout=50;
        do
        {
            error_flag=I2C_Write(STA013,reg_addr,1,1,send);
            timeout--;
            if(timeout==0) AMBERLED=0;
        }while((error_flag!=no_errors)&&(timeout!=0));
		printf("Sent to STA013: %2.2bX  %2.2bX \n\r",reg_addr,send[0]);
		reg_addr=STA_cfg_addr[j];
        /**** Read and print the set up values to verify the values. ****/
        error_flag=I2C_Read(STA013,reg_addr,1,1,rcv);
        printf("Output from register: %2.2bX  %2.2bX %2.2bX \n\r",reg_addr,rcv[0],error_flag);
	  }
      printf("Configuration Complete\n\r");
   }      
   return error_flag;
}
