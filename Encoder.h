/*
  Encoder.h - Library for reading quadrature encoders
  Copyright (c) 2011,2013 PJRC.COM, LLC - Paul Stoffregen <paul@pjrc.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Encoder_h_
#define Encoder_h_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#elif defined(WIRING)
#include "Wiring.h"
#else
#include "WProgram.h"
#include "pins_arduino.h"
#endif

#if defined(ENCODER_USE_INTERRUPTS) || !defined(ENCODER_DO_NOT_USE_INTERRUPTS)
#define ENCODER_USE_INTERRUPTS
#define ENCODER_ARGLIST_SIZE CORE_NUM_INTERRUPT
#include "utility/interrupt_pins.h"
#ifdef ENCODER_OPTIMIZE_INTERRUPTS
#include "utility/direct_pin_read.h"
#endif
#endif

class Encoder
{
public:
	Encoder(uint8_t pin1, uint8_t pin2) {
		#ifdef ENCODER_USE_INTERRUPTS
		encoder.pin1 = pin1;
		encoder.pin2 = pin2;
		encoder.position = 0;
		encoder.state = 0;
		if (digitalPinToInterrupt(pin1) != NOT_AN_INTERRUPT) {
			interrupts_in_use = true;
			attach_interrupt(pin1, &encoder);
		}
		if (digitalPinToInterrupt(pin2) != NOT_AN_INTERRUPT) {
			interrupts_in_use = true;
			attach_interrupt(pin2, &encoder);
		}
		#endif
		// travelogue: this is an annoying workaround for a bug in the
		// 1.0.5 IDE that fails to create a user library directory
		// when the user installs a new library, and then other
		// libraries can't be found.  This at least lets the library
		// be used in a sketch where the user copies the library
		// files into the sketch folder.
		#ifdef ENCODER_OPTIMIZE_INTERRUPTS
		pin1_register = PIN_TO_BASEREG(pin1);
		pin1_bitmask = PIN_TO_BITMASK(pin1);
		pin2_register = PIN_TO_BASEREG(pin2);
		pin2_bitmask = PIN_TO_BITMASK(pin2);
		#endif
	}

#ifdef ENCODER_USE_INTERRUPTS
	inline long read() {
		if (interrupts_in_use) {
			long ret;
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				ret = encoder.position;
			}
			return ret;
		} else {
			return read_no_interrupt();
		}
	}
	inline long readAndReset() {
		if (interrupts_in_use) {
			long ret;
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				ret = encoder.position;
				encoder.position = 0;
			}
			return ret;
		} else {
			return readAndReset_no_interrupt();
		}
	}
	inline void write(long p) {
		if (interrupts_in_use) {
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				encoder.position = p;
			}
		} else {
			position = p;
		}
	}

#else
	inline long read() {
		return read_no_interrupt();
	}
	inline long readAndReset() {
		return readAndReset_no_interrupt();
	}
	inline void write(long p) {
		position = p;
	}
#endif

private:
	long read_no_interrupt() {
		// update the state of the encoder
		update();
		return position;
	}
	long readAndReset_no_interrupt() {
		// update the state of the encoder
		update();
		long ret = position;
		position = 0;
		return ret;
	}
	// an update() method is not implemented for the non-interrupt version,
	// so that external programs can never call it by accident and performance
	// is not wasted checking the digital pins twice. Instead, the IRAM_ATTR
	// versions of the update methods are used in the read() and readAndReset()
	// methods.
#if !defined(ENCODER_USE_INTERRUPTS) && defined(__arm__) && defined(TEENSYDUINO)
	__attribute__((always_inline))
#endif
	void update(void) {
		uint8_t s = state & 3;
		if (digitalRead(pin1)) s |= 4;
		if (digitalRead(pin2)) s |= 8;
		switch (s) {
			case 0: case 5: case 10: case 15:
				break;
			case 1: case 7: case 8: case 14:
				position++; break;
			case 2: case 4: case 11: case 13:
				position--; break;
			case 3: case 12:
				position += 2; break;
			default:
				position -= 2; break;
		}
		state = (s >> 2);
	}
private:
#ifdef ENCODER_USE_INTERRUPTS
	volatile IO_REG_TYPE *pin1_register;
	volatile IO_REG_TYPE *pin2_register;
	IO_REG_TYPE pin1_bitmask;
	IO_REG_TYPE pin2_bitmask;
#endif
	uint8_t pin1;
	uint8_t pin2;
	uint8_t state;
	long position;

public:
#ifdef ENCODER_USE_INTERRUPTS
	static Encoder *interruptArgs[ENCODER_ARGLIST_SIZE];

	//                           _______         _______
	//               Pin1 ______|       |_______|       |______ Pin1
	// negative <---         _______         _______         __      --> positive
	//               Pin2 __|       |_______|       |_______|   Pin2

		//	new	new	old	old
		//	pin2	pin1	pin2	pin1	Result
		//	----	----	----	----	------
		//	0	0	0	0	no movement
		//	0	0	0	1	+1
		//	0	0	1	0	-1
		//	0	0	1	1	+2  (assume pin1 edges only)
		//	0	1	0	0	-1
		//	0	1	0	1	no movement
		//	0	1	1	0	-2  (assume pin1 edges only)
		//	0	1	1	1	+1
		//	1	0	0	0	+1
		//	1	0	0	1	-2  (assume pin1 edges only)
		//	1	0	1	0	no movement
		//	1	0	1	1	-1
		//	1	1	0	0	+2  (assume pin1 edges only)
		//	1	1	0	1	-1
		//	1	1	1	0	+1
		//	1	1	1	1	no movement
/*
	// Simple, easy-to-understand implementation.
	// http://www.dprg.org/tutorials/2013-01-12-how-to-use-a-rotary-encoder-in-a-mcu-project/
	//
	void update(void) {
		uint8_t p1val = digitalRead(pin1);
		uint8_t p2val = digitalRead(pin2);
		uint8_t state = encoder.state & 3;
		if (p1val) state |= 4;
		if (p2val) state |= 8;
		encoder.state = (state >> 2);
		switch (state) {
			case 1: case 7: case 8: case 14:
				encoder.position++;
				return;
			case 2: case 4: case 11: case 13:
				encoder.position--;
				return;
			case 3: case 12:
				encoder.position += 2;
				return;
			case 6: case 9:
				encoder.position -= 2;
				return;
		}
	}
*/

