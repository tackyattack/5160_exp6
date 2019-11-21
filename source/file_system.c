#include <stdio.h>
#include "main.h"
#include "sector_reader.h"
#include "Directory_Functions_struct.h"
#include "UART.h"
#include "SPI.h"
#include "print_bytes.h"
#include "file_system.h"

// Private constants (outside doesn't need to see these)
#define MBR_BPB_sec_offset              (0x01C6)
#define BPB_BytesPerSector_offset       (0x000B)
#define BPB_SectorsPerCluster_offset    (0x000D)
#define BPB_RsvdSectorCount_offset      (0x000E)
#define BPB_NumberFATs_offset           (0x0010)
#define BPB_RootEntryCount_offset       (0x0011)
#define BPB_TotSec16_offset             (0x0013)
#define BPB_FATSz16_offset              (0x0016)
#define BPB_FATSz32_offset              (0x0024)
#define BPB_TotSec32_offset             (0x0020)
#define BPB_RootClus_offset             (0x002C)


uint8_t mount_drive(uint8_t xdata *xram_data_array)
{
  uint8_t temp8, NumFAT;
  uint16_t temp16, RootEntCnt, RsvdSecCnt, FATSz16, TotSec16;
  uint32_t ClusterCount, FATSz, TotSec, DataSec, MBR_RelSec, RootClus;
  uint8_t *data_array;
  FS_values_t *drive_values;
  data_array = xram_data_array; // cast xdata to uint8_t since all other functions don't use xdata

  drive_values = Export_Drive_values();


  // Find the BIOS parameter block (BPB)
  printf("Finding BIOS parameter block\n");
  Read_Sector(0, 512, data_array);
  temp8 = read8(0, data_array); // check first byte
  MBR_RelSec = 0;
  if((temp8!=0xEB) && (temp8!=0xE9))
  {
    // likely MBR
    // If we're in the MBR, then get the offset for the BPB
    MBR_RelSec = read32(MBR_BPB_sec_offset, data_array);
    // now read the start of BPB
    Read_Sector(MBR_RelSec, 512, data_array);
    temp8 = read8(0, data_array);
  }

  // now check if we've made it into the BPB by checking for the jump instruction
  if((temp8!=0xEB) && (temp8!=0xE9))
  {
    printf("error: could not locate BPB\n");
    return MOUNT_FAIL;
  }


  // ------- Read in BIOS parameter block -------
  drive_values->BytesPerSec = read16(BPB_BytesPerSector_offset, data_array);

  // set shift value (used by print directory function)
  drive_values->BytesPerSecShift = 0;

  temp16 = drive_values->BytesPerSec;
  // shift right by one bit position until we've moved it all the way out
  while(temp16 != 0x01)
  {
     drive_values->BytesPerSecShift++;
     temp16 = temp16>>1;
  }

  drive_values->SecPerClus  = read8(BPB_SectorsPerCluster_offset, data_array);

  RsvdSecCnt  = read16(BPB_RsvdSectorCount_offset, data_array);
  NumFAT      = read8(BPB_NumberFATs_offset, data_array);
  RootEntCnt  = read16(BPB_RootEntryCount_offset, data_array);
  FATSz16     = read16(BPB_FATSz16_offset, data_array);
  TotSec16    = read16(BPB_TotSec16_offset, data_array);
  RootClus    = read32(BPB_RootClus_offset, data_array);


  if(FATSz16 != 0)
  {
    FATSz = read16(BPB_FATSz16_offset, data_array);
  }
  else
  {
    FATSz = read32(BPB_FATSz32_offset, data_array);
  }

  if(TotSec16 != 0)
  {
    TotSec = TotSec16;
  }
  else
  {
    TotSec = read32(BPB_TotSec32_offset, data_array);
  }

  // Determine how many sectors are in the root directory
  drive_values->RootDirSecs = ((RootEntCnt * 32) + (drive_values->BytesPerSec - 1)) / (drive_values->BytesPerSec);

  // Determine how many sectors are data sectors
  DataSec = TotSec - (RsvdSecCnt + (NumFAT*FATSz) + drive_values->RootDirSecs);

  // Determine first sector of FAT
  drive_values->StartofFAT = RsvdSecCnt + MBR_RelSec;

  // Determine the first sector of the data area
  drive_values->FirstDataSec = drive_values->StartofFAT + (NumFAT*FATSz) + drive_values->RootDirSecs;

  // Determine count of clusters and FAT type
  ClusterCount = DataSec / drive_values->SecPerClus;
  if(ClusterCount < 4085)
  {
    // FAT12
    printf("error: FAT12 unsupported\n");
    return MOUNT_FAIL;
  }
  else if(ClusterCount < 65525)
  {
    // FAT16
    drive_values->FATtype = FAT16;
    printf("FAT16 detected\n");
    drive_values->FirstRootDirSec = drive_values->StartofFAT + (NumFAT*FATSz);
  }
  else
  {
    // FAT32
    drive_values->FATtype = FAT32;
    printf("FAT32 detected\n");
    drive_values->FirstRootDirSec = ((RootClus-2)*drive_values->SecPerClus) + drive_values->FirstDataSec;
  }

  return MOUNT_SUCCESS;
}

