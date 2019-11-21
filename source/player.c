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
#include <stdio.h>

#define BUF_SIZE (512)
extern uint8_t xdata buf1[BUF_SIZE];
extern uint8_t xdata buf2[BUF_SIZE];


typedef enum {find_cluster_1, data_idle_1, load_buffer_1, data_send_1,
              find_cluster_2, data_idle_2, load_buffer_2, data_send_2, song_end} player_state_t;
player_state_t player_state;


// might be active low
#define DATA_REQ_ACTIVE   (DATA_REQ == 0)
#define DATA_REQ_INACTIVE (DATA_REQ != 0)

uint8_t idata *buffer1_flag;
uint8_t idata *buffer2_flag;

#define BUFFER_EMPTY (1)
#define BUFFER_FULL  (0)

uint16_t idata *index1;
uint16_t idata *index2;


uint32_t idata *base_sector;
uint32_t idata *sector_offset;
uint32_t idata *current_cluster;
static FS_values_t *drive_values;

void set_data_pool_player(uint32_t *p32, uint16_t *p16, uint8_t *p8)
{
  base_sector = p32 + 0;
  sector_offset = p32 + 1;
  current_cluster = p32 + 1;

  index1 = p16 + 0;
  index2 = p16 + 1;

  buffer1_flag = p8 + 0;
  buffer2_flag = p8 + 1;

}

void load_sector(uint8_t *buf)
{
  // check if we need to find the first sector of the cluster
  if(*sector_offset == 0) *base_sector = first_sector_i(*current_cluster);
  // read the sector in
  Read_Sector_i(((*base_sector) + (*sector_offset)), drive_values->BytesPerSec, buf);
  (*sector_offset)++; // go forward one sector
}

void init_player(uint32_t start_cluster)
{
  player_state    = data_send_1;
  *buffer1_flag    = BUFFER_FULL;
  *buffer2_flag    = BUFFER_EMPTY;
  *index1          = 0;
  *index2          = 0;
  *base_sector     = first_sector(start_cluster);
  *sector_offset   = 0;
  *current_cluster = start_cluster;
  load_sector(buf1);
  //while(player_state_machine_runner() == PLAYER_RUNNING);
}

// flush out as much as the decoder will take
uint8_t send_buffer(uint8_t *buf, uint16_t *index, uint8_t *buffer_flag)
{
  uint8_t temp8;
  P1_set_bit(STA_SPI_BIT_EN_bit);
  for(;((*index)<BUF_SIZE)&&(DATA_REQ_ACTIVE); (*index)++) SPI_Transfer(buf[(*index)], &temp8);
  P1_clear_bit(STA_SPI_BIT_EN_bit);
  // check if we flushed the entire buffer
  // if there's still more to go, then the state machine will know since the buffer didn't
  // get set empty and it can continue sending when it's ready again
  if((*index) == BUF_SIZE)
  {
    (*index)=0;
    (*buffer_flag)=BUFFER_EMPTY;
    return 1;
  }
  return 0;
}

uint8_t player_state_machine_runner(void)
{
  switch(player_state)
  {
    case find_cluster_1:
      LED_number(1);
      *current_cluster = find_next_clus_i(*current_cluster, buf1);
      if(*current_cluster != FAT_END_OF_FILE_MARKER)
      {
        *sector_offset = 0;
        player_state = data_idle_2;
      }
      else
      {
        player_state = song_end;
      }
      break;
    case data_idle_1:
      LED_number(2);
      if(DATA_REQ_ACTIVE) player_state = data_send_1;
      break;
    case load_buffer_1:
      LED_number(3);
      // check if we hit the end of this cluster
      // if so, load the FAT sector into the open buffer, but don't indicate that it's
      // filled since it isn't data
      if(sector_offset == drive_values->SecPerClus)
      {
        player_state = find_cluster_1;
      }
      else
      {
        load_sector(buf1);
        *buffer1_flag = BUFFER_FULL;
        player_state = data_idle_2;
      }
      break;
    case data_send_1:
      LED_number(4);
      if(DATA_REQ_ACTIVE)
      {
        if(*buffer1_flag == BUFFER_FULL) send_buffer(buf1, index1, buffer1_flag);
      }

      if((DATA_REQ_INACTIVE && (*buffer2_flag==BUFFER_EMPTY))
          || ((*buffer1_flag==BUFFER_EMPTY) && (*buffer2_flag==BUFFER_EMPTY)))
      {
        // if we're inactive and buffer2 is empty, go fill it
        // OR, if both buffers are empty, go fill buffer2
        player_state = load_buffer_2;
      }
      else if((*buffer1_flag==BUFFER_EMPTY) && DATA_REQ_ACTIVE)
      {
        // decoder wants data, but buffer1 was empty, so go check buffer2
        player_state = data_send_2;
      }
      else if(DATA_REQ_INACTIVE)
      {
        // if nothing else needs to be taken care of above, just go back to idle
        player_state = data_idle_1;
      }
      break;
    case find_cluster_2:
      LED_number(5);
      *current_cluster = find_next_clus_i(*current_cluster, buf2);
      if(*current_cluster != FAT_END_OF_FILE_MARKER)
      {
        *sector_offset = 0;
        player_state = data_idle_1;
      }
      else
      {
        player_state = song_end;
      }
      break;
    case data_idle_2:
      LED_number(6);
      if(DATA_REQ_ACTIVE) player_state = data_send_2;
      break;
    case load_buffer_2:
      LED_number(7);
      // check if we hit the end of this cluster
      // if so, load the FAT sector into the open buffer, but don't indicate that it's
      // filled since it isn't data
      if(*sector_offset == drive_values->SecPerClus)
      {
        //check if we hit the end of this cluster
        player_state = find_cluster_2;
      }
      else
      {
        load_sector(buf2);
        *buffer2_flag = BUFFER_FULL;
        player_state = data_idle_1;
      }
      break;
    case data_send_2:
      LED_number(8);
      if(DATA_REQ_ACTIVE)
      {
        if(*buffer2_flag == BUFFER_FULL) send_buffer(buf2, index2, buffer2_flag);
      }

      // determine which state to go to next
      if((DATA_REQ_INACTIVE && (*buffer1_flag==BUFFER_EMPTY))
        || ((*buffer1_flag==BUFFER_EMPTY) && (*buffer2_flag==BUFFER_EMPTY)))
      {
        // if we're inactive and buffer1 is empty, go fill it
        // OR, if both buffers are empty, go fill buffer1
        player_state = load_buffer_1;
      }
      else if((*buffer2_flag==BUFFER_EMPTY) && DATA_REQ_ACTIVE)
      {
        // decoder wants data, but buffer2 was empty, so go check buffer1
        player_state = data_send_1;
      }
      else if(DATA_REQ_INACTIVE)
      {
        // if nothing else needs to be taken care of above, just go back to idle
        player_state = data_idle_2;
      }
      break;
    case song_end:
      // do nothing -- outside functions could watch for this and do something while we stay here
      LED_number(9);
      break;
  }

  if(player_state == song_end) return PLAYER_COMPLETE;
  return PLAYER_RUNNING;
}
