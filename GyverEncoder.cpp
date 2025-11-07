#include <Arduino.h>
#include "GyverEncoder.h"

Encoder::Encoder(uint8_t clk, uint8_t dt, uint8_t sw, uint8_t type) {
	_pins[0] = clk;
	_pins[1] = dt;
	_pins[2] = sw;
	_type = type;
	pinMode(_pins[0], INPUT);
	pinMode(_pins[1], INPUT);
	pinMode(_pins[2], INPUT_PULLUP);
	_lastState = (digitalRead(_pins[0]) | (digitalRead(_pins[1]) << 1));
}
Encoder::Encoder(uint8_t clk, uint8_t dt, uint8_t type) {
	_pins[0] = clk;
	_pins[1] = dt;
	_type = type;
	pinMode(_pins[0], INPUT);
	pinMode(_pins[1], INPUT);
	_lastState = (digitalRead(_pins[0]) | (digitalRead(_pins[1]) << 1));
}

void Encoder::setType(uint8_t type) {
	_type = type;
}
void Encoder::setPin(uint8_t pin, uint8_t val) {
	_pins[pin] = val;
	if (pin == 2) pinMode(_pins[2], INPUT_PULLUP);
	else pinMode(_pins[pin], INPUT);
}
void Encoder::setPins(uint8_t clk, uint8_t dt) {
	_pins[0] = clk;
	_pins[1] = dt;
	pinMode(_pins[0], INPUT);
	pinMode(_pins[1], INPUT);
}
void Encoder::setPins(uint8_t clk, uint8_t dt, uint8_t sw) {
	_pins[0] = clk;
	_pins[1] = dt;
	_pins[2] = sw;
	pinMode(_pins[0], INPUT);
	pinMode(_pins[1], INPUT);
	pinMode(_pins[2], INPUT_PULLUP);
}

void Encoder::tick(bool hold) {
	tick();
	if (hold) {
		if (isHold()) {
			if (_isRight_f) _isRightH_f = true;
			if (_isLeft_f) _isLeftH_f = true;
			_isRight_f = false;
			_isLeft_f = false;
		} else {
			_isRightH_f = false;
			_isLeftH_f = false;
		}
	}
}

void Encoder::tick() {
	_turnFlag = false;
	_isRight_f = false;
	_isLeft_f = false;
	_isPress_f = false;
	_isRelease_f = false;
	_isClick_f = false;
	_isHolded_f = false;

	// опрос кнопки
	if (_pins[2] != 255) {
		uint32_t ms = millis();
		bool state = !digitalRead(_pins[2]);
		if (state && !_flag && (ms - _deb > 80)) {
			_deb = ms;
			_flag = true;
			_buttFlag = true;
			_isPress_f = true;
			_isHold_f = false;
			_hold_flag = false;
		}
		if (!_flag && _buttFlag && (ms - _deb > 1000) && !_hold_flag) {
			_isHolded_f = true;
			_hold_flag = true;
			_buttFlag = false;
		}
		if (_flag && !state) {
			_deb = ms;
			_flag = false;
			_isRelease_f = true;
			if (_buttFlag) _isClick_f = true;
			_buttFlag = false;
		}
		_isHold_f = _hold_flag;
	}

	// опрос энкодера
	_encState = (digitalRead(_pins[0]) | (digitalRead(_pins[1]) << 1));
	if (_encState != _lastState) {
		_turnFlag = true;
		if (_type == ENC_TYPE_STEP4_FULL) {
			if (_encState == 0b00) {
				if (_lastState == 0b01) {
					if (_direction) { _counter--; _isLeft_f = true; }
					else { _counter++; _isRight_f = true; }
				}
				if (_lastState == 0b10) {
					if (_direction) { _counter++; _isRight_f = true; }
					else { _counter--; _isLeft_f = true; }
				}
			}
		} else if (_type == ENC_TYPE_STEP2) {
			if (_encState == 0b00 && _lastState == 0b01 ||
			_encState == 0b11 && _lastState == 0b00 ||
			_encState == 0b10 && _lastState == 0b11 ||
			_encState == 0b01 && _lastState == 0b10) {
				if (_direction) { _counter--; _isLeft_f = true; }
				else { _counter++; _isRight_f = true; }
			}
			if (_encState == 0b00 && _lastState == 0b10 ||
			_encState == 0b11 && _lastState == 0b01 ||
			_encState == 0b10 && _lastState == 0b00 ||
			_encState == 0b01 && _lastState == 0b11) {
				if (_direction) { _counter++; _isRight_f = true; }
				else { _counter--; _isLeft_f = true; }
			}
		} else if (_type == ENC_TYPE_STEP1) {
			if (_encState == 0b00 && _lastState == 0b01 ||
			_encState == 0b01 && _lastState == 0b11 ||
			_encState == 0b11 && _lastState == 0b10 ||
			_encState == 0b10 && _lastState == 0b00) {
				if (_direction) { _counter--; _isLeft_f = true; }
				else { _counter++; _isRight_f = true; }
			}
			if (_encState == 0b00 && _lastState == 0b10 ||
			_encState == 0b10 && _lastState == 0b11 ||
			_encState == 0b11 && _lastState == 0b01 ||
			_encState == 0b01 && _lastState == 0b00) {
				if (_direction) { _counter++; _isRight_f = true; }
				else { _counter--; _isLeft_f = true; }
			}
		}
		_lastState = _encState;
	}
}

bool Encoder::isTurn() {
	return _turnFlag;
}
bool Encoder::isRight() {
	return _isRight_f;
}
bool Encoder::isLeft() {
	return _isLeft_f;
}
bool Encoder::isRightH() {
	return _isRightH_f;
}
bool Encoder::isLeftH() {
	return _isLeftH_f;
}

bool Encoder::isPress() {
	return _isPress_f;
}
bool Encoder::isRelease() {
	return _isRelease_f;
}
bool Encoder::isClick() {
	return _isClick_f;
}
bool Encoder::isHolded() {
	return _isHolded_f;
}
bool Encoder::isHold() {
	return _isHold_f;
}

void Encoder::setDirection(bool direction) {
	_direction = direction;
}

void Encoder::write(int counter) {
	_counter = counter;
}
int Encoder::read() {
	return _counter;
}