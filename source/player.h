#ifndef _PLAYER_H
#define _PLAYER_H

#include "main.h"

//------- Public Constant definitions --------------------------------
#define PLAYER_RUNNING     (0)
#define PLAYER_COMPLETE    (1)

// ------ Public function prototypes -------------------------------
void init_player(uint32_t start_cluster, uint8_t xdata *buf);
uint8_t player_state_machine_runner(void);

#endif
