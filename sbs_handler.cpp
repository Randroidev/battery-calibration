#include "sbs_handler.h"
#include "config.h"
#include "settings.h"
#include "state_machine.h"
#include <Wire.h>

// SMBus Commands for Smart Battery Specification
#define SBS_CMD_MANUFACTURER_ACCESS    0x00
#define SBS_CMD_REMAINING_CAPACITY_ALARM 0x01
#define SBS_CMD_REMAINING_TIME_ALARM   0x02
#define SBS_CMD_BATTERY_MODE           0x03
#define SBS_CMD_AT_RATE                0x04
#define SBS_CMD_AT_RATE_TIME_TO_FULL   0x05
#define SBS_CMD_AT_RATE_TIME_TO_EMPTY  0x06
#define SBS_CMD_AT_RATE_OK             0x07
#define SBS_CMD_TEMPERATURE            0x08
#define SBS_CMD_VOLTAGE                0x09
#define SBS_CMD_CURRENT                0x0A
#define SBS_CMD_AVERAGE_CURRENT        0x0B
#define SBS_CMD_MAX_ERROR              0x0C
#define SBS_CMD_RELATIVE_STATE_OF_CHARGE 0x0D
#define SBS_CMD_ABSOLUTE_STATE_OF_CHARGE 0x0E
#define SBS_CMD_REMAINING_CAPACITY     0x0F
#define SBS_CMD_FULL_CHARGE_CAPACITY   0x10
#define SBS_CMD_RUN_TIME_TO_EMPTY      0x11
#define SBS_CMD_AVERAGE_TIME_TO_EMPTY  0x12
#define SBS_CMD_AVERAGE_TIME_TO_FULL   0x13
#define SBS_CMD_CHARGING_CURRENT       0x14
#define SBS_CMD_CHARGING_VOLTAGE       0x15
#define SBS_CMD_BATTERY_STATUS         0x16
#define SBS_CMD_CYCLE_COUNT            0x17
#define SBS_CMD_DESIGN_CAPACITY        0x18
#define SBS_CMD_DESIGN_VOLTAGE         0x19
#define SBS_CMD_SPECIFICATION_INFO     0x1A
#define SBS_CMD_MANUFACTURE_DATE       0x1B
#define SBS_CMD_SERIAL_NUMBER          0x1C
#define SBS_CMD_MANUFACTURER_NAME      0x20
#define SBS_CMD_DEVICE_NAME            0x21
#define SBS_CMD_DEVICE_CHEMISTRY       0x22
#define SBS_CMD_MANUFACTURER_DATA      0x23
#define SBS_CMD_CELL_VOLTAGE_4         0x3C
#define SBS_CMD_CELL_VOLTAGE_3         0x3D
#define SBS_CMD_CELL_VOLTAGE_2         0x3E
#define SBS_CMD_CELL_VOLTAGE_1         0x3F


void initSBS() {
  Wire.begin();
  Wire.setClock(100000); // Standard I2C speed
}

// Helper function to read a 16-bit word from SMBus
bool readSMBusWord(uint8_t command, uint16_t& result) {
    Wire.beginTransmission(SMBUS_ADDRESS);
    Wire.write(command);
    if (Wire.endTransmission(false) != 0) { // Send command, false = restart
        return false;
    }

    Wire.requestFrom(SMBUS_ADDRESS, 2, true); // Request 2 bytes, true = stop
    if (Wire.available() == 2) {
        uint8_t lowByte = Wire.read();
        uint8_t highByte = Wire.read();
        result = (highByte << 8) | lowByte;
        return true;
    }
    return false;
}

bool readSBSData(sbs_data_t& data) {
  bool success = true;
  success &= readSMBusWord(SBS_CMD_VOLTAGE, data.voltage);
  success &= readSMBusWord(SBS_CMD_CURRENT, (uint16_t&)data.current); // Cast to uint16_t for read
  success &= readSMBusWord(SBS_CMD_TEMPERATURE, data.temperature);
  success &= readSMBusWord(SBS_CMD_MAX_ERROR, data.maxError);
  success &= readSMBusWord(SBS_CMD_REMAINING_CAPACITY, data.remainingCapacity);
  success &= readSMBusWord(SBS_CMD_FULL_CHARGE_CAPACITY, data.fullChargeCapacity);
  success &= readSMBusWord(SBS_CMD_CHARGING_CURRENT, data.chargingCurrent);
  success &= readSMBusWord(SBS_CMD_CHARGING_VOLTAGE, data.chargingVoltage);
  success &= readSMBusWord(SBS_CMD_CYCLE_COUNT, data.cycleCount);
  success &= readSMBusWord(SBS_CMD_DESIGN_VOLTAGE, data.designVoltage);
  success &= readSMBusWord(SBS_CMD_DESIGN_CAPACITY, data.designCapacity);
  success &= readSMBusWord(SBS_CMD_CELL_VOLTAGE_1, data.cellVoltage1);
  success &= readSMBusWord(SBS_CMD_CELL_VOLTAGE_2, data.cellVoltage2);
  success &= readSMBusWord(SBS_CMD_CELL_VOLTAGE_3, data.cellVoltage3);
  success &= readSMBusWord(SBS_CMD_CELL_VOLTAGE_4, data.cellVoltage4);
  success &= readSMBusWord(SBS_CMD_BATTERY_MODE, data.batteryMode);
  success &= readSMBusWord(SBS_CMD_BATTERY_STATUS, data.batteryStatus);

  data.dataValid = success;
  return success;
}

