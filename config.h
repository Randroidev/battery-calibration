#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
// LEDs
#define LED_DISCHARGE_PIN A0
#define LED_PAUSE_DISCHARGE_PIN A1
#define LED_CHARGE_PIN A2
#define LED_PAUSE_CHARGE_PIN A3

// Encoder
#define ENC_KEY_PIN 2
#define ENC_S1_PIN 3
#define ENC_S2_PIN 4

// Relays
#define RELAY_DISCHARGE_PIN 5
#define RELAY_CHARGE_PIN 7

// Display (ICSP connection is handled by the library)
#define TFT_CS_PIN 10
#define TFT_DC_PIN 9
#define TFT_RST_PIN 8 // Optional, can be connected to Arduino's RST

// SMBus
#define SMBUS_SDA_PIN A4
#define SMBUS_SCL_PIN A5
#define SMBUS_ADDRESS 0x0B // Standard Smart Battery address

// Constants
#define EEPROM_ADDR 0 // Start address for settings in EEPROM

#endif // CONFIG_H
