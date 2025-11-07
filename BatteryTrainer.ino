/*
 * Battery Trainer Sketch
 * Author: Jules
 * Date: 2025-11-07
 *
 * Description:
 * This sketch provides a menu-driven interface to test and cycle Smart Batteries.
 * It features several modes:
 * - Cycles: Automatically charge and discharge a battery for a set number of cycles.
 * - Device Ping: Read and display battery data continuously.
 * - Settings: Configure timings, cycle counts, and other parameters.
 * - Demo: Simulate the cycling process without a battery connected.
 *
 * Hardware:
 * - Arduino Nano
 * - ILI9341 TFT Display (240x320)
 * - Rotary Encoder with push button
 * - 4x LEDs for status indication
 * - 2x Relays for charge/discharge control
 * - SMBus connection to a Smart Battery
 */

#include "config.h"
#include "settings.h"
#include "display.h"
#include "sbs_handler.h"
#include "state_machine.h"
#include "GyverEncoder.h"

// =================== GLOBAL OBJECTS ===================
Encoder enc(ENC_S1_PIN, ENC_S2_PIN, ENC_KEY_PIN, ENC_TYPE_STEP2);
sbs_data_t sbsData; // Struct to hold all battery data

// =================== PROGRAM STATE ===================
enum Screen {
  MAIN_MENU,
  CYCLES_SCREEN,
  DEVICE_PING_SCREEN,
  SETTINGS_SCREEN,
  DEMO_SCREEN
};
Screen currentScreen = MAIN_MENU;

// Menu navigation variables
int8_t mainMenuSelection = 0;
int8_t settingsMenuSelection = 0;
bool settingsEditMode = false;
uint8_t cyclesToRun = 0;

// Timers for non-blocking operations
unsigned long sbsReadTimer = 0;
unsigned long displayUpdateTimer = 0;
unsigned long serialOutputTimer = 0;

// =================== FORWARD DECLARATIONS ===================
void handleEncoder();
void update_settings_value(int amount);

// =================== INTERRUPT SERVICE ROUTINE ===================
// Called on encoder pin change to ensure no rotation is missed.
void encISR() {
  enc.tick();
}

// =================== SETUP ===================
void setup() {
  Serial.begin(115200);

  // Load settings from EEPROM
  loadSettings();
  cyclesToRun = getSettings().cyclesCount;

  // Initialize hardware and modules
  initDisplay();
  initSBS();
  initStateMachine();

  // Attach interrupts for the encoder rotation. CLK=S1, DT=S2.
  attachInterrupt(digitalPinToInterrupt(ENC_S1_PIN), encISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_S2_PIN), encISR, CHANGE);

  // Draw the initial screen
  drawMainMenu(mainMenuSelection);
}

// =================== MAIN LOOP ===================
void loop() {
  // Handle user input from encoder
  handleEncoder();

  // Non-blocking data update task
  if (millis() - sbsReadTimer > (unsigned long)getSettings().smbusReadTimeout * 1000) {
    sbsReadTimer = millis();

    // Read data based on the current screen/mode
    switch(currentScreen) {
        case CYCLES_SCREEN:
            readSBSData(sbsData);
            updateStateMachine(sbsData); // Update state machine with real data
            break;
        case DEVICE_PING_SCREEN:
            readSBSData(sbsData);
            break;
        case DEMO_SCREEN:
            generateDemoData(sbsData, getCurrentState());
            updateStateMachine(sbsData); // Update state machine with demo data
            break;
        default:
            // No data reading needed on other screens
            break;
    }
  }

  // Non-blocking display update task (runs less frequently)
  if (millis() - displayUpdateTimer > 500) {
      displayUpdateTimer = millis();
      switch(currentScreen) {
          case CYCLES_SCREEN:
          case DEMO_SCREEN:
              drawCyclesScreen(sbsData, getCyclesLeft(), cyclesToRun, isProcessRunning());
              break;
          case DEVICE_PING_SCREEN:
              drawDevicePingScreen(sbsData, sbsData.dataValid);
              break;
          // Menu screens are redrawn instantly on encoder events
          default:
              break;
      }
  }

  // Non-blocking serial output task
  if (getSettings().serialOutputTimeout > 0 && millis() - serialOutputTimer > (unsigned long)getSettings().serialOutputTimeout * 1000) {
    serialOutputTimer = millis();
    if (currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN || currentScreen == DEVICE_PING_SCREEN) {
        printFullStatusToSerial(sbsData);
    }
  }
}

