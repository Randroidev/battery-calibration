#include "state_machine.h"
#include <Arduino.h>

// These are now defined in the main .ino file
extern TrainerState currentState;
extern bool processRunning;

TrainerState getCurrentState() {
  return currentState;
}

// This function is no longer used but kept for potential future use in logging.
uint8_t getCyclesLeft() {
    return 0; // Dummy value
}

bool isProcessRunning() {
    return processRunning;
}
