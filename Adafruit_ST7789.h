/**************************************************************************/
/*!
    @file    Adafruit_ST7789.h

    This is the library for the Adafruit ST7789 IPS display products
    ----> http://www.adafruit.com/products/358
    ----> http://www.adafruit.com/products/368
    ----> http://www.adafruit.com/products/369

    Check out the links above for our tutorials and wiring diagrams
    These displays use SPI to communicate, 4 or 5 pins are required to
    interface (RST is optional)
    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section author Written by Limor Fried/Ladyada for Adafruit Industries.
    @section license BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

#ifndef _ADAFRUIT_ST7789H_
#define _ADAFRUIT_ST7789H_

#include "Adafruit_ST77xx.h"

// clang-format off
// -------------------------------------------------------------------------
// ST7789 registers
// -------------------------------------------------------------------------
#define ST7789_SLEEP_IN         0x10    ///< Enter Sleep Mode
#define ST7789_SLEEP_OUT        0x11    ///< Sleep Out
#define ST7789_INVOFF           0x20    ///< Display Inversion Off
#define ST7789_INVON            0x21    ///< Display Inversion On
#define ST7789_DISPOFF          0x28    ///< Display Off
#define ST7789_DISPON           0x29    ///< Display On
#define ST7789_CASET            0x2A    ///< Column Address Set
#define ST7789_RASET            0x2B    ///< Row Address Set
#define ST7789_RAMWR            0x2C    ///< RAM Write
#define ST7789_RAMRD            0x2E    ///< RAM Read
#define ST7789_TEON             0x35    ///< Tearing Effect Line ON
#define ST7789_MADCTL           0x36    ///< Memory Data Access Control
#define ST7789_COLMOD           0x3A    ///< Interface Pixel Format

#define ST7789_WRITEDISBV       0x51    ///< Write Display Brightness
#define ST7789_WRITECTRLD       0x53    ///< Write CTRL Display

#define ST7789_RAMCTRL          0xB0    ///< RAM Control
#define ST7789_PORCTRL          0xB2    ///< Porch Control
#define ST7789_GCTRL            0xB7    ///< Gate Control
#define ST7789_VCOMS            0xBB    ///< VCOM Setting
#define ST7789_LCMCTRL          0xC0    ///< LCM Control
#define ST7789_VDVVRHEN         0xC2    ///< VDV and VRH Command Enable
#define ST7789_VRHS             0xC3    ///< VRH Set
#define ST7789_VDVS             0xC4    ///< VDV Set
#define ST7789_FRCTRL2          0xC6    ///< FR Control 2
#define ST7789_PWCTRL1          0xD0    ///< Power Control 1
#define ST7789_PVGAMCTRL        0xE0    ///< Positive Voltage Gamma Control
#define ST7789_NVGAMCTRL        0xE1    ///< Negative Voltage Gamma Control
// clang-format on

/**************************************************************************/
/*!
    @brief  Class that stores state and functions for interacting with
            ST7789 displays.
*/
/**************************************************************************/
class Adafruit_ST7789 : public Adafruit_ST77xx {
public:
  Adafruit_ST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                  int8_t rst = -1);
  Adafruit_ST7789(int8_t cs, int8_t dc, int8_t rst);
#if !defined(ESP8266)
  Adafruit_ST7789(SPIClass *spiClass, int8_t cs, int8_t dc, int8_t rst);
#endif // end !ESP8266

  void setRotation(uint8_t m);
  void init(uint16_t width, uint16_t height, uint8_t spiMode = SPI_MODE0);

protected:
  uint8_t _colstart2 = 0, ///< Offset from the left
      _rowstart2 = 0;     ///< Offset from the top

private:
  void sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                   uint8_t numDataBytes);
  void sendCommand(uint8_t commandByte, uint8_t *dataBytes,
                   uint8_t numDataBytes);
};

#endif // _ADAFRUIT_ST7789H_
