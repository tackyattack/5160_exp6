#include "main.h"
#include "SDCard.h"
#include "PORT.h"
#include "player.h"
#include "Outputs.h"
#include "file_system.h"
#include "Directory_Functions_struct.h"
#include "sector_reader.h"
#include "SPI.h"
#include "LED_Control.h"

void player_state_machine_runner(void);


typedef enum {find_cluster_1, data_idle_1, load_buffer_1, data_send_1,
              find_cluster_2, data_idle_2, load_buffer_2, data_send_2} player_state_t;
player_state_t player_state;

#define BUF_SIZE (512)
extern uint8_t xdata buf1[512];
extern uint8_t xdata buf2[512];


// might be active low
#define DATA_REQ_ACTIVE   (DATA_REQ == 0)
#define DATA_REQ_INACTIVE (DATA_REQ != 0)

uint8_t buffer1_full;
uint8_t buffer2_full;


uint32_t base_sector;
uint32_t sector_offset;
uint32_t current_cluster;
static FS_values_t *drive_values;

// assuming buf1 is filled first before calling this
void init_player(uint32_t start_cluster)
{
  player_state    = data_send_1;
  buffer1_full    = 1;
  buffer2_full    = 0;
  drive_values    = Export_Drive_values();
  base_sector     = first_sector(start_cluster);
  sector_offset   = 0;
  current_cluster = start_cluster;
  while(1) player_state_machine_runner();
}

void load_sector(uint8_t *buf)
{
  // check if we need to find the first sector of the cluster
  if(sector_offset == 0) base_sector = first_sector(current_cluster);
  // read the sector in
  Read_Sector((base_sector+sector_offset), drive_values->BytesPerSec, buf);
  sector_offset++; // go forward one sector
}

void send_buffer(uint8_t *buf)
{
  uint8_t temp8;
  uint16_t i;
  LEDS_ON(Green_LED);
  P1_set_bit(STA_SPI_BIT_EN_bit);
  for(i=0; i<BUF_SIZE; i++) SPI_Transfer(buf[i], &temp8);
  P1_clear_bit(STA_SPI_BIT_EN_bit);
  LEDS_OFF(Green_LED);
}

void player_state_machine_runner(void)
{

  switch(player_state)
  {
    case find_cluster_1:
      current_cluster = find_next_clus(current_cluster, buf1);
      sector_offset = 0;
      if(DATA_REQ_INACTIVE) player_state = data_idle_2;
      break;
    case data_idle_1:
      if(DATA_REQ_ACTIVE) player_state = data_send_1;
      break;
    case load_buffer_1:
      if(!buffer1_full)
      {
        load_sector(buf1);
        buffer1_full = 1;
      }
      break;
    case data_send_1:
      if(buffer1_full)
      {
        send_buffer(buf1);
        buffer1_full = 0;
      }
      // check if we hit the end of this cluster
      if(sector_offset == drive_values->SecPerClus)
      {
        player_state = find_cluster_2;
      }
      else
      {
        if((DATA_REQ_INACTIVE && !buffer2_full) || (!buffer1_full && !buffer2_full)) player_state = load_buffer_2;
      }
      break;
    case find_cluster_2:
      current_cluster = find_next_clus(current_cluster, buf2);
      sector_offset = 0;
      if(DATA_REQ_INACTIVE) player_state = data_idle_1;
      break;
    case data_idle_2:
      if(DATA_REQ_ACTIVE) player_state = data_send_2;
      break;
    case load_buffer_2:
      if(!buffer2_full)
      {
        load_sector(buf2);
        buffer2_full = 1;
      }
      break;
    case data_send_2:
      if(buffer2_full)
      {
        send_buffer(buf2);
        buffer2_full = 0;
      }
      // check if we hit the end of this cluster
      if(sector_offset == drive_values->SecPerClus)
      {
        player_state = find_cluster_2;
      }
      else
      {
        if((DATA_REQ_INACTIVE && !buffer1_full) || (!buffer1_full && !buffer2_full)) player_state = load_buffer_1;
      }
      break;
  }
}
