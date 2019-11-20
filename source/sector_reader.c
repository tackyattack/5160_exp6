#include "main.h"
#include "SDCard.h"
#include "PORT.h"
#include "SPI.h"
#include "sector_reader.h"

uint8_t Read_Sector(uint32_t sector_number, uint16_t sector_size, uint8_t *data_array)
{
  uint8_t SD_type, error_flag;
  SD_type = Return_SD_Card_Type();
  SPI_Select_Clear(SD_Card_Select);
  error_flag = SEND_COMMAND(17, (sector_number<<SD_type));
  if(error_flag == no_errors) read_block(sector_size, data_array);
  SPI_Select_Set(SD_Card_Select);

  if(error_flag != no_errors) return SECTOR_READ_FAIL;
  return SECTOR_READ_SUCCESS;
}

// Reads single byte
uint8_t read8(uint16_t offset, uint8_t *data_array)
{
  offset = offset & 0x1FF; // mask off for safety
  return data_array[offset];
}

// Reads little endian 16b value
uint16_t read16(uint16_t offset, uint8_t *data_array)
{
  uint16_t ret_val;
  offset = offset & 0x1FF; // mask off for safety
  ret_val = 0;
  ret_val = data_array[offset + 1];              // MSB
  ret_val = ret_val << 8;                     // shift to correct spot
  ret_val = ret_val | data_array[offset];        // LSB
  return ret_val;
}

// Reads little endian 32b value
uint32_t read32(uint16_t offset, uint8_t *data_array)
{
  uint32_t ret_val;
  uint8_t temp, i;
  offset = offset & 0x1FF; // mask off for safety
  ret_val = 0;
  for(i=0; i<4; i++)
  {
    temp = data_array[offset + (3-i)];
    ret_val = ret_val<<8;
    ret_val |= temp;
  }
  return ret_val;
}
