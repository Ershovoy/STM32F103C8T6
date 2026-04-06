#pragma once

#include "BaseDevice.h"

class AnalogInput : public BaseDevice
{
public:
    AnalogInput(uint32_t pin)
        : _pin(pin)
    {
        setName("AnalogInput");
        claimPin(pin);
        addInputRegister("ADC value");
    }

    void initialize() override
    {
        // TODO: параметр атиниешион attinuation в конфиг
        pinMode(getPin(_pin), INPUT_ANALOG);
    }

    void update() override
    {
        inputRegisterWrite(0, (uint16_t)analogRead(getPin(_pin)));
        // TODO:
        // analogReadResolution();
    }

private:
    uint32_t _pin;
};