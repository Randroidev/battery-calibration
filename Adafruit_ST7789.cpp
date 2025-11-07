/**************************************************************************/
/*!
    @file    Adafruit_ST7789.cpp

    @mainpage Adafruit ST7789 IPS Display Library

    @section intro_sec Introduction

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

    @section author Author

    Written by Limor Fried/Ladyada for Adafruit Industries.

    @section license License

    BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

#include "Adafruit_ST7789.h"
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST7789 driver with hardware SPI
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  mosi  SPI MOSI pin #
    @param  sclk  SPI Clock pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ST7789::Adafruit_ST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                               int8_t rst)
    : Adafruit_ST77xx(240, 320, cs, dc, mosi, sclk, rst) {}

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST7789 driver with hardware SPI
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ST7789::Adafruit_ST7789(int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_ST77xx(240, 320, cs, dc, rst) {}

#if !defined(ESP8266)
/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST7789 driver with selectable hardware SPI
    @param  spiClass  Pointer to an SPI device to use
    @param  cs        Chip select pin #
    @param  dc        Data/Command pin #
    @param  rst       Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ST7789::Adafruit_ST7789(SPIClass *spiClass, int8_t cs, int8_t dc,
                               int8_t rst)
    : Adafruit_ST77xx(240, 320, spiClass, cs, dc, rst) {}
#endif // end !ESP8266

/**************************************************************************/
/*!
  @brief  SPI displays set an address window rectangle for blitting pixels
  @param  x  Top left corner x coordinate
  @param  y  Top left corner y coordinate
  @param  w  Width of window
  @param  h  Height of window
**************************************************************************/
void Adafruit_ST7789::setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                                   uint16_t h) {
  x += _xstart;
  y += _ystart;
  uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
  uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

  writeCommand(ST7789_CASET); // Column addr set
  SPI_WRITE32(xa);

  writeCommand(ST7789_RASET); // Row addr set
  SPI_WRITE32(ya);

  writeCommand(ST7789_RAMWR); // write to RAM
}

/**************************************************************************/
/*!
    @brief  Initialize ST7789 chip
    Connects to the ST7789 over SPI and sends initialization procedure commands
    @param  width     Display width in pixels
    @param  height    Display height in pixels
    @param  spiMode   SPI bus mode, one of SPI_MODE0, SPI_MODE1, SPI_MODE2,
                      SPI_MODE3
*/
/**************************************************************************/
void Adafruit_ST7789::init(uint16_t width, uint16_t height, uint8_t spiMode) {
  // ST7789 is a 240x320 display, but the physical orientation can vary.
  // The user provides the dimensions in the orientation they're holding the
  // device, and this library can rotate the display to match.
  _width = width;
  _height = height;
  _xstart = (320 - width) / 2;
  _ystart = (240 - height) / 2;

  initSPI(spiMode);

  displayInit(generic_st7789);

  setRotation(0);
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The rotation index, 0-3.
*/
/**************************************************************************/
void Adafruit_ST7789::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  switch (rotation) {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    _xstart = (320 - _width) / 2;
    _ystart = (240 - _height) / 2;
    break;
  case 1:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _xstart = (240 - _width) / 2;
    _ystart = (320 - _height) / 2;
    break;
  case 2:
    madctl = ST77XX_MADCTL_RGB;
    _xstart = (320 - _width) / 2;
    _ystart = (240 - _height) / 2;
    break;
  case 3:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    _xstart = (240 - _width) / 2;
    _ystart = (320 - _height) / 2;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}