// =================== ENCODER HANDLING ===================
void handleEncoder() {
    // We also need to call tick() in the main loop for button handling
    enc.tick();

    bool needsRedraw = false;

    // Handle rotation left (moves selection DOWN, increases value)
    if (enc.isLeft()) {
        if (currentScreen == MAIN_MENU) {
            mainMenuSelection = (mainMenuSelection < 3) ? mainMenuSelection + 1 : 0;
            needsRedraw = true;
        } else if (currentScreen == SETTINGS_SCREEN) {
            if (settingsEditMode) update_settings_value(1); // Increase value
            else settingsMenuSelection = (settingsMenuSelection < 9) ? settingsMenuSelection + 1 : 0; // Navigate menu
            needsRedraw = true;
        } else if ((currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN) && !isProcessRunning()) {
            cyclesToRun = (cyclesToRun < 99) ? cyclesToRun + 1 : 99;
        }
    }

    // Handle rotation right (moves selection UP, decreases value)
    if (enc.isRight()) {
        if (currentScreen == MAIN_MENU) {
            mainMenuSelection = (mainMenuSelection > 0) ? mainMenuSelection - 1 : 3;
            needsRedraw = true;
        } else if (currentScreen == SETTINGS_SCREEN) {
            if (settingsEditMode) update_settings_value(-1); // Decrease value
            else settingsMenuSelection = (settingsMenuSelection > 0) ? settingsMenuSelection - 1 : 9; // Navigate menu
            needsRedraw = true;
        } else if ((currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN) && !isProcessRunning()) {
            cyclesToRun = (cyclesToRun > 0) ? cyclesToRun - 1 : 0;
        }
    }

    // Handle button click
    if (enc.isClick()) {
        switch(currentScreen) {
            case MAIN_MENU:
                // Enter the selected screen
                currentScreen = (Screen)(mainMenuSelection + 1); // Maps menu item to screen enum
                cyclesToRun = getSettings().cyclesCount; // Reset cycles to setting value
                if (currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN) {
                    drawCyclesScreen(sbsData, 0, cyclesToRun, false); // Initial draw
                } else if (currentScreen == DEVICE_PING_SCREEN) {
                    readSBSData(sbsData); // Initial read
                    drawDevicePingScreen(sbsData, sbsData.dataValid);
                } else if (currentScreen == SETTINGS_SCREEN) {
                    settingsMenuSelection = 0;
                    settingsEditMode = false;
                    drawSettingsScreen(settingsMenuSelection, settingsEditMode);
                }
                break;

            case CYCLES_SCREEN:
            case DEMO_SCREEN:
                 // Start or stop the training process
                 startStopProcess(cyclesToRun);
                 break;

            case DEVICE_PING_SCREEN:
                // Exit to main menu
                currentScreen = MAIN_MENU;
                needsRedraw = true;
                break;

            case SETTINGS_SCREEN:
                if (settingsMenuSelection == 8) { // RESET is now item 8
                    resetSettings();
                } else if (settingsMenuSelection == 9) { // RETURN is now item 9
                    getSettings().cyclesCount = cyclesToRun; // Save cycles count on exit
                    saveSettings();
                    currentScreen = MAIN_MENU;
                } else {
                    // Enter/exit edit mode for a setting
                    settingsEditMode = !settingsEditMode;
                }
                needsRedraw = true;
                break;
        }
    }

    // Redraw screen if needed
    if (needsRedraw) {
        if (currentScreen == MAIN_MENU) {
            drawMainMenu(mainMenuSelection);
        } else if (currentScreen == SETTINGS_SCREEN) {
            drawSettingsScreen(settingsMenuSelection, settingsEditMode);
        }
    }
}

// =================== SETTINGS HELPER ===================
void update_settings_value(int amount) {
    Settings& s = getSettings();
    switch (settingsMenuSelection) {
        case 0: s.cyclesCount = constrain(s.cyclesCount + amount, 0, 99); cyclesToRun = s.cyclesCount; break;
        case 1: s.devicePingTimeout = constrain(s.devicePingTimeout + amount, 0, 99); break;
        case 2: s.pauseAfterCharge = constrain(s.pauseAfterCharge + amount, 0, 999); break;
        case 3: s.pauseAfterDischarge = constrain(s.pauseAfterDischarge + amount, 0, 999); break;
        case 4: s.smbusReadTimeout = constrain(s.smbusReadTimeout + amount, 0, 99); break;
        case 5: s.demoChargeTime = constrain(s.demoChargeTime + amount, 0, 99); break;
        case 6: s.demoDischargeTime = constrain(s.demoDischargeTime + amount, 0, 99); break;
        case 7: s.serialOutputTimeout = constrain(s.serialOutputTimeout + amount, 0, 99); break;
    }
}
