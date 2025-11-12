#include "serial_logger.h"
#include <Arduino.h>

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

void printFullStatusToSerial(const sbs_data_t& data, int currentCycle, int totalCycles, unsigned long elapsedTime) {
    char buf[80];

    Serial.println("\n===== BATTERY TRAINER STATUS =====");

    // Calibration Status
    if (currentCycle != -1 && totalCycles != -1) {
        unsigned long hours = elapsedTime / 3600000;
        unsigned long minutes = (elapsedTime % 3600000) / 60000;
        unsigned long seconds = (elapsedTime % 60000) / 1000;
        sprintf(buf, "Calibration Cycle: %d / %d | Elapsed Time: %lu h %lu m %lu s",
                currentCycle, totalCycles, hours, minutes, seconds);
        Serial.println(buf);
    }

    // Trainer Status
    sprintf(buf, "Process Running: %s | State: %s", isProcessRunning() ? "YES" : "NO", stateToString(getCurrentState()));
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

void printKeyValueStatusToSerial(const sbs_data_t& data) {
    Serial.print("running:"); Serial.print(isProcessRunning() ? "true" : "false"); Serial.print(",");
    Serial.print("state:"); Serial.print(stateToString(getCurrentState())); Serial.print(",");
    Serial.print("data_valid:"); Serial.print(data.dataValid ? "true" : "false");

    if (data.dataValid) {
        Serial.print(",");
        Serial.print("voltage:"); Serial.print(data.voltage); Serial.print(",");
        Serial.print("current:"); Serial.print(data.current); Serial.print(",");
        Serial.print("temperature:"); Serial.print(data.temperature); Serial.print(",");
        Serial.print("max_error:"); Serial.print(data.maxError); Serial.print(",");
        Serial.print("rem_cap:"); Serial.print(data.remainingCapacity); Serial.print(",");
        Serial.print("full_cap:"); Serial.print(data.fullChargeCapacity); Serial.print(",");
        Serial.print("chg_voltage:"); Serial.print(data.chargingVoltage); Serial.print(",");
        Serial.print("chg_current:"); Serial.print(data.chargingCurrent); Serial.print(",");
        Serial.print("cycle_count:"); Serial.print(data.cycleCount); Serial.print(",");
        Serial.print("design_voltage:"); Serial.print(data.designVoltage); Serial.print(",");
        Serial.print("design_cap:"); Serial.print(data.designCapacity); Serial.print(",");
        Serial.print("cell1_v:"); Serial.print(data.cellVoltage1); Serial.print(",");
        Serial.print("cell2_v:"); Serial.print(data.cellVoltage2); Serial.print(",");
        Serial.print("cell3_v:"); Serial.print(data.cellVoltage3); Serial.print(",");
        Serial.print("cell4_v:"); Serial.print(data.cellVoltage4); Serial.print(",");
        Serial.print("status_flags:"); Serial.print(data.batteryStatus, HEX); Serial.print(",");
        Serial.print("mode_flags:"); Serial.print(data.batteryMode, HEX);
    }

    Serial.println(); // End of line for the key-value pair data
}
