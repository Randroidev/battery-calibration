#ifndef interrupt_pins_h_
#define interrupt_pins_h_

// This file is designed to be overriden by a platform-specific
// implementation.  If this version is used, the library will
// not be able to automatically use interrupts.  You will need
// to call Encoder::update() manually from a timer interrupt
// or in your main loop.
//
// See the teensy/interrupt_pins.h for an example of how to
// automatically use interrupts.

#define CORE_NUM_INTERRUPT 0
#define digitalPinToInterrupt(p) NOT_AN_INTERRUPT

#endif
