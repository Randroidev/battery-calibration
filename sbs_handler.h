#ifndef SBS_HANDLER_H
#define SBS_HANDLER_H

#include "types.h"

void initSBS();
bool readSBSData(sbs_data_t& data);
void generateDemoData(sbs_data_t& data, TrainerState state);

#endif // SBS_HANDLER_H
