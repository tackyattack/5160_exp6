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
#include "song_navigator.h"
#include "switch.h"
#include "LCD_Routines.h"
#include <stdio.h>

#define ERROR_FINDING_FILE (1)
#define FOUND_FILE (1)

extern xdata uint8_t buf1[512];

uint8_t code LCD_str_instructions[]="1:nxt2:prv 3:sel";
uint8_t code LCD_str_play_instructions[]="playing.. 3:stop";

// NOTE: using volatile ensures that the compiler doesn't optimize register access
//       This allows us to have "thread" safe switches and keep the ISR short to avoid
//       overflowing the memory with extra stack space
volatile SW_values_t idata SW1_val, SW2_val, SW3_val, SW4_val;
static uint32_t idata cwd;
//static uint32_t idata clus;

void switch_ISR(void) interrupt Timer_2_Overflow
{
  TF2 = 0;
  read_switch(&SW1_val);
  read_switch(&SW2_val);
  read_switch(&SW3_val);
  read_switch(&SW4_val);
}

void setup_switches(uint16_t interval_ms)
{
  uint16_t preload = (uint16_t)(65536 - ((OSC_FREQ*interval_ms)/(OSC_PER_INST*1000UL)));
  switch_init(&SW1_val, 1<<0);
  switch_init(&SW2_val, 1<<1);
  switch_init(&SW3_val, 1<<2);
  switch_init(&SW4_val, 1<<3);

  T2CON=0;
  RCAP2H=(uint8_t)preload/256;
  RCAP2L=(uint8_t)preload%256;
  TF2=0;
  ET2=1;
  EA=1;
  TR2=1;
}

uint8_t get_prev_valid_entry(uint32_t cd, uint16_t *cur_entry_p, entry_t *entry_p)
{
  entry_t temp_entry;
  uint8_t entry_status;
  uint8_t i;
  uint8_t found_prev = 0;
  if((*cur_entry_p) < 1) return ERROR_FINDING_FILE;
  (*cur_entry_p)--;

  do
  {

    entry_status = get_entry(cd, (*cur_entry_p), &temp_entry, buf1);
    // if hidden, system or Vol_ID bit is set, keep looking
    if(entry_status != NO_MORE_ENTRIES)
    {
      if((temp_entry.entry_attr&(HIDDEN_BIT|SYSTEM_BIT|VOLUME_ID_BIT)) == 0)
      {
        found_prev = 1;
      }
      else
      {
        if((*cur_entry_p) > 0) (*cur_entry_p)--;
      }
    }
  }
  while((!found_prev) && (entry_status != NO_MORE_ENTRIES) && ((*cur_entry_p) > 0));

  if(!found_prev) return ERROR_FINDING_FILE;

  entry_p->entry_attr = temp_entry.entry_attr;
  entry_p->entry_number = temp_entry.entry_number;
  entry_p->cluster = temp_entry.cluster;
  for(i = 0; i < 12; i++) entry_p->name[i] = temp_entry.name[i];

  return FOUND_FILE;
}


uint8_t get_next_valid_entry(uint32_t cd, uint16_t *cur_entry_p, entry_t *entry_p)
{
  entry_t temp_entry;
  uint8_t entry_status;
  uint8_t i;
  uint8_t found_next = 0;
  (*cur_entry_p)++;
  entry_status = get_entry(cd, (*cur_entry_p), &temp_entry, buf1);
  if(entry_status == NO_MORE_ENTRIES)
  {
    // wrap back around
    (*cur_entry_p) = 0;
  }
  do
  {
    entry_status = get_entry(cd, (*cur_entry_p), &temp_entry, buf1);
    // if hidden, system or Vol_ID bit is set, keep looking
    if(entry_status != NO_MORE_ENTRIES)
    {
      if((temp_entry.entry_attr&(HIDDEN_BIT|SYSTEM_BIT|VOLUME_ID_BIT)) == 0)
      {
        found_next = 1;
      }
      else
      {
        (*cur_entry_p)++;
      }
    }

  }
  while((!found_next) && (entry_status != NO_MORE_ENTRIES));

  if(!found_next) return ERROR_FINDING_FILE;

  entry_p->entry_attr = temp_entry.entry_attr;
  entry_p->entry_number = temp_entry.entry_number;
  entry_p->cluster = temp_entry.cluster;
  for(i = 0; i < 12; i++) entry_p->name[i] = temp_entry.name[i];

  return FOUND_FILE;
}

void song_navigator_runner(void)
{
  entry_t idata entry;
  uint8_t temp8;
  uint16_t i;
  uint16_t cur_entry = 0;

  temp8 = get_next_valid_entry(cwd, &cur_entry, &entry);
  for(i = 0; i < 12; i++) printf("%c", entry.name[i]);
  LCD_Print(line1, 12, entry.name);
  LCD_Print(line2, 16, LCD_str_instructions);

  while(1)
  {
    // next
    if(SW1_val.SW_state == pressed)
    {
      printf("next\n");
      SW1_val.SW_state = held;
      temp8 = get_next_valid_entry(cwd, &cur_entry, &entry);
      for(i = 0; i < 12; i++) printf("%c", entry.name[i]);
      LCD_Print(line1, 12, entry.name);
    }

    // prev
    if(SW2_val.SW_state == pressed)
    {
      printf("previous\n");
      SW2_val.SW_state = held;
      temp8 = get_prev_valid_entry(cwd, &cur_entry, &entry);
      for(i = 0; i < 12; i++) printf("%c", entry.name[i]);
      LCD_Print(line1, 12, entry.name);
    }

    // select
    if(SW3_val.SW_state == pressed)
    {
      printf("select: %d\n", cur_entry);
      SW3_val.SW_state = held;
      if(entry.cluster & directory_bit)
      {
        entry.cluster &= 0x0FFFFFFF; // mask off upper four bits to print another directory
        cwd = first_sector(entry.cluster);
        cur_entry = 0;
        temp8 = get_next_valid_entry(cwd, &cur_entry, &entry);
        for(i = 0; i < 12; i++) printf("%c", entry.name[i]);
        LCD_Print(line1, 12, entry.name);
      }
      else
      {
        init_player(entry.cluster);
        printf("playing\n");
        LCD_Print(line2, 16, LCD_str_play_instructions);
        while((player_state_machine_runner() == PLAYER_RUNNING) && (SW3_val.SW_state != pressed));
        if(SW3_val.SW_state == pressed) SW3_val.SW_state = held;
        LCD_Print(line2, 16, LCD_str_instructions);
      }
    }
  }
}

void song_navigator_init(uint32_t root_dir)
{
  cwd = root_dir;
  setup_switches(MS_PER_TICK);
}
