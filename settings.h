#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

struct Settings {
  uint8_t cyclesCount;
  uint8_t devicePingTimeout;
  uint16_t pauseAfterCharge;
  uint16_t pauseAfterDischarge;
  uint8_t smbusReadTimeout;
  uint8_t demoChargeTime;
  uint8_t demoDischargeTime;
  uint8_t serialOutputTimeout;
};

void loadSettings();
void saveSettings();
void resetSettings();
Settings& getSettings();

#endif // SETTINGS_H
