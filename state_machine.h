#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// sbs_data_t is defined in sbs_handler.h
#include "sbs_handler.h"

enum TrainerState {
  IDLE,
  DISCHARGING,
  PAUSE_AFTER_DISCHARGE,
  CHARGING,
  PAUSE_AFTER_CHARGE
};

void initStateMachine();
void startStopProcess(uint8_t cycles);
void updateStateMachine(const sbs_data_t& sbsData);
TrainerState getCurrentState();
uint8_t getCyclesLeft();
bool isProcessRunning();

#endif // STATE_MACHINE_H
