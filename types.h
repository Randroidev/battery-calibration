#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

// Structure to hold Smart Battery Data
typedef struct {
  uint16_t voltage;
  int16_t current;
  uint16_t temperature;
  uint16_t maxError;
  uint16_t remainingCapacity;
  uint16_t fullChargeCapacity;
  uint16_t chargingCurrent;
  uint16_t chargingVoltage;
  uint16_t cycleCount;
  uint16_t designVoltage;
  uint16_t designCapacity;
  uint16_t cellVoltage1;
  uint16_t cellVoltage2;
  uint16_t cellVoltage3;
  uint16_t cellVoltage4;
  uint16_t batteryMode;
  uint16_t batteryStatus;
  bool dataValid;
} sbs_data_t;

// Enum for the trainer's state machine
enum TrainerState {
  IDLE,
  DISCHARGING,
  PAUSE_AFTER_DISCHARGE,
  CHARGING,
  PAUSE_AFTER_CHARGE
};

#endif // TYPES_H