void generateDemoData(sbs_data_t& data, uint8_t state) {
    static unsigned long demoTimer = 0;
    static float fakeVoltage = 12500; // in mV
    static float fakeCapacity = 2000; // in mAh
    Settings& s = getSettings();

    if (millis() - demoTimer > 1000) { // Update once per second
        if (state == 1) { // Discharging
            fakeVoltage -= (float)s.demoDischargeTime * 10;
            fakeCapacity -= (float)s.demoDischargeTime * 5;
            if (fakeVoltage < 10000) fakeVoltage = 10000;
            if (fakeCapacity < 0) fakeCapacity = 0;
        } else if (state == 3) { // Charging
            fakeVoltage += (float)s.demoChargeTime * 10;
            fakeCapacity += (float)s.demoChargeTime * 5;
            if (fakeVoltage > 14800) fakeVoltage = 14800;
            if (fakeCapacity > 2200) fakeCapacity = 2200;
        }
        demoTimer = millis();
    }

    data.voltage = fakeVoltage;
    data.current = (state == 1) ? -1500 : (state == 3) ? 1500 : 0;
    data.temperature = 2981; // 25 C in Kelvin * 10
    data.maxError = 2;
    data.remainingCapacity = fakeCapacity;
    data.fullChargeCapacity = 2200;
    data.chargingCurrent = (state == 3) ? 1500 : 0;
    data.chargingVoltage = (state == 3) ? 14800 : 0;
    data.cycleCount = 42;
    data.designVoltage = 14800;
    data.designCapacity = 2200;
    data.cellVoltage1 = data.voltage / 4;
    data.cellVoltage2 = data.voltage / 4;
    data.cellVoltage3 = data.voltage / 4;
    data.cellVoltage4 = data.voltage / 4;

    // Simulate status flags
    data.batteryMode = 0x0001; // Internal Charge Controller
    if (state == 1 && fakeCapacity < 100) {
        data.batteryStatus = (1 << 5); // Fully Discharged
    } else if (state == 3 && fakeCapacity > 2100) {
        data.batteryStatus = (1 << 4); // Fully Charged
    } else {
        data.batteryStatus = (state == 1) ? (1 << 6) : 0; // Discharging bit
    }

    data.dataValid = true;
}

const char* stateToString(TrainerState state) {
    switch(state) {
        case IDLE: return "IDLE";
        case DISCHARGING: return "DISCHARGING";
        case PAUSE_AFTER_DISCHARGE: return "PAUSE (after discharge)";
        case CHARGING: return "CHARGING";
        case PAUSE_AFTER_CHARGE: return "PAUSE (after charge)";
        default: return "UNKNOWN";
    }
}

void printFullStatusToSerial(const sbs_data_t& data) {
    char buf[80];

    Serial.println("\n===== BATTERY TRAINER STATUS =====");

    // Trainer Status
    sprintf(buf, "Process Running: %s | State: %s", isProcessRunning() ? "YES" : "NO", stateToString(getCurrentState()));
    Serial.println(buf);
    sprintf(buf, "Cycles Left: %d", getCyclesLeft());
    Serial.println(buf);

    Serial.println("--- SMART BATTERY DATA ---");
    if (!data.dataValid) {
        Serial.println("!!! NO BATTERY DATA !!!");
        Serial.println("================================");
        return;
    }

    sprintf(buf, "Voltage: %.2f V | Current: %.2f A", data.voltage / 1000.0, data.current / 1000.0);
    Serial.println(buf);

    sprintf(buf, "Temperature: %.2f C | Max Error: %d %%", (data.temperature / 10.0) - 273.15, data.maxError);
    Serial.println(buf);

    sprintf(buf, "Rem. Cap: %d mAh | Full Cap: %d mAh", data.remainingCapacity, data.fullChargeCapacity);
    Serial.println(buf);

    sprintf(buf, "Charging Vol/Cur: %.2f V / %.2f A", data.chargingVoltage / 1000.0, data.chargingCurrent / 1000.0);
    Serial.println(buf);

    sprintf(buf, "Cycle Count: %d", data.cycleCount);
    Serial.println(buf);

    sprintf(buf, "Cell Voltages: #1:%.2fV #2:%.2fV #3:%.2fV #4:%.2fV",
            data.cellVoltage1 / 1000.0, data.cellVoltage2 / 1000.0,
            data.cellVoltage3 / 1000.0, data.cellVoltage4 / 1000.0);
    Serial.println(buf);

    Serial.print("Battery Status Flags: ");
    Serial.println(data.batteryStatus, BIN);

    Serial.print("Battery Mode Flags:   ");
    Serial.println(data.batteryMode, BIN);

    Serial.println("================================");
}
