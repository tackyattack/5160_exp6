#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "main.h"

//------- Public Constant definitions --------------------------------
#define MOUNT_SUCCESS (0)
#define MOUNT_FAIL (1)

#define FAT_END_OF_FILE_MARKER (0x0FFFFFFF)


// ------ Public function prototypes -------------------------------
uint8_t mount_drive(uint8_t xdata *xram_data_array);
void print_file(uint32_t cluster_num, uint8_t xdata *xram_data_array);
uint32_t first_sector(uint32_t cluster_num);
uint32_t find_next_clus(uint32_t cluster_num, uint8_t xdata *xram_data_array);
uint32_t first_sector(uint32_t cluster_num);

// reentrant functions
uint32_t find_next_clus_i(uint32_t cluster_num, uint8_t xdata *xram_data_array);
uint32_t first_sector_i(uint32_t cluster_num);

#endif
