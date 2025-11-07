#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "sbs_handler.h"

void initDisplay();
void drawMainMenu(int8_t selectedItem);
void drawCyclesScreen(const sbs_data_t& sbsData, uint8_t cyclesLeft, uint8_t cyclesTotal, bool isRunning);
void drawDevicePingScreen(const sbs_data_t& sbsData, bool isConnected);
void drawSettingsScreen(int8_t selectedItem, bool editMode);
void updateDisplay(); // To handle periodic updates

#endif // DISPLAY_H
