#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "types.h"

void initStateMachine();
void startStopProcess(uint8_t cycles);
void updateStateMachine(const sbs_data_t& sbsData);
TrainerState getCurrentState();
uint8_t getCyclesLeft();
bool isProcessRunning();

#endif // STATE_MACHINE_H
