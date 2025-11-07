#include "display.h"
#include "config.h"
#include "settings.h"

// =================== COLOR DEFINITIONS ===================
#define COLOR_DARK_BLUE     0x001F
#define COLOR_MAROON        0x7800
#define COLOR_YELLOW        0xFFE0
#define COLOR_RED           0xF800
#define COLOR_DARK_GREEN    0x03E0
#define COLOR_BLACK         ILI9341_BLACK
#define COLOR_WHITE         ILI9-341_WHITE
#define COLOR_GREY          0x8410

// =================== OBJECTS ===================
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN);

// =================== MENU ITEMS ===================
const char* menuItems[] = {"Cycles", "Device Ping", "Settings", "Demo"};

// =================== INITIALIZATION ===================
void initDisplay() {
  tft.begin();
  tft.setRotation(1); // Portrait orientation
  tft.fillScreen(COLOR_BLACK);
}

// =================== SCREEN DRAWING FUNCTIONS ===================

void drawMainMenu(int8_t selectedItem) {
  tft.fillScreen(COLOR_BLACK);
  tft.setTextSize(3);

  for (int i = 0; i < 4; i++) {
    // Highlight the selected item by inverting colors
    if (i == selectedItem) {
      tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
    } else {
      tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
    }
    tft.setCursor(50, 80 + i * 50);
    tft.println(menuItems[i]);
  }
}

// Helper function to draw a data field with a border and background
void drawField(int16_t x, int16_t y, int16_t w, int16_t h, const char* value, uint16_t bgColor, uint16_t textColor, uint8_t textSize) {
    tft.fillRect(x, y, w, h, bgColor);
    tft.drawRect(x, y, w, h, COLOR_GREY);
    // Center text vertically
    tft.setCursor(x + 5, y + (h - 8 * textSize) / 2);
    tft.setTextColor(textColor);
    tft.setTextSize(textSize);
    tft.print(value);
}

// Helper function to draw status bits as colored squares
void drawStatusBits(int16_t x, int16_t y, uint16_t statusWord) {
    for (int i = 0; i < 16; i++) {
        // Bit is 1: RED, Bit is 0: GREEN
        uint16_t color = (statusWord >> i) & 0x01 ? COLOR_RED : COLOR_DARK_GREEN;
        tft.fillRect(x + i * 15, y, 13, 13, color);
        tft.drawRect(x + i * 15, y, 13, 13, COLOR_GREY);
    }
}