public:
	// A vald CW or CCW move returns 1, invalid returns 0.
	static const int8_t ENCODER_SEQUENCE_LEN = 4;
	static const int8_t ENCODER_STATE_TABLE
		[ENCODER_SEQUENCE_LEN*ENCODER_SEQUENCE_LEN];

	void update(void) {
	#ifdef ENCODER_OPTIMIZE_INTERRUPTS
		uint8_t p1val = DIRECT_PIN_READ(pin1_register, pin1_bitmask);
		uint8_t p2val = DIRECT_PIN_READ(pin2_register, pin2_bitmask);
	#else
		uint8_t p1val = digitalRead(encoder.pin1);
		uint8_t p2val = digitalRead(encoder.pin2);
	#endif
		uint8_t newState = (p1val << 1) | p2val;
		uint8_t oldState = encoder.state;
		encoder.state = newState;
		int8_t moved =
			ENCODER_STATE_TABLE[oldState*ENCODER_SEQUENCE_LEN + newState];
		if (moved) {
			encoder.position = encoder.position + moved;
		}
	}


private:
	struct Encoder_internal_state_t {
		uint8_t pin1;
		uint8_t pin2;
		#ifdef ENCODER_OPTIMIZE_INTERRUPTS
		volatile IO_REG_TYPE *pin1_register;
		volatile IO_REG_TYPE *pin2_register;
		IO_REG_TYPE pin1_bitmask;
		IO_REG_TYPE pin2_bitmask;
		#endif
		volatile long position;
		volatile uint8_t state;
	};
	static Encoder_internal_state_t encoder;

	bool interrupts_in_use;

	static void attach_interrupt(uint8_t pin, Encoder *instance) {
		uint8_t interrupt_num = digitalPinToInterrupt(pin);
		#if defined(__AVR_ATmega32U4__)
		// The update is different for the ATmega32U4. It is necessary to
		// check if the pins are on the same port and share the same
		// interrupt. With this check the library is about 2x slower, but
		// it is necessary for the ATmega32U4.
		if ((interrupt_num == 0 && instance->encoder.pin1 == 3 && instance->encoder.pin2 == 2) ||
			(interrupt_num == 0 && instance->encoder.pin1 == 2 && instance->encoder.pin2 == 3) ||
			(interrupt_num == 1 && instance->encoder.pin1 == 0 && instance->encoder.pin2 == 1) ||
			(interrupt_num == 1 && instance->encoder.pin1 == 1 && instance->encoder.pin2 == 0)) {
			// This is a shared interrupt, we need to check both pins.
			interruptArgs[interrupt_num] = instance;
			attachInterrupt(interrupt_num, update, CHANGE);
		} else {
		#endif
			// This is a single interrupt, we only need to check one pin.
			interruptArgs[interrupt_num] = instance;
			attachInterrupt(interrupt_num, update, CHANGE);
		#if defined(__AVR_ATmega32U4__)
		}
		#endif
	}

	static void update(void) {
		// all of the interrupts are routed to this function
		uint8_t interrupt_num = 0;
		// it is necessary to check which interrupt has been triggered
		#if defined(__AVR_ATmega32U4__)
		if (interrupt_num == 0) {
			// check if the interrupt is for the encoder
			if (interruptArgs[0] != NULL) {
				interruptArgs[0]->update();
			}
		} else if (interrupt_num == 1) {
			// check if the interrupt is for the encoder
			if (interruptArgs[1] != NULL) {
				interruptArgs[1]->update();
			}
		}
		#else
		// on other boards, we can just check the interrupt number
		interrupt_num = digitalPinToInterrupt(
			interruptArgs[interrupt_num]->encoder.pin1);
		if (interrupt_num != NOT_AN_INTERRUPT &&
			interruptArgs[interrupt_num] != NULL) {
			interruptArgs[interrupt_num]->update();
		}
		#endif
	}

#endif
};

#endif
