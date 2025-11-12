#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#include "types.h"
#include "state_machine.h"

void printFullStatusToSerial(const sbs_data_t& data, int currentCycle = -1, int totalCycles = -1, unsigned long elapsedTime = 0);
void printKeyValueStatusToSerial(const sbs_data_t& data);

#endif // SERIAL_LOGGER_H
