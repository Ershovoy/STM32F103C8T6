#pragma once

#include "BaseDevice.h"

class MyLed : public BaseDevice
{
public:
    MyLed(int pin)
    {
        setName("MyLed");
        claimPin(pin);
        addHoldingRegister();

        _pin = pin;
    }
    void initialize() override
    {
        pinMode(getPin(_pin), OUTPUT);
    }
    void update() override
    {
        if (holdingRegisterRead(0) > 0)
        {
            digitalWrite(getPin(_pin), LOW);
        }
        else
        {
            digitalWrite(getPin(_pin), HIGH);
        }
    }

private:
    uint32_t _pin;
};
