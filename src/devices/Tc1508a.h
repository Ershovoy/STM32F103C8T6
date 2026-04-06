#pragma once

#include <FS_MX1508.h>

#include "BaseDevice.h"

class Tc1508aDevice : public BaseDevice
{
public:
    Tc1508aDevice(uint32_t pinIn1, uint32_t pinIn2,
                  DecayMode decayMode = SLOW_DECAY)
        : _motor(pinIn1, pinIn2, decayMode)
    {
        setName("TC1508A");

        claimPin(pinIn1);
        claimPin(pinIn2);
        addHoldingRegister("Motor PWM, -255 to 255");
        addCoil("Motor brake");
    }

    void initialize() override
    {
        _motor.motorStop();
    }

    void update() override
    {
        uint16_t rawA = holdingRegisterRead(0);
        uint8_t brakeA = coilRead(0);

        if (brakeA)
        {
            _motor.motorBrake();
        }
        else
        {
            _motor.motorGo((int16_t)rawA);
        }
    }

private:
    MX1508 _motor;
};