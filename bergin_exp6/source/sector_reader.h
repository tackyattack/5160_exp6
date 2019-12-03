#ifndef _SECTOR_READER_H
#define _SECTOR_READER_H

#include "main.h"

//------- Public Constant definitions --------------------------------
#define SECTOR_READ_SUCCESS (0)
#define SECTOR_READ_FAIL    (1)


// ------ Public function prototypes -------------------------------
uint8_t read8(uint16_t offset, uint8_t *data_array);
uint16_t read16(uint16_t offset, uint8_t *data_array);
uint32_t read32(uint16_t offset, uint8_t *data_array);
uint8_t Read_Sector(uint32_t sector_number, uint16_t sector_size, uint8_t *data_array);

#endif
