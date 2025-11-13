/*
 * Battery Trainer Sketch (Nano Serial Version)
 * Author: Jules
 * Date: 2025-11-12
 */

#include "config.h"
#include "sbs_handler.h"
#include "state_machine.h"
#include "serial_logger.h"
#include "demo_data_generator.h"

// =================== GLOBAL OBJECTS & STATE ===================
sbs_data_t sbsData;
bool isDemoMode = false;
TrainerState currentState = IDLE;
bool processRunning = false;

// =================== PROTOTYPES ===================
void showMenu();
void handleUserInput();
void runCalibrationProcess();
void runChargeProcess(bool partOfCalibration = false);
void runDischargeProcess(bool partOfCalibration = false);
void runPause(long unsigned int pauseMillis, int currentCycle, int totalCycles, unsigned long startTime);
void readAndDisplayBatteryData();
void controlChargeRelay(bool on);
void controlDischargeRelay(bool on);
char readSerialChar();
int readSerialInteger(int minVal, int maxVal);
String readSerialLine();

// =================== SETUP ===================
void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(CHARGE_RELAY_PIN, OUTPUT);
  pinMode(DISCHARGE_RELAY_PIN, OUTPUT);
  pinMode(LED_CHARGE_RUN_PIN, OUTPUT);
  pinMode(LED_CHARGE_DONE_PIN, OUTPUT);
  pinMode(LED_DISCHARGE_RUN_PIN, OUTPUT);
  pinMode(LED_DISCHARGE_DONE_PIN, OUTPUT);

  // Relays are active-low, so HIGH is OFF
  digitalWrite(CHARGE_RELAY_PIN, HIGH);
  digitalWrite(DISCHARGE_RELAY_PIN, HIGH);
  // Turn all LEDs off
  digitalWrite(LED_CHARGE_RUN_PIN, LOW);
  digitalWrite(LED_CHARGE_DONE_PIN, LOW);
  digitalWrite(LED_DISCHARGE_RUN_PIN, LOW);
  digitalWrite(LED_DISCHARGE_DONE_PIN, LOW);

  initSBS();

  for (int attempts = 1; attempts <= 3; attempts++) {
    if (readSBSData(sbsData)) {
      printFullStatusToSerial(sbsData);
      break; // Exit loop on success
    } else {
      Serial.print("Device not found [");
      Serial.print(attempts);
      Serial.println("]");
      delay(1000);
    }
  }
  showMenu(); // Always show menu after attempts
}

// =================== MAIN LOOP ===================
void loop() {
  handleUserInput();
}

// =================== MENU & USER INPUT ===================
void showMenu() {
  Serial.println("\nSelect an option:");
  Serial.println("1. Run Calibration");
  Serial.println("2. Read Battery Data");
  Serial.println("3. Start Charge");
  Serial.println("4. Start Discharge");
  Serial.println("5. Demo");
}

void handleUserInput() {
  if (Serial.available() > 0) {
    char command = readSerialChar();
    switch (command) {
      case '1':
        isDemoMode = false;
        runCalibrationProcess();
        break;
      case '2':
        readAndDisplayBatteryData();
        break;
      case '3':
        isDemoMode = false;
        runChargeProcess();
        break;
      case '4':
        isDemoMode = false;
        runDischargeProcess();
        break;
      case '5':
        isDemoMode = true;
        runCalibrationProcess(); // Demo mode now calls the same calibration process
        break;
      default:
        // Ignore invalid input
        break;
    }
  }
}

String readSerialLine() {
  String line = "";
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        // End of line
        if (line.length() > 0) {
          // Skip trailing LF if it's a CRLF
          if (c == '\r' && Serial.peek() == '\n') {
            Serial.read();
          }
          return line;
        }
      } else {
        line += c;
      }
    }
  }
}

// Function to read a single character, ignoring line endings
char readSerialChar() {
  return readSerialLine().charAt(0);
}

int readSerialInteger(int minVal, int maxVal) {
  while (true) {
    String line = readSerialLine();
    int num = line.toInt();
    if (num >= minVal && num <= maxVal) {
      return num;
    } else {
      Serial.print("Invalid input. Please enter a number between ");
      Serial.print(minVal);
      Serial.print(" and ");
      Serial.print(maxVal);
      Serial.println(".");
    }
  }
}


// =================== CORE PROCESSES ===================
void runCalibrationProcess() {
  processRunning = true;
  Serial.println("Enter cycles count [0...5]");
  int cycles = readSerialInteger(0, 5);

  if (cycles == 0) {
    processRunning = false;
    showMenu();
    return;
  }

  Serial.println("Starting calibration pre-charge...");
  currentState = CHARGING;
  runChargeProcess(true);

  Serial.println("Waiting 1 hour after pre-charge...");
  currentState = PAUSE_AFTER_CHARGE;
  if (isDemoMode) {
    generateDemoData(sbsData, PAUSE_AFTER_CHARGE);
    printFullStatusToSerial(sbsData, 0, cycles, 0);
  }
  if (!isDemoMode) controlChargeRelay(true);
  runPause(3600000UL, 0, cycles, millis());

  unsigned long startTime = millis();

  for (int i = 1; i <= cycles; i++) {
    Serial.print("Starting calibration cycle "); Serial.print(i); Serial.print("/"); Serial.print(cycles); Serial.println(": Discharge");
    currentState = DISCHARGING;
    runDischargeProcess(true);
    if (!isDemoMode) controlDischargeRelay(false);

    Serial.print("Calibration cycle "); Serial.print(i); Serial.print("/"); Serial.print(cycles); Serial.println(": 5-hour pause");
    currentState = PAUSE_AFTER_DISCHARGE;
    if(isDemoMode) {
      generateDemoData(sbsData, PAUSE_AFTER_DISCHARGE);
      printFullStatusToSerial(sbsData, i, cycles, millis() - startTime);
    }
    runPause(18000000UL, i, cycles, startTime);

    Serial.print("Calibration cycle "); Serial.print(i); Serial.print("/"); Serial.print(cycles); Serial.println(": Charge");
    currentState = CHARGING;
    runChargeProcess(true);

    Serial.print("Calibration cycle "); Serial.print(i); Serial.print("/"); Serial.print(cycles); Serial.println(": 1-hour pause");
    currentState = PAUSE_AFTER_CHARGE;
    if (isDemoMode) {
      generateDemoData(sbsData, PAUSE_AFTER_CHARGE);
      printFullStatusToSerial(sbsData, i, cycles, millis() - startTime);
    }
    if (!isDemoMode) controlChargeRelay(true);
    runPause(3600000UL, i, cycles, startTime);
  }

  Serial.println("\nCalibration process finished!");
  processRunning = false;
  currentState = IDLE;
  isDemoMode = false;
  showMenu();
}

