#ifndef _SPI_H
#define _SPI_H

#include "Main.H"

//------- Public Constant definitions --------------------------------
// error values
#define init_okay (0)
#define no_errors (0)
#define SPI_ERROR (0)
#define illegal_clockrate (0x0F)
#define spi_timeout_error (0x80)


// ------ Public function prototypes -------------------------------
uint8_t SPI_Master_Init(uint32_t clock_rate);
uint8_t SPI_Transfer(uint8_t data_input, uint8_t * data_output_p);
void SPI_Select_Set(uint8_t Output_Pin);
void SPI_Select_Clear(uint8_t Output_Pin);

#endif