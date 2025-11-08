#include "display.h"
#include "config.h"
#include "settings.h"
#include <Adafruit_ST7789.h>

// =================== COLOR DEFINITIONS ===================
#define COLOR_DARK_BLUE     0x001F
#define COLOR_MAROON        0x7800
#define COLOR_YELLOW        ST77XX_YELLOW
#define COLOR_RED           ST77XX_RED
#define COLOR_DARK_GREEN    0x03E0
#define COLOR_BLACK         ST77XX_BLACK
#define COLOR_WHITE         ST77XX_WHITE
#define COLOR_GREY          0x8410

// =================== OBJECTS and VARS ===================
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);
const char* menuItems[] = {"Cycles", "Device Ping", "Settings", "Demo"};

static sbs_data_t lastSbsData;
static int8_t oldMainMenuSelection = -1;
static int8_t oldSettingsMenuSelection = -1;
static uint8_t lastCyclesLeft = 255, lastCyclesTotal = 255;
static bool lastIsRunning = false;

// =================== INITIALIZATION ===================
void initDisplay() {
  tft.init(240, 320);
  tft.setRotation(0);
  tft.fillScreen(COLOR_BLACK);
  tft.invertDisplay(false);
}

// =================== HELPERS ===================
void drawField(int16_t x, int16_t y, int16_t w, int16_t h, const char* value, uint16_t bgColor, uint16_t textColor, uint8_t textSize) {
    tft.fillRect(x, y, w, h, bgColor);
    tft.drawRect(x, y, w, h, COLOR_GREY);
    tft.setCursor(x + 5, y + (h - 8 * textSize) / 2);
    tft.setTextColor(textColor);
    tft.setTextSize(textSize);
    tft.print(value);
}

void drawStatusBits(int16_t x, int16_t y, uint16_t statusWord) {
    for (int i = 0; i < 16; i++) {
        uint16_t color = (statusWord >> i) & 0x01 ? COLOR_RED : COLOR_DARK_GREEN;
        tft.fillRect(x + i * 15, y, 13, 13, color);
        tft.drawRect(x + i * 15, y, 13, 13, COLOR_GREY);
    }
}

// =================== SCREEN DRAWING FUNCTIONS ===================

void drawMainMenu(int8_t selectedItem, bool fullRedraw) {
  if (fullRedraw) {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextSize(3);
    for (int i = 0; i < 4; i++) {
      if (i == selectedItem) tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
      else tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
      tft.setCursor(20, 80 + i * 50);
      tft.println(menuItems[i]);
    }
  } else {
    if (selectedItem != oldMainMenuSelection) {
      tft.setTextSize(3);
      tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
      tft.setCursor(20, 80 + oldMainMenuSelection * 50);
      tft.println(menuItems[oldMainMenuSelection]);
      tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
      tft.setCursor(20, 80 + selectedItem * 50);
      tft.println(menuItems[selectedItem]);
    }
  }
  oldMainMenuSelection = selectedItem;
}

void drawCyclesScreen(const sbs_data_t& sbsData, uint8_t cyclesLeft, uint8_t cyclesTotal, bool isRunning, bool fullRedraw) {
    char buf[40];

    if (fullRedraw) {
        tft.fillScreen(COLOR_BLACK);
        tft.setTextColor(COLOR_WHITE);
        tft.setTextSize(1);
        tft.setCursor(2, 285); tft.print("Status:");
        tft.setCursor(2, 305); tft.print("Mode:");
        // Force redraw of all fields on next pass
        memset(&lastSbsData, 0xFF, sizeof(sbs_data_t));
        lastCyclesLeft = 255; lastCyclesTotal = 255; lastIsRunning = !isRunning;
    }

    if (cyclesLeft != lastCyclesLeft || cyclesTotal != lastCyclesTotal || isRunning != lastIsRunning) {
        sprintf(buf, isRunning ? "CYCLES %d/%d LEFT" : "CYCLES %d", cyclesLeft, cyclesTotal);
        tft.fillRect(0, 0, 240, 30, isRunning ? COLOR_DARK_GREEN : COLOR_BLACK);
        tft.setCursor(5, 8); tft.setTextSize(2); tft.setTextColor(COLOR_WHITE); tft.print(buf);
    }

    if (sbsData.voltage != lastSbsData.voltage) {
        sprintf(buf, "%1.2f V", sbsData.voltage / 1000.0);
        drawField(0, 32, 120, 30, buf, COLOR_DARK_BLUE, COLOR_WHITE, 2);
    }
    if (sbsData.current != lastSbsData.current) {
        sprintf(buf, "%1.2f A", sbsData.current / 1000.0);
        drawField(120, 32, 120, 30, buf, COLOR_MAROON, COLOR_WHITE, 2);
    }
    if (sbsData.temperature != lastSbsData.temperature) {
        sprintf(buf, "%1.2f C", (sbsData.temperature / 10.0) - 273.15);
        drawField(0, 64, 120, 30, buf, COLOR_YELLOW, COLOR_BLACK, 2);
    }
    if (sbsData.maxError != lastSbsData.maxError) {
        sprintf(buf, "%d %%", sbsData.maxError);
        drawField(120, 64, 120, 30, buf, COLOR_RED, COLOR_WHITE, 2);
    }
    if (sbsData.remainingCapacity != lastSbsData.remainingCapacity) {
        sprintf(buf, "Rem. Cap: %d mAh", sbsData.remainingCapacity);
        drawField(0, 96, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);
    }
    if (sbsData.fullChargeCapacity != lastSbsData.fullChargeCapacity) {
        sprintf(buf, "Full Cap: %d mAh", sbsData.fullChargeCapacity);
        drawField(0, 128, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);
    }
    if (sbsData.chargingVoltage != lastSbsData.chargingVoltage || sbsData.chargingCurrent != lastSbsData.chargingCurrent) {
        sprintf(buf, "Chg: %1.2fV %1.2fA", sbsData.chargingVoltage / 1000.0, sbsData.chargingCurrent / 1000.0);
        drawField(0, 160, 240, 30, buf, COLOR_DARK_GREEN, COLOR_WHITE, 2);
    }
    if (sbsData.cycleCount != lastSbsData.cycleCount) {
        sprintf(buf, "Cycles: %d", sbsData.cycleCount);
        drawField(0, 192, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);
    }
    if (sbsData.designVoltage != lastSbsData.designVoltage || sbsData.designCapacity != lastSbsData.designCapacity) {
        sprintf(buf, "Design: %1.2fV %dmAh", sbsData.designVoltage / 1000.0, sbsData.designCapacity);
        drawField(0, 224, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);
    }
    if (sbsData.cellVoltage1 != lastSbsData.cellVoltage1) {
        sprintf(buf, "%1.2fV", sbsData.cellVoltage1 / 1000.0);
        drawField(0, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);
    }
    if (sbsData.cellVoltage2 != lastSbsData.cellVoltage2) {
        sprintf(buf, "%1.2fV", sbsData.cellVoltage2 / 1000.0);
        drawField(60, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);
    }
    if (sbsData.cellVoltage3 != lastSbsData.cellVoltage3) {
        sprintf(buf, "%1.2fV", sbsData.cellVoltage3 / 1000.0);
        drawField(120, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);
    }
    if (sbsData.cellVoltage4 != lastSbsData.cellVoltage4) {
        sprintf(buf, "%1.2fV", sbsData.cellVoltage4 / 1000.0);
        drawField(180, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);
    }
    if (sbsData.batteryStatus != lastSbsData.batteryStatus) {
        drawStatusBits(45, 283, sbsData.batteryStatus);
    }
    if (sbsData.batteryMode != lastSbsData.batteryMode) {
        drawStatusBits(45, 303, sbsData.batteryMode);
    }

    lastSbsData = sbsData;
    lastCyclesLeft = cyclesLeft;
    lastCyclesTotal = cyclesTotal;
    lastIsRunning = isRunning;
}