void drawCyclesScreen(const sbs_data_t& sbsData, uint8_t cyclesLeft, uint8_t cyclesTotal, bool isRunning) {
    char buf[30]; // Buffer for formatting strings
    tft.fillScreen(COLOR_BLACK);

    // Top status bar for cycles
    tft.setTextColor(COLOR_WHITE);
    tft.setTextSize(2);
    tft.setCursor(5, 8);
    // Format: CYCLES 2/10 LEFT or CYCLES 10 (if not running)
    if (isRunning) {
        sprintf(buf, "CYCLES %d/%d LEFT", cyclesLeft, cyclesTotal);
    } else {
        sprintf(buf, "CYCLES %d", cyclesTotal);
    }
    tft.fillRect(0, 0, 240, 30, isRunning ? COLOR_DARK_GREEN : COLOR_BLACK);
    tft.setCursor(5, 8);
    tft.print(buf);


    // If data is invalid (e.g., battery disconnected), show a warning and stop.
    if (!sbsData.dataValid && isRunning) {
        drawField(10, 140, 220, 40, "N/A - NO CONNECTION", COLOR_RED, COLOR_WHITE, 2);
        return;
    }

    // Voltage | Current
    sprintf(buf, "%1.2f V", sbsData.voltage / 1000.0);
    drawField(0, 32, 120, 30, buf, COLOR_DARK_BLUE, COLOR_WHITE, 2);
    sprintf(buf, "%1.2f A", sbsData.current / 1000.0);
    drawField(120, 32, 120, 30, buf, COLOR_MAROON, COLOR_WHITE, 2);

    // Temperature | MaxError
    sprintf(buf, "%1.2f C", (sbsData.temperature / 10.0) - 273.15);
    drawField(0, 64, 120, 30, buf, COLOR_YELLOW, COLOR_BLACK, 2);
    sprintf(buf, "%d %%", sbsData.maxError);
    drawField(120, 64, 120, 30, buf, COLOR_RED, COLOR_WHITE, 2);

    // Remaining Capacity
    sprintf(buf, "Rem. Cap: %d mAh", sbsData.remainingCapacity);
    drawField(0, 96, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);

    // Full Charge Capacity
    sprintf(buf, "Full Cap: %d mAh", sbsData.fullChargeCapacity);
    drawField(0, 128, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);

    // Charging Voltage / Current
    sprintf(buf, "Chg: %1.2fV %1.2fA", sbsData.chargingVoltage / 1000.0, sbsData.chargingCurrent / 1000.0);
    drawField(0, 160, 240, 30, buf, COLOR_DARK_GREEN, COLOR_WHITE, 2);

    // Cycle Count
    sprintf(buf, "Cycles: %d", sbsData.cycleCount);
    drawField(0, 192, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);

    // Design Voltage / Capacity
    sprintf(buf, "Design: %1.2fV %dmAh", sbsData.designVoltage / 1000.0, sbsData.designCapacity);
    drawField(0, 224, 240, 30, buf, COLOR_BLACK, COLOR_WHITE, 2);

    // Cell Voltages
    sprintf(buf, "%1.2fV", sbsData.cellVoltage1 / 1000.0);
    drawField(0, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);
    sprintf(buf, "%1.2fV", sbsData.cellVoltage2 / 1000.0);
    drawField(60, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);
    sprintf(buf, "%1.2fV", sbsData.cellVoltage3 / 1000.0);
    drawField(120, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);
    sprintf(buf, "%1.2fV", sbsData.cellVoltage4 / 1000.0);
    drawField(180, 256, 60, 25, buf, COLOR_DARK_BLUE, COLOR_WHITE, 1);

    // Battery Status & Mode bits
    tft.setTextColor(COLOR_WHITE);
    tft.setTextSize(1);
    tft.setCursor(2, 285);
    tft.print("Status:");
    drawStatusBits(45, 283, sbsData.batteryStatus);
    tft.setCursor(2, 305);
    tft.print("Mode:");
    drawStatusBits(45, 303, sbsData.batteryMode);
}

void drawDevicePingScreen(const sbs_data_t& sbsData, bool isConnected) {
    if (isConnected) {
        // Reuse the cycles screen to display all available data
        drawCyclesScreen(sbsData, 0, 0, true);
    } else {
        tft.fillScreen(COLOR_BLACK);
        drawField(10, 140, 220, 40, "Device not connected", COLOR_RED, COLOR_WHITE, 2);
    }
}

void drawSettingsScreen(int8_t selectedItem, bool editMode) {
    tft.fillScreen(COLOR_BLACK);
    tft.setTextSize(2);
    Settings& s = getSettings();
    char buf[30];

    const char* labels[] = {
        "Cycles Count", "Ping Timeout (s)", "Pause Chg (min)", "Pause Dis (min)",
        "SMBus Timeout (s)", "Demo Chg (s)", "Demo Dis (s)", "Serial Timeout (s)",
        "RESET", "RETURN"
    };

    uint16_t values[] = {
        s.cyclesCount, s.devicePingTimeout, s.pauseAfterCharge, s.pauseAfterDischarge,
        s.smbusReadTimeout, s.demoChargeTime, s.demoDischargeTime, s.serialOutputTimeout
    };

    for (int i = 0; i < 10; i++) {
        uint16_t fgColor = COLOR_WHITE;
        uint16_t bgColor = COLOR_BLACK;

        if (i == selectedItem) {
            bgColor = COLOR_WHITE;
            fgColor = COLOR_BLACK;
            if (editMode) {
                bgColor = COLOR_RED; // Indicate editing mode with a red background
            }
        }

        tft.setCursor(5, 5 + i * 32);
        tft.setTextColor(fgColor, bgColor);

        if (i < 8) { // Settings with values
            sprintf(buf, "%-20s %-3d", labels[i], values[i]);
        } else { // RESET and RETURN
            sprintf(buf, "%s", labels[i]);
        }
        tft.print(buf);
    }
}
