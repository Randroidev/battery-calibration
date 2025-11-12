#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "types.h"

// Global state variables defined in the main .ino file
extern TrainerState currentState;
extern bool processRunning;

TrainerState getCurrentState();
uint8_t getCyclesLeft();
bool isProcessRunning();

#endif // STATE_MACHINE_H
