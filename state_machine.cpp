#include "state_machine.h"
#include "config.h"
#include "settings.h"
#include <Arduino.h>

static TrainerState currentState = IDLE;
static uint8_t cyclesLeft = 0;
static uint8_t totalCycles = 0;
static unsigned long pauseTimer = 0;
static bool processRunning = false;

// Function to set all indicators and relays off
void allOff() {
    digitalWrite(LED_DISCHARGE_PIN, LOW);
    digitalWrite(LED_PAUSE_DISCHARGE_PIN, LOW);
    digitalWrite(LED_CHARGE_PIN, LOW);
    digitalWrite(LED_PAUSE_CHARGE_PIN, LOW);
    digitalWrite(RELAY_DISCHARGE_PIN, LOW);
    digitalWrite(RELAY_CHARGE_PIN, LOW);
}

void initStateMachine() {
  pinMode(LED_DISCHARGE_PIN, OUTPUT);
  pinMode(LED_PAUSE_DISCHARGE_PIN, OUTPUT);
  pinMode(LED_CHARGE_PIN, OUTPUT);
  pinMode(LED_PAUSE_CHARGE_PIN, OUTPUT);
  pinMode(RELAY_DISCHARGE_PIN, OUTPUT);
  pinMode(RELAY_CHARGE_PIN, OUTPUT);
  allOff();
}

void startStopProcess(uint8_t cycles) {
  processRunning = !processRunning;
  if (processRunning) {
    if (cycles > 0) {
      cyclesLeft = cycles;
      totalCycles = cycles;
      currentState = DISCHARGING; // Start with discharge
    } else {
      processRunning = false; // Cannot start with 0 cycles
    }
  } else {
    currentState = IDLE;
    allOff();
  }
}

void updateStateMachine(const sbs_data_t& sbsData) {
  if (!processRunning) {
    currentState = IDLE;
    allOff();
    return;
  }

  Settings& s = getSettings();

  switch(currentState) {
    case IDLE:
      allOff();
      break;

    case DISCHARGING:
      allOff();
      digitalWrite(LED_DISCHARGE_PIN, HIGH);
      digitalWrite(RELAY_DISCHARGE_PIN, HIGH);
      // Check for end of discharge: Fully Discharged bit (5) in BatteryStatus
      if (sbsData.dataValid && (sbsData.batteryStatus & (1 << 5))) {
        currentState = PAUSE_AFTER_DISCHARGE;
        pauseTimer = millis();
      }
      break;

    case PAUSE_AFTER_DISCHARGE:
      allOff();
      digitalWrite(LED_PAUSE_DISCHARGE_PIN, HIGH);
      if (millis() - pauseTimer > (unsigned long)s.pauseAfterDischarge * 60000) {
        currentState = CHARGING;
      }
      break;

    case CHARGING:
      allOff();
      digitalWrite(LED_CHARGE_PIN, HIGH);
      digitalWrite(RELAY_CHARGE_PIN, HIGH);
      // Check for end of charge: Fully Charged bit (4) in BatteryStatus
      if (sbsData.dataValid && (sbsData.batteryStatus & (1 << 4))) {
        currentState = PAUSE_AFTER_CHARGE;
        pauseTimer = millis();
      }
      break;

    case PAUSE_AFTER_CHARGE:
      allOff();
      digitalWrite(LED_PAUSE_CHARGE_PIN, HIGH);
      if (millis() - pauseTimer > (unsigned long)s.pauseAfterCharge * 60000) {
        cyclesLeft--;
        if (cyclesLeft > 0) {
          currentState = DISCHARGING;
        } else {
          currentState = IDLE;
          processRunning = false;
        }
      }
      break;
  }
}

TrainerState getCurrentState() {
  return currentState;
}

uint8_t getCyclesLeft() {
    return cyclesLeft;
}

bool isProcessRunning() {
    return processRunning;
}
