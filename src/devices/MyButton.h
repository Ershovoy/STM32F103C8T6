#pragma once

#include "BaseDevice.h"

class MyButton : public BaseDevice
{
public:
    MyButton(int pin)
    {
        setName("MyButton");
        addCoil();
        claimPin(pin);
        _pin = pin;
    }
    void initialize() override
    {
        pinMode(getPin(_pin), INPUT_PULLUP);
    }
    void update() override
    {
        bool buttonPressed = (digitalRead(getPin(_pin)) == LOW);
        coilWrite(1, buttonPressed ? 1 : 0);
    }

private:
    int _pin;
};