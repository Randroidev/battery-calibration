/*
    GyverEncoder - библиотека для энкодера
    - Быстрая работа на прерываниях
    - Работа в основном цикле
    - Подключение - любое
    - Встроенный обработчик кнопки
    - Свой обработчик кнопки
    - Фильтрация помех
    - Настраиваемое направление
    - Типы энкодеров:
        - ENC_TYPE_STEP4_FULL - полный шаг, 4 такта на щелчок
        - ENC_TYPE_STEP2 - полушаг, 2 такта на щелчок
        - ENC_TYPE_STEP1 - четверть шага, 1 такт на щелчок

    Документация: https://alexgyver.ru/gyverencoder/
    Создано AlexGyver, 2019
    Версия 1.7
*/

#ifndef GyverEncoder_h
#define GyverEncoder_h
#include <Arduino.h>

#define _CLK 0
#define _DT 1
#define _SW 2

// ******** НАСТРОЙКИ ********
#define ENC_TYPE_STEP4_FULL 0   // полный шаг, 4 такта на щелчок
#define ENC_TYPE_STEP2 1        // полушаг, 2 такта на щелчок
#define ENC_TYPE_STEP1 2        // четверть шага, 1 такт на щелчок

class Encoder {
public:
    // можно передать пины (CLK, DT, SW) и тип энкодера
    Encoder(uint8_t clk, uint8_t dt, uint8_t sw, uint8_t type = ENC_TYPE_STEP4_FULL);

    // можно передать пины (CLK, DT) и тип энкодера
    Encoder(uint8_t clk, uint8_t dt, uint8_t type = ENC_TYPE_STEP4_FULL);

    void setType(uint8_t type);				// установка типа энкодера (по умолч. ENC_TYPE_STEP4_FULL)
    void setPin(uint8_t pin, uint8_t val);	// смена пина CLK/DT/SW
    void setPins(uint8_t clk, uint8_t dt);	// смена пинов CLK/DT
    void setPins(uint8_t clk, uint8_t dt, uint8_t sw);	// смена пинов CLK/DT/SW

    void tick();							// опрос энкодера, нужно вызывать постоянно или в прерывании
    void tick(bool hold);					// опрос энкодера с удержанием (кнопка)

    bool isTurn(); 			// возвращает true, если был совершён поворот в любую сторону
    bool isRight();			// возвращает true, если был поворот направо
    bool isLeft();			// возвращает true, если был поворот налево
    bool isRightH();		// возвращает true, если было удержание + поворот направо
    bool isLeftH();			// возвращает true, если было удержание + поворот налево

    bool isPress();			// возвращает true, если была нажата кнопка
    bool isRelease();		// возвращает true, если была отпущена кнопка
    bool isClick();			// возвращает true, если был клик
    bool isHolded();		// возвращает true, если была удержана кнопка
    bool isHold();			// возвращает true, если кнопка удерживается

    void setDirection(bool direction);		// смена направления вращения
    void write(int counter);				// записать новое значение счётчика
    int read();								// прочитать счётчик

private:
    uint8_t _pins[3];
    uint8_t _type = ENC_TYPE_STEP4_FULL;
    bool _direction = false;
    volatile int _counter = 0;
    volatile bool _turnFlag = false;
    volatile bool _isRight_f = false;
    volatile bool _isLeft_f = false;
    volatile bool _isRightH_f = false;
    volatile bool _isLeftH_f = false;

    volatile bool _isPress_f = false;
    volatile bool _isRelease_f = false;
    volatile bool _isClick_f = false;
    volatile bool _isHolded_f = false;
    volatile bool _isHold_f = false;

    uint32_t _deb;
    bool _flag = false;
    bool _buttFlag = false;
    bool _hold_flag = false;

    volatile byte _encState = 0;
    volatile byte _lastState = 0;
};
#endif
