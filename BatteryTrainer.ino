/*
 * Battery Trainer Sketch
 * Author: Jules
 * Date: 2025-11-07
 */

#include "config.h"
#include "settings.h"
#include "display.h"
#include "sbs_handler.h"
#include "state_machine.h"

// =================== GLOBAL OBJECTS ===================
sbs_data_t sbsData;

// =================== PROGRAM STATE ===================
enum Screen { MAIN_MENU, CYCLES_SCREEN, DEVICE_PING_SCREEN, SETTINGS_SCREEN, DEMO_SCREEN };
Screen currentScreen = MAIN_MENU;

// Menu navigation
int8_t mainMenuSelection = 0;
int8_t settingsMenuSelection = 0;
bool settingsEditMode = false;
uint8_t cyclesToRun = 0;

// Timers
unsigned long sbsReadTimer = 0;
unsigned long displayUpdateTimer = 0;
unsigned long serialOutputTimer = 0;

// =================== ENCODER VARS ===================
volatile long encoderPos = 0;
unsigned long lastButtonPress = 0;

// =================== ISR ===================
void readEncoder() {
  static uint8_t old_AB = 0;
  // Read the current state of CLK and DT
  old_AB <<= 2;
  old_AB |= (digitalRead(ENC_S1_PIN) << 1) | digitalRead(ENC_S2_PIN);
  // A standard quadrature encoder pattern this will produce is 3, 1, 0, 2
  // Clockwise: 3, 1, 0, 2, ...
  // Counter-clockwise: 3, 2, 0, 1, ...
  if ((old_AB & 0x0F) == 0x0b) encoderPos++; // Clockwise
  if ((old_AB & 0x0F) == 0x07) encoderPos--; // Counter-clockwise
}

// =================== SETUP ===================
void setup() {
  Serial.begin(115200);

  pinMode(ENC_S1_PIN, INPUT);
  pinMode(ENC_S2_PIN, INPUT);
  pinMode(ENC_KEY_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_S1_PIN), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_S2_PIN), readEncoder, CHANGE);

  loadSettings();
  cyclesToRun = getSettings().cyclesCount;

  initDisplay();
  initSBS();
  initStateMachine();

  drawMainMenu(mainMenuSelection, true);
}

// =================== MAIN LOOP ===================
void loop() {
  handleEncoder();
  handleButton();

  if (millis() - sbsReadTimer > (unsigned long)getSettings().smbusReadTimeout * 1000) {
    sbsReadTimer = millis();
    if (currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN) {
      readSBSData(sbsData);
      updateStateMachine(sbsData);
    } else if (currentScreen == DEVICE_PING_SCREEN) {
      readSBSData(sbsData);
    }
  }

  if (millis() - displayUpdateTimer > 500) {
      displayUpdateTimer = millis();
      if (currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN) {
        drawCyclesScreen(sbsData, getCyclesLeft(), cyclesToRun, isProcessRunning(), false);
      } else if (currentScreen == DEVICE_PING_SCREEN) {
        drawDevicePingScreen(sbsData, sbsData.dataValid, false);
      }
  }

  if (getSettings().serialOutputTimeout > 0 && millis() - serialOutputTimer > (unsigned long)getSettings().serialOutputTimeout * 1000) {
    serialOutputTimer = millis();
    if (currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN || currentScreen == DEVICE_PING_SCREEN) {
        printFullStatusToSerial(sbsData);
    }
  }
}

// =================== HANDLERS ===================
void handleEncoder() {
  static long lastEncoderPos = 0;
  long currentPos;

  // Atomically read the encoder position
  noInterrupts();
  currentPos = encoderPos;
  interrupts();

  if (currentPos != lastEncoderPos) {
    int direction = (currentPos > lastEncoderPos) ? 1 : -1;

    switch(currentScreen) {
        case MAIN_MENU:
            mainMenuSelection = (mainMenuSelection + direction + 4) % 4;
            drawMainMenu(mainMenuSelection, false);
            break;
        case SETTINGS_SCREEN:
            if(settingsEditMode) {
                update_settings_value(direction);
            } else {
                settingsMenuSelection = (settingsMenuSelection + direction + 10) % 10;
            }
            drawSettingsScreen(settingsMenuSelection, settingsEditMode, false);
            break;
        case CYCLES_SCREEN:
        case DEMO_SCREEN:
            if (!isProcessRunning()) {
                cyclesToRun = constrain(cyclesToRun + direction, 0, 99);
                drawCyclesScreen(sbsData, 0, cyclesToRun, false, false);
            }
            break;
    }
    lastEncoderPos = currentPos;
  }
}

void handleButton() {
    if (digitalRead(ENC_KEY_PIN) == LOW && millis() - lastButtonPress > 250) {
        lastButtonPress = millis();

        switch(currentScreen) {
            case MAIN_MENU:
                currentScreen = (Screen)(mainMenuSelection + 1);
                cyclesToRun = getSettings().cyclesCount;
                if (currentScreen == CYCLES_SCREEN || currentScreen == DEMO_SCREEN) drawCyclesScreen(sbsData, 0, cyclesToRun, false, true);
                else if (currentScreen == DEVICE_PING_SCREEN) { readSBSData(sbsData); drawDevicePingScreen(sbsData, sbsData.dataValid, true); }
                else if (currentScreen == SETTINGS_SCREEN) drawSettingsScreen(settingsMenuSelection, settingsEditMode, true);
                break;
            case CYCLES_SCREEN: case DEMO_SCREEN:
                 startStopProcess(cyclesToRun);
                 drawCyclesScreen(sbsData, getCyclesLeft(), cyclesToRun, isProcessRunning(), true);
                 break;
            case DEVICE_PING_SCREEN:
                currentScreen = MAIN_MENU;
                drawMainMenu(mainMenuSelection, true);
                break;
            case SETTINGS_SCREEN:
                if (settingsMenuSelection == 8) { resetSettings(); drawSettingsScreen(settingsMenuSelection, settingsEditMode, true); }
                else if (settingsMenuSelection == 9) { getSettings().cyclesCount = cyclesToRun; saveSettings(); currentScreen = MAIN_MENU; drawMainMenu(mainMenuSelection, true); }
                else { settingsEditMode = !settingsEditMode; drawSettingsScreen(settingsMenuSelection, settingsEditMode, false); }
                break;
        }
    }
}

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
