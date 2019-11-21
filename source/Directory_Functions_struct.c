#include "AT89C51RC2.h"
#include <stdio.h>
#include "main.h"
#include "PORT.H"
#include "UART.h"
#include "SPI.h"
#include "SDCard.h"
#include "File_System.h"
#include "print_bytes.h"
#include "Directory_Functions_struct.h"
#include "sector_reader.h"



FS_values_t idata Drive_values;

// use one of the buffers to store our sectors
extern uint8_t xdata buf1[512];
// use the other as a print buffer
extern uint8_t xdata buf2[512];
// how many bytes are in our print buffer
uint16_t idata print_buffer_pos = 0;


// --------- data pool ---------
// Note: since we're using a pool, only one task can be running at a time
uint32_t idata pool32[4];
uint16_t idata pool16[3];
uint8_t  idata pool8[4];

typedef struct
{
  uint32_t *Sector;
  uint32_t *Sector_num;
  uint32_t *max_sectors;
  uint16_t *i;
  uint16_t *entries;
  uint8_t *j;
  uint8_t *attr;
  uint8_t *out_val;
  uint8_t *error_flag;
} print_directory_task_context_t;

typedef struct
{
  uint32_t *Sector;
  uint32_t *return_clus;
  uint32_t *Sector_num;
  uint32_t *max_sectors;
  uint16_t *i;
  uint16_t *entries;
  uint16_t *entry;
  uint8_t *j;
  uint8_t *attr;
  uint8_t *out_val;
  uint8_t *error_flag;
} read_dir_entry_task_context_t;
// -----------------------------

void get_print_directory_task_context(print_directory_task_context_t *context)
{
  context->Sector      = pool32 + 0;
  context->Sector_num  = pool32 + 1;
  context->max_sectors = pool32 + 2;

  context->i           = pool16 + 0;
  context->entries     = pool16 + 1;

  context->j           = pool8 + 0;
  context->attr        = pool8 + 1;
  context->out_val     = pool8 + 2;
  context->error_flag  = pool8 + 3;
}

void get_read_dir_entry_task_context(read_dir_entry_task_context_t *context)
{
  context->Sector      = pool32 + 0;
  context->return_clus = pool32 + 1;
  context->max_sectors = pool32 + 2;
  context->Sector_num  = pool32 + 3;

  context->i           = pool16 + 0;
  context->entries     = pool16 + 1;
  context->entry       = pool16 + 2;

  context->j           = pool8 + 0;
  context->attr        = pool8 + 1;
  context->out_val     = pool8 + 2;
  context->error_flag  = pool8 + 3;
}

uint16_t get_directory_print_buffer_pos(void)
{
  return print_buffer_pos;
}

void set_directory_print_buffer_pos(uint16_t pos)
{
  print_buffer_pos = pos;
}

uint8_t *get_directory_print_buffer(void)
{
  return buf2;
}

void print_directory_init(uint32_t sector_number_in)
{
  print_directory_task_context_t context;
  get_print_directory_task_context(&context);

  *context.entries=0;
  *context.i=0;
  if (Drive_values.FATtype==FAT16)  // included for FAT16 compatibility
  {
    *context.max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
  }
  else
  {
     *context.max_sectors=Drive_values.SecPerClus;
  }
  *context.Sector=sector_number_in;
  *context.Sector_num=sector_number_in;
  *context.error_flag=Read_Sector(*context.Sector,Drive_values.BytesPerSec,buf1);
}

void read_dir_entry_init(uint16_t entry_in, uint32_t sector_number_in)
{
  read_dir_entry_task_context_t context;
  get_read_dir_entry_task_context(&context);

  *context.entries=0;
  *context.i=0;
  *context.return_clus=0;
  if (Drive_values.FATtype==FAT16)  // included for FAT16 compatibility
  {
     *context.max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
  }
  else
  {
     *context.max_sectors=Drive_values.SecPerClus;
  }
  *context.Sector=sector_number_in;
  *context.Sector_num=sector_number_in;
  *context.entry=entry_in;
  *context.error_flag=Read_Sector(*context.Sector,Drive_values.BytesPerSec,buf1);
}


