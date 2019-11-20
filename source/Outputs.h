#ifndef _Outputs_H
#define _Outputs_H

#include "Main.H"

// Port Output Functions

void P0_output_bits(uint8_t Port_Data);
// void P1_output_bits(uint8_t Port_Data);
// void P2_output_bits(uint8_t Port_Data);
// void P3_output_bits(uint8_t Port_Data);

// Bit Output Functions

// void P0_clear_bit(uint8_t Bit_Data);
void P1_clear_bit(uint8_t Bit_Data);
void P2_clear_bit(uint8_t Bit_Data);
void P3_clear_bit(uint8_t Bit_Data);

// void P0_set_bit(uint8_t Bit_Data);
void P1_set_bit(uint8_t Bit_Data);
void P2_set_bit(uint8_t Bit_Data);
void P3_set_bit(uint8_t Bit_Data);


#endif