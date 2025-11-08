#include "state_machine.h"
#include "config.h"
#include "settings.h"
#include <Arduino.h>

static TrainerState currentState = IDLE;
static uint8_t cyclesLeft = 0;
static unsigned long phaseTimer = 0;
static bool processRunning = false;

void allOff() {
    digitalWrite(LED_DISCHARGE_PIN, LOW);
    digitalWrite(LED_PAUSE_DISCHARGE_PIN, LOW);
    digitalWrite(LED_CHARGE_PIN, LOW);
    digitalWrite(LED_PAUSE_CHARGE_PIN, LOW);
    digitalWrite(RELAY_DISCHARGE_PIN, LOW);
    digitalWrite(RELAY_CHARGE_PIN, LOW);
}

void initStateMachine() {
    pinMode(LED_DISCHARGE_PIN, OUTPUT);
    pinMode(LED_PAUSE_DISCHARGE_PIN, OUTPUT);
    pinMode(LED_CHARGE_PIN, OUTPUT);
    pinMode(LED_PAUSE_CHARGE_PIN, OUTPUT);
    pinMode(RELAY_DISCHARGE_PIN, OUTPUT);
    pinMode(RELAY_CHARGE_PIN, OUTPUT);
    allOff();
}

void startStopProcess(uint8_t cycles) {
    processRunning = !processRunning;
    if (processRunning && cycles > 0) {
        cyclesLeft = cycles;
        currentState = DISCHARGING;
        phaseTimer = millis();
    } else {
        processRunning = false;
        currentState = IDLE;
        allOff();
    }
}

void updateStateMachine(const sbs_data_t& sbsData, bool isDemoMode) {
    if (!processRunning) return;

    Settings& s = getSettings();
    unsigned long chargeTime = isDemoMode ? (unsigned long)s.demoChargeTime * 1000 : 0;
    unsigned long dischargeTime = isDemoMode ? (unsigned long)s.demoDischargeTime * 1000 : 0;

    switch(currentState) {
        case DISCHARGING:
            digitalWrite(RELAY_CHARGE_PIN, LOW);
            digitalWrite(RELAY_DISCHARGE_PIN, HIGH);
            if ((isDemoMode && millis() - phaseTimer > dischargeTime) || (!isDemoMode && (sbsData.batteryStatus & (1 << 5)))) {
                currentState = PAUSE_AFTER_DISCHARGE;
                phaseTimer = millis();
            }
            break;
        case PAUSE_AFTER_DISCHARGE:
            allOff();
            if (millis() - phaseTimer > (isDemoMode ? (unsigned long)s.demoDischargeTime * 1000 : (unsigned long)s.pauseAfterDischarge * 60000)) {
                currentState = CHARGING;
                phaseTimer = millis();
            }
            break;
        case CHARGING:
            digitalWrite(RELAY_DISCHARGE_PIN, LOW);
            digitalWrite(RELAY_CHARGE_PIN, HIGH);
            if ((isDemoMode && millis() - phaseTimer > chargeTime) || (!isDemoMode && (sbsData.batteryStatus & (1 << 4)))) {
                currentState = PAUSE_AFTER_CHARGE;
                phaseTimer = millis();
            }
            break;
        case PAUSE_AFTER_CHARGE:
            allOff();
            if (millis() - phaseTimer > (isDemoMode ? (unsigned long)s.demoChargeTime * 1000 : (unsigned long)s.pauseAfterCharge * 60000)) {
                cyclesLeft--;
                if (cyclesLeft > 0) {
                    currentState = DISCHARGING;
                    phaseTimer = millis();
                } else {
                    processRunning = false;
                    currentState = IDLE;
                }
            }
            break;
        case IDLE:
        default:
            allOff();
            break;
    }
}

TrainerState getCurrentState() { return currentState; }
uint8_t getCyclesLeft() { return cyclesLeft; }
bool isProcessRunning() { return processRunning; }