uint8_t print_directory_task(uint16_t *entries_in)
{
  uint8_t temp8;
  print_directory_task_context_t context;
  uint8_t *values = buf1;
  get_print_directory_task_context(&context);

  temp8=read8(0 + (*context.i),values);  // read first byte to see if empty
  if((temp8!=0xE5)&&(temp8!=0x00))
  {
    *context.attr=read8(0x0b+ (*context.i),values);
    if(((*context.attr) & 0x0E)==0)   // if hidden, system or Vol_ID bit is set do not print
    {
      (*context.entries)++;
      print_buffer_pos += sprintf(buf2+print_buffer_pos, "%5d. ", *context.entries);  // print entry number with a fixed width specifier
      for(*context.j=0;*context.j<8;(*context.j)++)
      {
        *context.out_val=read8((*context.i)+(*context.j),values);   // print the 8 byte name
        print_buffer_pos += sprintf(buf2+print_buffer_pos, "%c", *context.out_val);
      }
      if(((*context.attr)&0x10)==0x10)  // indicates directory
      {
        for(*context.j=8;*context.j<11;(*context.j)++)
        {
          *context.out_val=read8((*context.i)+(*context.j),values);
          print_buffer_pos += sprintf(buf2+print_buffer_pos, "%c", *context.out_val);
        }
        print_buffer_pos += sprintf(buf2+print_buffer_pos, "[DIR]\n");
      }
      else       // print a period and the three byte extension for a file
      {
        print_buffer_pos += sprintf(buf2+print_buffer_pos, ".", *context.out_val);
        for(*context.j=8;*context.j<11;(*context.j)++)
        {
          *context.out_val=read8((*context.i) + (*context.j),values);
          print_buffer_pos += sprintf(buf2+print_buffer_pos, "%c", *context.out_val);
        }
        print_buffer_pos += sprintf(buf2+print_buffer_pos, "\r\n", *context.out_val);
      }
    }
  }
  *context.i=(*context.i)+32;  // next entry
  if(*context.i>510)
  {
    (*context.Sector)++;
    if(((*context.Sector)-(*context.Sector_num))<*context.max_sectors)
    {
      *context.error_flag=Read_Sector(*context.Sector,Drive_values.BytesPerSec,buf1);
      if(*context.error_flag!=no_errors)
      {
        *context.entries=0;   // no entries found indicates disk read error
        temp8=0;     // forces a function exit
      }
      *context.i=0;
    }
    else
    {
      *context.entries=(*context.entries)|more_entries;  // set msb to indicate more entries in another cluster
      temp8=0;                       // forces a function exit
    }
  }
  if(temp8==0)
  {
    *entries_in = *context.entries;
    return TASK_COMPLETE;
  }
  return TASK_RUNNING;
}

uint8_t read_dir_entry_task(uint32_t *cluster)
{
  read_dir_entry_task_context_t context;
  uint8_t temp8;
  uint8_t *values = buf1;
  get_read_dir_entry_task_context(&context);

  temp8=read8(0 + (*context.i),values);  // read first byte to see if empty
  if((temp8!=0xE5)&&(temp8!=0x00))
  {
    *context.attr=read8(0x0b + (*context.i), values);
    if(((*context.attr)&0x0E)==0)    // if hidden do not print
    {
      (*context.entries)++;
      if(*context.entries==*context.entry)
      {
        if(Drive_values.FATtype==FAT32)
        {
          *context.return_clus=read8(21 + (*context.i),values);
          *context.return_clus=(*context.return_clus)<<8;
          *context.return_clus|=read8(20 + (*context.i),values);
          *context.return_clus=(*context.return_clus)<<8;
        }
        *context.return_clus|=read8(27+ (*context.i),values);
        *context.return_clus=(*context.return_clus)<<8;
        *context.return_clus|=read8(26 + (*context.i),values);
        *context.attr=read8(0x0b + (*context.i),values);
        if((*context.attr)&0x10) (*context.return_clus)= (*context.return_clus) | directory_bit;
        temp8=0;    // forces a function exit
      }

    }
  }
  *context.i= (*context.i)+32;  // next entry
  if((*context.i)>510)
  {
    (*context.Sector)++;
    if(((*context.Sector)-(*context.Sector_num))<(*context.max_sectors))
    {
      *context.error_flag=Read_Sector(*context.Sector,Drive_values.BytesPerSec,values);
      if(*context.error_flag!=no_errors)
      {
        *context.return_clus=no_entry_found;
        temp8=0;
      }
      *context.i=0;
    }
    else
    {
      temp8=0;                       // forces a function exit
    }
  }

  if(temp8 != 0)
  {
    return TASK_RUNNING;
  }

  if(*context.return_clus==0)
  {
    *cluster=no_entry_found;
  }
  else
  {
    *cluster=*context.return_clus;
  }

  return TASK_COMPLETE;
 }


/***********************************************************************
DESC: Returns a pointer to the global structure Drive_values to export to other files
INPUT: void
RETURNS: Pointer to the structure Drive_values
CAUTION:
************************************************************************/

FS_values_t * Export_Drive_values(void)
{
   return &Drive_values;
}


/***********************************************************************
DESC: Prints all short file name entries for a given directory
INPUT: Starting Sector of the directory and the pointer to a
block of memory in xdata that can be used to read blocks from the SD card
RETURNS: uint16_t number of entries found in the directory
CAUTION: Supports FAT16, SD_shift must be set before using this function
************************************************************************/



