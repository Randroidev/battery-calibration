#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#include "types.h"

void logHumanReadable(const sbs_data_t& data, TrainerState state, uint8_t cyclesLeft);
void logMachineReadable(const sbs_data_t& data, TrainerState state, uint8_t cyclesLeft);

#endif // SERIAL_LOGGER_H
