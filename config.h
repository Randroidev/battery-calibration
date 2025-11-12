#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions

// Relay pins (Note: Assumed to be active-low)
#define CHARGE_RELAY_PIN      7
#define DISCHARGE_RELAY_PIN   5

// LED pins
#define LED_DISCHARGE_RUN_PIN A0
#define LED_DISCHARGE_DONE_PIN A1
#define LED_CHARGE_RUN_PIN    A2
#define LED_CHARGE_DONE_PIN   A3

// SMBus
#define SMBUS_SDA_PIN A4
#define SMBUS_SCL_PIN A5
#define SMBUS_ADDRESS 0x0B // Standard Smart Battery address

// Constants
#define EEPROM_ADDR 0 // Start address for settings in EEPROM

#endif // CONFIG_H