// uint16_t  Print_Directory(uint32_t Sector_num, uint8_t xdata * array_in)
// {
//    uint32_t Sector, max_sectors;
//    uint16_t i, entries;
//    uint8_t temp8, j, attr, out_val, error_flag;
//    uint8_t * values;
//
//
//    values=array_in;
//    entries=0;
//    i=0;
//    if (Drive_values.FATtype==FAT16)  // included for FAT16 compatibility
//    {
//       max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
//    }
//    else
//    {
//       max_sectors=Drive_values.SecPerClus;
//    }
//    Sector=Sector_num;
//    error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
//    if(error_flag==no_errors)
//    {
//      do
//      {
//         temp8=read8(0+i,values);  // read first byte to see if empty
//         if((temp8!=0xE5)&&(temp8!=0x00))
// 	    {
// 	       attr=read8(0x0b+i,values);
// 		   if((attr&0x0E)==0)   // if hidden, system or Vol_ID bit is set do not print
// 		   {
// 		      entries++;
// 			  printf("%5d. ",entries);  // print entry number with a fixed width specifier
// 		      for(j=0;j<8;j++)
// 			  {
// 			     out_val=read8(i+j,values);   // print the 8 byte name
// 			     putchar(out_val);
// 			  }
//               if((attr&0x10)==0x10)  // indicates directory
// 			  {
// 			     for(j=8;j<11;j++)
// 			     {
// 			        out_val=read8(i+j,values);
// 			        putchar(out_val);
// 			     }
// 			     printf("[DIR]\n");
// 			  }
// 			  else       // print a period and the three byte extension for a file
// 			  {
// 			     putchar(0x2E);
// 			     for(j=8;j<11;j++)
// 			     {
// 			        out_val=read8(i+j,values);
// 			        putchar(out_val);
// 			     }
// 			     putchar(0x0d);
//                  putchar(0x0a);
// 			  }
// 		    }
// 		}
// 		    i=i+32;  // next entry
// 		    if(i>510)
// 		    {
// 			  Sector++;
//               if((Sector-Sector_num)<max_sectors)
// 			  {
//                  error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
// 			     if(error_flag!=no_errors)
// 			     {
// 			        entries=0;   // no entries found indicates disk read error
// 				    temp8=0;     // forces a function exit
// 			     }
//                  i=0;
// 			  }
// 			  else
// 			  {
// 			     entries=entries|more_entries;  // set msb to indicate more entries in another cluster
// 			     temp8=0;                       // forces a function exit
// 			  }
// 		    }
//
// 	  }while(temp8!=0);
// 	}
// 	else
// 	{
// 	   entries=0;    // no entries found indicates disk read error
// 	}
//    return entries;
//  }
//
//
// /***********************************************************************
// DESC: Uses the same method as Print_Directory to locate short file names,
//       but locates a specified entry and returns and cluster
// INPUT: Starting Sector of the directory, an entry number and a pointer to a
// block of memory in xdata that can be used to read blocks from the SD card
// RETURNS: uint32_t with cluster in lower 28 bits.  Bit 28 set if this is
//          a directory entry, clear for a file.  Bit 31 set for error.
// CAUTION:
// ************************************************************************/
//
// uint32_t Read_Dir_Entry(uint32_t Sector_num, uint16_t Entry, uint8_t xdata * array_in)
// {
//    uint32_t Sector, max_sectors, return_clus;
//    uint16_t i, entries;
//    uint8_t temp8, attr, error_flag;
//    uint8_t * values;
//
//    values=array_in;
//    entries=0;
//    i=0;
//    return_clus=0;
//    if (Drive_values.FATtype==FAT16)  // included for FAT16 compatibility
//    {
//       max_sectors=Drive_values.RootDirSecs;   // maximum sectors in a FAT16 root directory
//    }
//    else
//    {
//       max_sectors=Drive_values.SecPerClus;
//    }
//    Sector=Sector_num;
//    error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
//    if(error_flag==no_errors)
//    {
//      do
//      {
//         temp8=read8(0+i,values);  // read first byte to see if empty
//         if((temp8!=0xE5)&&(temp8!=0x00))
// 	    {
// 	       attr=read8(0x0b+i,values);
// 		   if((attr&0x0E)==0)    // if hidden do not print
// 		   {
// 		      entries++;
//               if(entries==Entry)
//               {
// 			    if(Drive_values.FATtype==FAT32)
//                 {
//                    return_clus=read8(21+i,values);
// 				   return_clus=return_clus<<8;
//                    return_clus|=read8(20+i,values);
//                    return_clus=return_clus<<8;
//                 }
//                 return_clus|=read8(27+i,values);
// 			    return_clus=return_clus<<8;
//                 return_clus|=read8(26+i,values);
// 			    attr=read8(0x0b+i,values);
// 			    if(attr&0x10) return_clus|=directory_bit;
//                 temp8=0;    // forces a function exit
//               }
//
// 		    }
// 		}
// 		    i=i+32;  // next entry
// 		    if(i>510)
// 		    {
// 			  Sector++;
// 			  if((Sector-Sector_num)<max_sectors)
// 			  {
//                  error_flag=Read_Sector(Sector,Drive_values.BytesPerSec,values);
// 			     if(error_flag!=no_errors)
// 			     {
// 			         return_clus=no_entry_found;
//                      temp8=0;
// 			     }
// 			     i=0;
// 			  }
// 			  else
// 			  {
// 			     temp8=0;                       // forces a function exit
// 			  }
// 		    }
//
// 	  }while(temp8!=0);
// 	}
// 	else
// 	{
// 	   return_clus=no_entry_found;
// 	}
// 	if(return_clus==0) return_clus=no_entry_found;
//    return return_clus;
//  }
