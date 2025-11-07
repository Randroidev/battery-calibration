#ifndef SBS_HANDLER_H
#define SBS_HANDLER_H

#include <Arduino.h>
#include "state_machine.h" // Needed for trainer state

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

void initSBS();
bool readSBSData(sbs_data_t& data);
void generateDemoData(sbs_data_t& data, uint8_t state);
void printFullStatusToSerial(const sbs_data_t& data);

#endif // SBS_HANDLER_H
