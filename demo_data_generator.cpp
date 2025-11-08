#include "demo_data_generator.h"
#include "sbs_handler.h"

void generateDemoData(sbs_data_t& sbsData, TrainerState currentState) {
    sbsData.dataValid = true;
    sbsData.designVoltage = 12000;
    sbsData.designCapacity = 2200;
    sbsData.fullChargeCapacity = 2150;
    sbsData.cycleCount = 68;
    sbsData.maxError = 2;
    sbsData.temperature = 250; // 25.0 C
    sbsData.cellVoltage1 = 3000;
    sbsData.cellVoltage2 = 3000;
    sbsData.cellVoltage3 = 3000;
    sbsData.cellVoltage4 = 0;
    sbsData.chargingVoltage = 12600;
    sbsData.chargingCurrent = 1100;

    // Default flags
    sbsData.batteryMode = 0x0001; // Internal charge controller
    sbsData.batteryStatus = 0x0000;

    switch (currentState) {
        case CHARGING:
            sbsData.voltage = 12500;
            sbsData.current = 1000;
            sbsData.remainingCapacity = 1800;
            sbsData.batteryStatus |= (1 << 6); // CHARGING
            break;
        case PAUSE_AFTER_CHARGE:
            sbsData.voltage = 12600;
            sbsData.current = 0;
            sbsData.remainingCapacity = 2150;
            sbsData.batteryStatus |= (1 << 5); // FULLY_CHARGED
            sbsData.batteryStatus |= (1 << 7); // TERMINATE_CHARGE_ALARM
            break;
        case DISCHARGING:
            sbsData.voltage = 11500;
            sbsData.current = -500;
            sbsData.remainingCapacity = 1200;
            sbsData.batteryStatus |= (1 << 4); // DISCHARGING
            break;
        case PAUSE_AFTER_DISCHARGE:
            sbsData.voltage = 10800;
            sbsData.current = 0;
            sbsData.remainingCapacity = 50;
            sbsData.batteryStatus |= (1 << 4); // DISCHARGING - to indicate it's empty
            sbsData.batteryStatus |= (1 << 8); // TERMINATE_DISCHARGE_ALARM
            break;
        case IDLE:
            sbsData.voltage = 12200;
            sbsData.current = 0;
            sbsData.remainingCapacity = 1500;
            break;
    }
}