void runChargeProcess(bool partOfCalibration) {
  if (!partOfCalibration) processRunning = true;
  currentState = CHARGING;
  controlChargeRelay(true);
  bool readFailed = false;

  if (isDemoMode) {
    generateDemoData(sbsData, CHARGING);
    printFullStatusToSerial(sbsData);
    delay(3000);
    sbsData.batteryStatus |= (1 << 5); // Set FC flag
    controlChargeRelay(false);
  } else {
      unsigned long lastReadTime = 0;
      const long readInterval = 15000;

      while (true) {
        if (millis() - lastReadTime >= readInterval) {
          lastReadTime = millis();
          if (readSBSData(sbsData)) {
            printFullStatusToSerial(sbsData);
            if (sbsData.batteryStatus & (1 << 5)) {
              Serial.println("Battery fully charged.");
              break;
            }
          } else {
            Serial.println("Failed to read battery data. Stopping charge.");
            readFailed = true;
            break;
          }
        }
      }
  }

  if (!partOfCalibration) {
    if (!isDemoMode) controlChargeRelay(false);
    if (readFailed) {
      Serial.println("Pausing for 5 seconds before returning to menu...");
      delay(5000);
    }
    processRunning = false;
    currentState = IDLE;
    showMenu();
  }
}

void runDischargeProcess(bool partOfCalibration) {
  if (!partOfCalibration) processRunning = true;
  currentState = DISCHARGING;
  controlDischargeRelay(true);
  bool readFailed = false;

  if (isDemoMode) {
    generateDemoData(sbsData, DISCHARGING);
    printFullStatusToSerial(sbsData);
    delay(3000);
    sbsData.batteryStatus |= (1 << 4); // Set FD flag
    controlDischargeRelay(false);
  } else {
      unsigned long lastReadTime = 0;
      const long readInterval = 15000;

      while (true) {
        if (millis() - lastReadTime >= readInterval) {
          lastReadTime = millis();
          if (readSBSData(sbsData)) {
            printFullStatusToSerial(sbsData);
            if (sbsData.batteryStatus & (1 << 4)) {
              Serial.println("Battery fully discharged.");
              break;
            }
          } else {
            Serial.println("Failed to read battery data. Stopping discharge.");
            readFailed = true;
            break;
          }
        }
      }
  }

  if (!partOfCalibration) {
    if (!isDemoMode) controlDischargeRelay(false);
     if (readFailed) {
      Serial.println("Pausing for 5 seconds before returning to menu...");
      delay(5000);
    }
    processRunning = false;
    currentState = IDLE;
    showMenu();
  }
}

void runPause(long unsigned int pauseMillis, int currentCycle, int totalCycles, unsigned long startTime) {
    if (isDemoMode) {
        delay(3000);
        return;
    }

    unsigned long pauseStartTime = millis();
    unsigned long lastReadTime = 0;
    const long readInterval = 60000;

    while (millis() - pauseStartTime < pauseMillis) {
        if (millis() - lastReadTime >= readInterval) {
            lastReadTime = millis();
            if(readSBSData(sbsData)) {
                printFullStatusToSerial(sbsData, currentCycle, totalCycles, millis() - startTime);
            } else {
                Serial.println("Failed to read battery data during pause.");
            }
        }
    }
}


void readAndDisplayBatteryData() {
    if (readSBSData(sbsData)) {
        printFullStatusToSerial(sbsData);
    } else {
        Serial.println("Failed to read battery data.");
    }
    showMenu();
}

// =================== HARDWARE CONTROL ===================
void controlChargeRelay(bool on) {
  if (on) {
    digitalWrite(LED_CHARGE_DONE_PIN, LOW);
  }

  digitalWrite(CHARGE_RELAY_PIN, on ? LOW : HIGH);
  digitalWrite(LED_CHARGE_RUN_PIN, on ? HIGH : LOW);

  if (!on) {
    digitalWrite(LED_CHARGE_DONE_PIN, HIGH);
  }
}

void controlDischargeRelay(bool on) {
  if (on) {
    digitalWrite(LED_DISCHARGE_DONE_PIN, LOW);
  }

  digitalWrite(DISCHARGE_RELAY_PIN, on ? LOW : HIGH);
  digitalWrite(LED_DISCHARGE_RUN_PIN, on ? HIGH : LOW);

  if (!on) {
    digitalWrite(LED_DISCHARGE_DONE_PIN, HIGH);
  }
}
