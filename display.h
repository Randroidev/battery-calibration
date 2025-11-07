#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"
#include <Adafruit_ST7789.h>

void initDisplay();
void drawMainMenu(int8_t selectedItem);
void drawCyclesScreen(const sbs_data_t& sbsData, uint8_t cyclesLeft, uint8_t cyclesTotal, bool isRunning);
void drawDevicePingScreen(const sbs_data_t& sbsData, bool isConnected);
void drawSettingsScreen(int8_t selectedItem, bool editMode);
void updateDisplay(); // To handle periodic updates

#endif // DISPLAY_H
