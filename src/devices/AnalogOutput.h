#pragma once

#include "BaseDevice.h"

class AnalogOutput : public BaseDevice
{
public:
    AnalogOutput(uint32_t pin, uint16_t defaultFreqHz = 1000, uint16_t defaultResolution = 255)
        : _pin(pin), _freqHz(defaultFreqHz), _resolution(defaultResolution)
    {
        setName("AnalogOutput");
        claimPin(pin);
        addHoldingRegister("PWM value");
        addHoldingRegister("PWM frequency");
        addHoldingRegister("PWM resolution");
    }

    void initialize() override
    {
        pinMode(getPin(_pin), OUTPUT);

        holdingRegisterWrite(1, _freqHz);
        holdingRegisterWrite(2, _resolution);

        if (_freqHz != 0 && _resolution != 0)
        {
            analogWriteFrequency(_freqHz);
            analogWriteResolution(_resolution);
        }
        analogWrite(getPin(_pin), 0);
    }

    void update() override
    {
        // TODO: Избыточно, обычно задаются один раз при создании и не меняются во время выполнения.
        uint16_t newFreq = holdingRegisterRead(1);
        uint16_t newResolution = holdingRegisterRead(2);

        if (newFreq != _freqHz || newResolution != _resolution)
        {
            _freqHz = newFreq;
            _resolution = newResolution;
            if (_freqHz != 0 && _resolution != 0)
            {
                analogWriteFrequency(_freqHz);
                analogWriteResolution(_resolution);
            }
        }

        uint16_t value = holdingRegisterRead(0);
        value = constrain(value, 0, _resolution);
        analogWrite(getPin(_pin), value);
        // TODO: Не должны ли мы вернуть прошлое значене шима?
    }

private:
    uint32_t _pin;
    uint16_t _freqHz;
    uint16_t _resolution;
};