void drawDevicePingScreen(const sbs_data_t& sbsData, bool isConnected, bool fullRedraw) {
    if (isConnected) {
        drawCyclesScreen(sbsData, 0, 0, true, fullRedraw);
    } else {
        if (fullRedraw) {
            tft.fillScreen(COLOR_BLACK);
            drawField(10, 140, 220, 40, "Device not connected", COLOR_RED, COLOR_WHITE, 2);
        }
    }
}

void drawSettingsScreen(int8_t selectedItem, bool editMode, bool fullRedraw) {
    char buf[30];
    Settings& s = getSettings();
    const char* labels[] = { "Cycles", "Ping Timeout", "Pause Chg", "Pause Dis", "SMBus Timeout", "Demo Chg", "Demo Dis", "Serial Out", "RESET", "SAVE"  };

    if (fullRedraw) {
        tft.fillScreen(COLOR_BLACK);
        tft.setTextSize(2);
        for (int i = 0; i < 10; i++) {
            uint16_t values[] = { s.cyclesCount, s.devicePingTimeout, s.pauseAfterCharge, s.pauseAfterDischarge, s.smbusReadTimeout, s.demoChargeTime, s.demoDischargeTime, s.serialOutputTimeout };
             uint16_t fgColor = (i == selectedItem) ? COLOR_BLACK : COLOR_WHITE;
             uint16_t bgColor = (i == selectedItem) ? (editMode ? COLOR_RED : COLOR_WHITE) : COLOR_BLACK;

             tft.setCursor(5, 5 + i * 32);
             tft.setTextColor(fgColor, bgColor);
             if (i < 8) sprintf(buf, "%-15s %-3d", labels[i], values[i]);
             else sprintf(buf, "%s", labels[i]);
             tft.print(buf);
        }
    } else {
        uint16_t values[] = { s.cyclesCount, s.devicePingTimeout, s.pauseAfterCharge, s.pauseAfterDischarge, s.smbusReadTimeout, s.demoChargeTime, s.demoDischargeTime, s.serialOutputTimeout };
        tft.setTextSize(2);

        if (selectedItem != oldSettingsMenuSelection) {
            uint16_t old_values[] = { s.cyclesCount, s.devicePingTimeout, s.pauseAfterCharge, s.pauseAfterDischarge, s.smbusReadTimeout, s.demoChargeTime, s.demoDischargeTime, s.serialOutputTimeout };
            tft.setCursor(5, 5 + oldSettingsMenuSelection * 32);
            tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
            if (oldSettingsMenuSelection < 8) sprintf(buf, "%-15s %-3d", labels[oldSettingsMenuSelection], old_values[oldSettingsMenuSelection]);
            else sprintf(buf, "%s", labels[oldSettingsMenuSelection]);
            tft.print(buf);
        }

        uint16_t fgColor = COLOR_BLACK;
        uint16_t bgColor = editMode ? COLOR_RED : COLOR_WHITE;
        tft.setCursor(5, 5 + selectedItem * 32);
        tft.setTextColor(fgColor, bgColor);
        if (selectedItem < 8) sprintf(buf, "%-15s %-3d", labels[selectedItem], values[selectedItem]);
        else sprintf(buf, "%s", labels[selectedItem]);
        tft.print(buf);
    }
    oldSettingsMenuSelection = selectedItem;
}
