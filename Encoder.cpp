/*
  Encoder.cpp - Library for reading quadrature encoders
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


#include "Encoder.h"

#if defined(ENCODER_USE_INTERRUPTS) && !defined(ENCODER_DO_NOT_USE_INTERRUPTS)

// The ENCODER_USE_INTERRUPTS flag is to make it easier to discover
// bugs by forcing all IO to go through the digitalRead() function.
//
// This is not the most optimal code for hand-built ISRs, but we're
// trying to be compatible with a wide range of Arduino boards without
// requiring the user to write different code for different boards.
// It is possible to more than double the performance of this library
// by creating custom ISRs for specific boards and chips, but that
// would require the user to configure this library for their hardware.

Encoder * Encoder::interruptArgs[ENCODER_ARGLIST_SIZE];

const int8_t Encoder::ENCODER_STATE_TABLE
	[ENCODER_SEQUENCE_LEN*ENCODER_SEQUENCE_LEN] =
{
	// -1 = invalid state, 0 = no move, 1 = positive, 2 = negative
	// The table is stored in the following format:
	// oldState*ENCODER_SEQUENCE_LEN + newState
	 0,  2,  1, -1, // 0000, 0001, 0010, 0011
	 1,  0, -1,  2, // 0100, 0101, 0110, 0111
	 2, -1,  0,  1, // 1000, 1001, 1010, 1011
	-1,  1,  2,  0  // 1100, 1101, 1110, 1111
};

Encoder::Encoder_internal_state_t Encoder::encoder;

#endif
