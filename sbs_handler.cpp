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

void generateDemoData(sbs_data_t& data, TrainerState state) {
    static float fakeVoltage = 12500;
    static float fakeCapacity = 2000;
    static uint16_t fakeStatus = 0;

    // Animate voltage and capacity based on state
    if (state == DISCHARGING) {
        fakeVoltage -= 10.0;
        fakeCapacity -= 0.5;
        if (fakeVoltage <= 10000) {
            fakeVoltage = 10000;
            fakeStatus |= 0x0020; // Set "fully discharged"
        }
    } else if (state == CHARGING) {
        fakeVoltage += 10.0;
        fakeCapacity += 0.5;
        if (fakeVoltage >= 14800) {
            fakeVoltage = 14800;
            fakeStatus |= 0x0010; // Set "fully charged"
        }
    } else {
        // In pause states, clear the flags
        fakeStatus &= ~0x0030;
    }

    // Clamp values to realistic ranges
    fakeVoltage = constrain(fakeVoltage, 10000, 14800);
    fakeCapacity = constrain(fakeCapacity, 0, 2200);

    // Populate all fields of the sbs_data_t struct
    data.voltage = fakeVoltage;
    data.current = (state == DISCHARGING) ? -1500 : (state == CHARGING) ? 1500 : 0;
    data.temperature = 2981; // 25 C in tenths of a degree Kelvin
    data.maxError = 2; // 2%
    data.remainingCapacity = fakeCapacity;
    data.fullChargeCapacity = 2200;
    data.chargingCurrent = (state == CHARGING) ? 1500 : 0;
    data.chargingVoltage = (state == CHARGING) ? 14800 : 0;
    data.cycleCount = 42;
    data.designVoltage = 14800;
    data.designCapacity = 2200;

    // Distribute voltage somewhat realistically among cells
    float cellVolt1 = fakeVoltage / 4 + 50;
    float cellVolt2 = fakeVoltage / 4 - 30;
    float cellVolt3 = fakeVoltage / 4 + 10;
    data.cellVoltage1 = cellVolt1;
    data.cellVoltage2 = cellVolt2;
    data.cellVoltage3 = cellVolt3;
    data.cellVoltage4 = fakeVoltage - cellVolt1 - cellVolt2 - cellVolt3;

    data.batteryMode = 0x8001; // Internal Charge Controller, Capacity Mode
    data.batteryStatus = fakeStatus;
    data.dataValid = true;
}

