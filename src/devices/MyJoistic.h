#pragma once

#include "BaseDevice.h"

class MyJoistic : public BaseDevice
{
public:
    MyJoistic(uint32_t vrxPin, uint32_t vryPin, uint32_t swPin)
    {
        claimPin(vrxPin);
        claimPin(vryPin);
        claimPin(swPin);
        addInputRegister("vrx");
        addInputRegister("vry");
        addInputRegister("sw");

        _vrxPin = vrxPin;
        _vryPin = vryPin;
        _swPin = swPin;
    }

    void initialize() override
    {
    }
    void update() override
    {
        inputRegisterWrite(0, analogRead(getPin(_vrxPin)));
        inputRegisterWrite(1, analogRead(getPin(_vryPin)));
        inputRegisterWrite(2, analogRead(getPin(_swPin)));
    }

private:
    uint32_t _vrxPin;
    uint32_t _vryPin;
    uint32_t _swPin;
};
