/**************************************************************************/
/*!
    @file    Adafruit_ST77xx.cpp

    @mainpage Adafruit ST7735/ST7789 Library

    @section intro_sec Introduction

    This is a library for the Adafruit ST7735 and ST7789 displays.

    @section author Author

    Written by Limor Fried/Ladyada for Adafruit Industries.

    @section license License

    BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/

#include "Adafruit_ST77xx.h"
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>

#ifndef_BV
#define _BV(bit) (1 << (bit))
#endif

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST77xx driver with hardware SPI
    @param  w     Display width in pixels
    @param  h     Display height in pixels
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  mosi  SPI MOSI pin #
    @param  sclk  SPI Clock pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ST77xx::Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t cs, int8_t dc,
                               int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_SPITFT(w, h, cs, dc, mosi, sclk, rst) {}

/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST77xx driver with hardware SPI
    @param  w     Display width in pixels
    @param  h     Display height in pixels
    @param  cs    Chip select pin #
    @param  dc    Data/Command pin #
    @param  rst   Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ST77xx::Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t cs, int8_t dc,
                               int8_t rst)
    : Adafruit_SPITFT(w, h, cs, dc, rst) {}

#if !defined(ESP8266)
/**************************************************************************/
/*!
    @brief  Instantiate Adafruit ST77xx driver with selectable hardware SPI
    @param  w         Display width in pixels
    @param  h         Display height in pixels
    @param  spiClass  Pointer to an SPI device to use
    @param  cs        Chip select pin #
    @param  dc        Data/Command pin #
    @param  rst       Reset pin # (optional, pass -1 if unused)
*/
/**************************************************************************/
Adafruit_ST77xx::Adafruit_ST77xx(uint16_t w, uint16_t h, SPIClass *spiClass,
                               int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_SPITFT(w, h, spiClass, cs, dc, rst) {}
#endif // end !ESP8266

/**************************************************************************/
/*!
    @brief  Companion code to the initiliazation tables. Reads and issues
            a series of LCD commands stored in PROGMEM byte array.
    @param  addr  Flash memory address of command list table.
*/
/**************************************************************************/
void Adafruit_ST77xx::displayInit(const uint8_t *addr) {

  uint8_t numCommands, cmd, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++); // Number of commands to follow
  while (numCommands--) {              // For each command...
    cmd = pgm_read_byte(addr++);       // Read command
    numArgs = pgm_read_byte(addr++);   // Number of args to follow
    ms = numArgs & 0x80;               // If hibit set, delay follows args
    numArgs &= ~0x80;                  // Mask out delay bit
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;

    if (ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if (ms == 255)
        ms = 500; // If 255, delay for 500 ms
      delay(ms);
    }
  }
}

/**************************************************************************/
/*!
    @brief   Initialize a ST77xx chip, trying to determine driver type.
    @param   freq  SPI frequency to use.
*/
/**************************************************************************/
// Based on code from github user @astuder.  Thanks!
void Adafruit_ST77xx::init(uint32_t freq) {

  initSPI(freq);

  // So, we have a variety of ST77xx chips documented. The following sequence
  // should, in theory, differentiate them by reading the display ID.
  // Unfortunately, display IDs are not a reliable way to differentiate chips,
  // so we're avoiding that entirely and just providing init procedures for
  // the specific displays we carry.
}

/**************************************************************************/
/*!
  @brief  SPI displays set an address window rectangle for blitting pixels
  @param  x  Top left corner x coordinate
  @param  y  Top left corner y coordinate
  @param  w  Width of window
  @param  h  Height of window
**************************************************************************/
void Adafruit_ST77xx::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t w,
                                    uint16_t h) {
  uint16_t x1 = x0 + w - 1;
  uint16_t y1 = y0 + h - 1;
  writeCommand(ST77XX_CASET);
  SPI_WRITE16(x0);
  SPI_WRITE16(x1);
  writeCommand(ST77XX_RASET);
  SPI_WRITE16(y0);
  SPI_WRITE16(y1);
  writeCommand(ST77XX_RAMWR);
}

/**************************************************************************/
/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  r  The rotation index, 0-3.
*/
/**************************************************************************/
void Adafruit_ST77xx::setRotation(uint8_t r) {
  uint8_t madctl = 0;
  rotation = r & 3; // can't be higher than 3

  switch (rotation) {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    break;
  case 1:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    break;
  case 2:
    madctl = ST77XX_MADCTL_RGB;
    break;
  case 3:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    break;
  }
  sendCommand(ST77XX_MADCTL, &madctl, 1);
}

/**************************************************************************/
/*!
    @brief  Sends a command byte to the display.
    @param  commandByte The command byte to send.
    @param  dataBytes   A pointer to the data bytes to send.
    @param  numDataBytes  The number of data bytes to send.
*/
/**************************************************************************/
void Adafruit_ST77xx::sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  SPI_BEGIN_TRANSACTION();
  DC_LOW();
  spiWrite(commandByte);
  DC_HIGH();
  for (int i = 0; i < numDataBytes; i++) {
    spiWrite(dataBytes[i]);
  }
  SPI_END_TRANSACTION();
}

/**************************************************************************/
/*!
    @brief  Read 8 bits of data from display register.
    @param  commandByte The command byte to send.
    @param  index       The byte index to read (0-3).
    @return Unsigned 8-bit data read from display register.
*/
/**************************************************************************/
uint8_t Adafruit_ST77xx::readcommand8(uint8_t commandByte, uint8_t index) {
  uint32_t data = 0;
  SPI_BEGIN_TRANSACTION();
  DC_LOW();
  spiWrite(commandByte);
  DC_HIGH();
  do {
    data = spiRead();
  } while (index--); // Discard bytes up to index'th
  SPI_END_TRANSACTION();

  return data;
}

/**************************************************************************/
/*!
    @brief Invert the display colors.
    @param i True to invert, false to restore.
*/
/**************************************************************************/
void Adafruit_ST77xx::invertDisplay(bool i) {
  writeCommand(i ? ST77XX_INVON : ST77XX_INVOFF);
}
