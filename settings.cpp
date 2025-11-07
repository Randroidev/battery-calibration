#include "settings.h"
#include "config.h"
#include <EEPROM.h>

static Settings currentSettings;

// Default values
const Settings defaultSettings = {
  2,   // cyclesCount
  0,   // devicePingTimeout
  15,  // pauseAfterCharge
  15,  // pauseAfterDischarge
  5,   // smbusReadTimeout
  3,   // demoChargeTime
  3,   // demoDischargeTime
  5    // serialOutputTimeout
};

void loadSettings() {
  EEPROM.get(EEPROM_ADDR, currentSettings);
  // A simple check to see if EEPROM was initialized
  if (currentSettings.cyclesCount > 99) { // Assuming cyclesCount will never be legitimately > 99
    resetSettings();
  }
}

void saveSettings() {
  EEPROM.put(EEPROM_ADDR, currentSettings);
}

void resetSettings() {
  currentSettings = defaultSettings;
  saveSettings();
}

Settings& getSettings() {
  return currentSettings;
}
