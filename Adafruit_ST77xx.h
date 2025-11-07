/**************************************************************************/
/*!
    @file    Adafruit_ST77xx.h

    @mainpage Adafruit ST7735/ST7789 Library

    @section intro_sec Introduction

    This is a library for the Adafruit ST7735 and ST7789 displays.

    @section author Author

    Written by Limor Fried/Ladyada for Adafruit Industries.

    @section license License

    BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

#ifndef _ADAFRUIT_ST77XXH_
#define _ADAFRUIT_ST77XXH_

#include "Arduino.h"
#include "Print.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <SPI.h>

// clang-format off
// -------------------------------------------------------------------------
// ST77xx commands
// -------------------------------------------------------------------------
#define ST77XX_NOP              0x00
#define ST77XX_SWRESET          0x01
#define ST77XX_RDDID            0x04
#define ST77XX_RDDST            0x09

#define ST77XX_SLPIN            0x10
#define ST77XX_SLPOUT           0x11
#define ST77XX_PTLON            0x12
#define ST77XX_NORON            0x13

#define ST77XX_INVOFF           0x20
#define ST77XX_INVON            0x21
#define ST77XX_DISPOFF          0x28
#define ST77XX_DISPON           0x29
#define ST77XX_CASET            0x2A
#define ST77XX_RASET            0x2B
#define ST77XX_RAMWR            0x2C
#define ST77XX_RAMRD            0x2E

#define ST77XX_PTLAR            0x30
#define ST77XX_COLMOD           0x3A
#define ST77XX_MADCTL           0x36

#define ST77XX_MADCTL_MY        0x80
#define ST77XX_MADCTL_MX        0x40
#define ST77XX_MADCTL_MV        0x20
#define ST77XX_MADCTL_ML        0x10
#define ST77XX_MADCTL_RGB       0x00

#define ST77XX_RDID1            0xDA
#define ST77XX_RDID2            0xDB
#define ST77XX_RDID3            0xDC
#define ST77XX_RDID4            0xDD

// Color definitions
#define ST77XX_BLACK            0x0000
#define ST77XX_WHITE            0xFFFF
#define ST77XX_RED              0xF800
#define ST77XX_GREEN            0x07E0
#define ST77XX_BLUE             0x001F
#define ST77XX_CYAN             0x07FF
#define ST77XX_MAGENTA          0xF81F
#define ST77XX_YELLOW           0xFFE0
#define ST77XX_ORANGE           0xFC00
// clang-format on

/**************************************************************************/
/*!
    @brief  Parent class for ST77xx displays
*/
/**************************************************************************/
class Adafruit_ST77xx : public Adafruit_SPITFT {
public:
  Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t cs, int8_t dc, int8_t mosi,
                  int8_t sclk, int8_t rst = -1);
  Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t cs, int8_t dc,
                  int8_t rst = -1);
#if !defined(ESP8266)
  Adafruit_ST77xx(uint16_t w, uint16_t h, SPIClass *spiClass, int8_t cs,
                  int8_t dc, int8_t rst = -1);
#endif // end !ESP8266

  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void setRotation(uint8_t r);
  void invertDisplay(bool i);

protected:
  uint16_t _xstart = 0, ///< The x start address in RAM
      _ystart = 0;        ///< The y start address in RAM

  const uint8_t *generic_st7789 =
      (const uint8_t *)"\x01\x80\x96"     // SWRESET and Delay 150ms
      "\x11\x80\xFF"     // SLPOUT and Delay 500ms
      "\x3A\x81\x55\x0A" // COLMOD and Delay 10ms
      "\x36\x01\x00"     // MADCTL
      "\x21\x80\x0A"     // INVON and Delay 10ms
      "\x13\x80\x0A"     // NORON and Delay 10ms
      "\x29\x80\xFF"     // DISPON and Delay 500ms
      "\x00";            ///< End of list marker

  void displayInit(const uint8_t *addr);
  void sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                   uint8_t numDataBytes);
  uint8_t readcommand8(uint8_t commandByte, uint8_t index = 0);
};

#endif // _ADAFRUIT_ST77XXH_
