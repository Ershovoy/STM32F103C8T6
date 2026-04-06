#pragma once

#include <Servo.h>

#include "BaseDevice.h"

class MyServo : public BaseDevice
{
public:
    MyServo(uint32_t pin)
    {
        setName("MyServo");
        addHoldingRegister("Some description");
        claimPin(pin);

        _pin = getPin(pin);
    }
    void initialize() override
    {
        _servo.attach(getPin(_pin));
        _servo.write(90);
    }
    void update() override
    {
        int angle = holdingRegisterRead(0);

        if (angle)
        {
            angle = angle > 180 ? 180 : angle;
            _servo.write(angle);
        }
    }

private:
    Servo _servo;
    uint32_t _pin;
};