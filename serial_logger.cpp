#include "serial_logger.h"
#include <Arduino.h>

const char* stateToString(TrainerState state) {
    switch(state) {
        case IDLE: return "IDLE";
        case DISCHARGING: return "DISCHARGING";
        case PAUSE_AFTER_DISCHARGE: return "PAUSE_AFTER_DISCHARGE";
        case CHARGING: return "CHARGING";
        case PAUSE_AFTER_CHARGE: return "PAUSE_AFTER_CHARGE";
        default: return "UNKNOWN";
    }
}

void logHumanReadable(const sbs_data_t& data, TrainerState state, uint8_t cyclesLeft) {
    if (!data.dataValid) {
        Serial.println("--- NO BATTERY DATA ---");
        return;
    }

    char buf[100];
    Serial.println("\n===== BATTERY STATUS =====");
    sprintf(buf, "State: %s | Cycles Left: %d", stateToString(state), cyclesLeft);
    Serial.println(buf);

    Serial.println("--- Main Parameters ---");
    sprintf(buf, "Voltage: %.2f V | Current: %.2f A", data.voltage / 1000.0, data.current / 1000.0);
    Serial.println(buf);
    sprintf(buf, "Temperature: %.2f C | Remaining Capacity: %d mAh", (data.temperature / 10.0) - 273.15, data.remainingCapacity);
    Serial.println(buf);

    Serial.println("--- Flags ---");
    Serial.print("Battery Status (BIN): "); Serial.println(data.batteryStatus, BIN);
    Serial.print("Battery Mode (BIN):   "); Serial.println(data.batteryMode, BIN);
    Serial.println("============================");
}

void logMachineReadable(const sbs_data_t& data, TrainerState state, uint8_t cyclesLeft) {
    if (!data.dataValid) {
        Serial.println("DATA_INVALID");
        return;
    }

    String output = "BEGIN|";
    output += "STATE:" + String(stateToString(state)) + "|";
    output += "CYCLES_LEFT:" + String(cyclesLeft) + "|";
    output += "VOLTAGE:" + String(data.voltage) + "|";
    output += "CURRENT:" + String(data.current) + "|";
    output += "TEMP:" + String(data.temperature) + "|";
    output += "REM_CAP:" + String(data.remainingCapacity) + "|";
    output += "FULL_CAP:" + String(data.fullChargeCapacity) + "|";
    output += "STATUS:" + String(data.batteryStatus, BIN) + "|";
    output += "MODE:" + String(data.batteryMode, BIN) + "|";
    output += "END";
    Serial.println(output);
}