uint32_t first_sector(uint32_t cluster_num)
{
  FS_values_t *drive_values = Export_Drive_values();

  // Calculate starting sector of cluster
  if(cluster_num == 0)
  {
    return drive_values->FirstRootDirSec;
  }
  else
  {
    return ((cluster_num - 2)*drive_values->SecPerClus) + drive_values->FirstDataSec;
  }
}

uint32_t find_next_clus(uint32_t cluster_num, uint8_t xdata *xram_data_array)
{
  uint8_t idata *data_array;
  uint16_t idata FAToffset;
  uint32_t idata sector, next_clus;
  FS_values_t *drive_values = Export_Drive_values();

  data_array = xram_data_array; // cast xdata to uint8_t since all other functions don't use xdata

  // FATtype will be either 2 (FAT16) or 4 (FAT32)
  sector = ((cluster_num*drive_values->FATtype)/drive_values->BytesPerSec) + drive_values->StartofFAT;
  Read_Sector(sector, drive_values->BytesPerSec, data_array);

  // Determine the offset of the cluster within this sector
  FAToffset = (uint16_t)((cluster_num*drive_values->FATtype)%(drive_values->BytesPerSec));

  // Read the cluster entry from the FAT sector
  if(drive_values->FATtype == FAT32)
  {
    return (read32(FAToffset, data_array)&0x0FFFFFFF);
  }
  else if(drive_values->FATtype == FAT16)
  {
    next_clus = (uint32_t)(read16(FAToffset, data_array)&0xFFFF);

    // modify EOF to match for FAT32
    if(next_clus == 0xFFFF)
    {
      return 0x0FFFFFFF;
    }
    else
    {
      return next_clus;
    }
  }
  else
  {
    printf("Error: FAT type unsupported\n");
    return 0;
  }

  printf("Error: couldn't find next cluster\n");
  return 0;
}


// void print_file(uint32_t cluster_num, uint8_t xdata *xram_data_array)
// {
//    uint32_t base_sector, sector_offset;
//    uint8_t temp8;
//    uint8_t *data_array;
//    FS_values_t *drive_values = Export_Drive_values();
//    data_array = xram_data_array; // cast xdata to uint8_t since all other functions don't use xdata
//
//    sector_offset = 0; // start at first sector
//
//    printf(" **** Opening File ****\n");
//    printf("Press 'e' to exit\n");
//    printf("Press 'n' to print nextn sector\n");
//
//    do
//    {
//       // check if we need to find the first sector of the cluster
//       if(sector_offset == 0) base_sector = first_sector(cluster_num);
//
//       // read the sector in
//       Read_Sector((base_sector+sector_offset), drive_values->BytesPerSec, data_array);
//
//
//       printf(" **** cluster: %lu   sector: %lu ****\n", cluster_num, base_sector+sector_offset);
//       print_memory(data_array, drive_values->BytesPerSec);
//
//       sector_offset++; // go forward one sector
//
//       // check if we need to go to the next cluster
//       if(sector_offset == drive_values->SecPerClus)
//       {
//         cluster_num = find_next_clus(cluster_num, data_array);
//         sector_offset = 0;
//       }
//
//       // wait for user to choose an action
//       do
//       {
//         temp8 = getchar();
//       }while((temp8!='e') && (temp8!='n'));
//
//    }while(temp8 != 'e');
// }
