#include "sEOS.h"
#include "Directory_Functions_struct.h"
#include "UART.h"
#include "file_system.h"
#include "player.h"
#include "LED_Control.h"
#include <stdio.h>

extern uint8_t xdata buf1[512];
extern uint8_t xdata buf2[512];

typedef enum {os_init, os_idle, print_buffer, recv_input, run_print_dir_task, run_read_dir_entry_task,
              start_song, play_song_task} seos_state_t;
seos_state_t seos_state;

uint16_t  temp16, cwd_entries;
uint8_t   temp8;
uint32_t  entry_num;
uint32_t  cwd, clus;
uint8_t   tens = 0;

bit print_dir_task_running = 0;
bit read_dir_entry_task_running = 0;

void os_loop()
{
  switch(seos_state)
  {
    case os_init:
      cwd = Export_Drive_values()->FirstRootDirSec; // start out at root directory
      print_directory_init(cwd);
      print_dir_task_running=1;
      entry_num = 0;
      seos_state = os_idle;
      break;
    case os_idle:
      LED_number(1);
      temp8 = UART_Receive_Non_Blocking();
      if(temp8 != 0)
      {
        seos_state = recv_input;
      }
      if(print_dir_task_running) seos_state = run_print_dir_task;
      if(read_dir_entry_task_running) seos_state = run_read_dir_entry_task;
      if(get_directory_print_buffer_pos() > 0)
      {
        seos_state = print_buffer;
      }
      break;
    case recv_input:
      LED_number(2);
      if(temp8 == '\r')
      {
        if(entry_num <= cwd_entries)
        {
        read_dir_entry_init(entry_num, cwd);
        read_dir_entry_task_running = 1;
        }
        else
        {
          set_directory_print_buffer_pos
          (
            get_directory_print_buffer_pos()
            + sprintf(get_directory_print_buffer()+get_directory_print_buffer_pos(), "\nError: invalid entry choice. Try again:\n")
          );
        }
        tens = 0;
        seos_state = os_idle;
        entry_num = 0;
      }
      else if((temp8 >= '0') && (temp8 <= '9'))
      {
        putchar(temp8);
        entry_num = temp8-'0' + entry_num*10;
        seos_state = os_idle;
      }
      else
      {
        seos_state = os_idle;
      }
      break;
    case print_buffer:
      LED_number(3);
      putchar(get_directory_print_buffer()[temp16]);
      temp16++;
      if(temp16 > get_directory_print_buffer_pos())
      {
        set_directory_print_buffer_pos(0);
        temp16=0;
      }
      seos_state = os_idle;
      break;
    case run_print_dir_task:
      LED_number(4);
      if(print_directory_task(&cwd_entries) == TASK_RUNNING)
      {
        temp16 = 0;
        seos_state = print_buffer;
      }
      else
      {
        print_dir_task_running = 0;
        seos_state = print_buffer;
        set_directory_print_buffer_pos
        (
          get_directory_print_buffer_pos()
          + sprintf(get_directory_print_buffer()+get_directory_print_buffer_pos(), "\nEnter an entry number:\n")
        );
      }
      break;
    case run_read_dir_entry_task:
      LED_number(5);
      if(read_dir_entry_task(&clus) == TASK_RUNNING)
      {
        // go back to idle since we're still going
        seos_state = os_idle;
      }
      else
      {
        read_dir_entry_task_running = 0;
        if(clus & directory_bit)
        {
          seos_state = os_idle;
          clus &= 0x0FFFFFFF; // mask off upper four bits to print another directory
          cwd = first_sector(clus);
          print_directory_init(cwd);
          print_dir_task_running=1;
        }
        else
        {
          seos_state = start_song;
        }
      }
      break;
    case start_song:
      LED_number(6);
      init_player(clus);
      seos_state = play_song_task;
      break;
    case play_song_task:
      LED_number(7);
      if(player_state_machine_runner() == PLAYER_RUNNING)
      {
        seos_state = play_song_task;
      }
      else
      {
        print_directory_init(cwd);
        print_dir_task_running=1;
        seos_state = os_idle;
      }
      break;
  }
}

void sEOS_ISR(void) interrupt 5 using 1
{
  TF2 = 0; // clear timer flag that caused this interrupt
  os_loop();
}

void sEOS_init(uint8_t interval_ms)
{
  uint16_t preload;
  seos_state = os_init;
  //while(1) os_loop();

  T2CON=0;
  preload=(65536-((OSC_FREQ*interval_ms)/(OSC_PER_INST*1000UL)));
  RCAP2H=(uint8_t)preload/256;
  RCAP2L=(uint8_t)preload%256;
  TF2=0; // Clear overflow
  ET2=1; // Enable Timer 2 interrupt
  EA=1;  // Set Global Interrupt Enable
  TR2=1; // Start Timer Running
}
