#include "state_machine.h"
#include "config.h"
#include "settings.h"
#include <Arduino.h>

static TrainerState currentState = IDLE;
static uint8_t cyclesLeft = 0;
static uint8_t totalCycles = 0;
static unsigned long phaseTimer = 0;
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
      currentState = DISCHARGING;
      phaseTimer = millis();
    } else {
      processRunning = false;
    }
  } else {
    currentState = IDLE;
    allOff();
  }
}

void updateStateMachine(const sbs_data_t& sbsData, bool isDemoMode) {
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
      if ((isDemoMode && millis() - phaseTimer > (unsigned long)s.demoDischargeTime * 1000) ||
          (!isDemoMode && sbsData.dataValid && (sbsData.batteryStatus & (1 << 5)))) {
        currentState = PAUSE_AFTER_DISCHARGE;
        phaseTimer = millis();
      }
      break;

    case PAUSE_AFTER_DISCHARGE:
      allOff();
      digitalWrite(LED_PAUSE_DISCHARGE_PIN, HIGH);
      if (millis() - phaseTimer > (isDemoMode ? 3000 : (unsigned long)s.pauseAfterDischarge * 60000)) {
        currentState = CHARGING;
        phaseTimer = millis();
      }
      break;

    case CHARGING:
      allOff();
      digitalWrite(LED_CHARGE_PIN, HIGH);
      digitalWrite(RELAY_CHARGE_PIN, HIGH);
      if ((isDemoMode && millis() - phaseTimer > (unsigned long)s.demoChargeTime * 1000) ||
          (!isDemoMode && sbsData.dataValid && (sbsData.batteryStatus & (1 << 4)))) {
        currentState = PAUSE_AFTER_CHARGE;
        phaseTimer = millis();
      }
      break;

    case PAUSE_AFTER_CHARGE:
      allOff();
      digitalWrite(LED_PAUSE_CHARGE_PIN, HIGH);
      if (millis() - phaseTimer > (isDemoMode ? 3000 : (unsigned long)s.pauseAfterCharge * 60000)) {
        cyclesLeft--;
        if (cyclesLeft > 0) {
          currentState = DISCHARGING;
          phaseTimer = millis();
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
