#ifndef direct_pin_read_h_
#define direct_pin_read_h_

// This file is designed to be overriden by a platform-specific
// implementation.  If no platform specific version is found,
// the C-language digitalRead function is used.  The C version
// is slow, but it will work on any Arduino compatible board.
//
// To create a platform-specific version, create a file in your
// platform's ".../cores/arduino" folder called "direct_pin_read.h"
// which contains a #define for DIRECT_PIN_READ(pin).
//
// See the teensy/direct_pin_read.h for a simple example.

#if !defined(DIRECT_PIN_READ)
#define DIRECT_PIN_READ(pin) digitalRead(pin)
#endif

#endif
