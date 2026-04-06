#pragma once

#include "BaseDevice.h"

class DigitalInput : public BaseDevice
{
public:
    DigitalInput(uint32_t pin1  = INVALID_PIN, uint32_t mode1  = INVALID_MODE, uint32_t pin2  = INVALID_PIN, uint32_t mode2  = INVALID_MODE,
                 uint32_t pin3  = INVALID_PIN, uint32_t mode3  = INVALID_MODE, uint32_t pin4  = INVALID_PIN, uint32_t mode4  = INVALID_MODE,
                 uint32_t pin5  = INVALID_PIN, uint32_t mode5  = INVALID_MODE, uint32_t pin6  = INVALID_PIN, uint32_t mode6  = INVALID_MODE,
                 uint32_t pin7  = INVALID_PIN, uint32_t mode7  = INVALID_MODE, uint32_t pin8  = INVALID_PIN, uint32_t mode8  = INVALID_MODE,
                 uint32_t pin9  = INVALID_PIN, uint32_t mode9  = INVALID_MODE, uint32_t pin10 = INVALID_PIN, uint32_t mode10 = INVALID_MODE,
                 uint32_t pin11 = INVALID_PIN, uint32_t mode11 = INVALID_MODE, uint32_t pin12 = INVALID_PIN, uint32_t mode12 = INVALID_MODE,
                 uint32_t pin13 = INVALID_PIN, uint32_t mode13 = INVALID_MODE, uint32_t pin14 = INVALID_PIN, uint32_t mode14 = INVALID_MODE,
                 uint32_t pin15 = INVALID_PIN, uint32_t mode15 = INVALID_MODE, uint32_t pin16 = INVALID_PIN, uint32_t mode16 = INVALID_MODE)
    {
        setName("DigitalInput");
        addInputRegister("Input register bitmask 0-15");

        _pins[0]  = pin1;  _pins[1]  = pin2;  _pins[2]  = pin3;  _pins[3]  = pin4;
        _pins[4]  = pin5;  _pins[5]  = pin6;  _pins[6]  = pin7;  _pins[7]  = pin8;
        _pins[8]  = pin9;  _pins[9]  = pin10; _pins[10] = pin11; _pins[11] = pin12;
        _pins[12] = pin13; _pins[13] = pin14; _pins[14] = pin15; _pins[15] = pin16;

        _mods[0]  = mode1;  _mods[1]  = mode2;  _mods[2]  = mode3;  _mods[3]  = mode4;
        _mods[4]  = mode5;  _mods[5]  = mode6;  _mods[6]  = mode7;  _mods[7]  = mode8;
        _mods[8]  = mode9;  _mods[9]  = mode10; _mods[10] = mode11; _mods[11] = mode12;
        _mods[12] = mode13; _mods[13] = mode14; _mods[14] = mode15; _mods[15] = mode16;

        for (uint8_t i = 0; i < 16; i++)
        {
            if (_pins[i] != INVALID_PIN && _mods[i] != INVALID_MODE)
            {
                claimPin(_pins[i]);
            }
        }
    }
    void initialize() override
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            if (_pins[i] != INVALID_PIN && _mods[i] != INVALID_MODE)
            {
                pinMode(getPin(_pins[i]), _mods[i]);
            }
        }
    }
    void update() override
    {
        uint16_t resultMask = 0;
        for (uint8_t i = 0; i < 16; i++)
        {
            if (_pins[i] != INVALID_PIN && _mods[i] != INVALID_MODE)
            {
                if (digitalRead(getPin(_pins[i])))
                {
                    resultMask |= (1 << i);
                }
            }
        }

        inputRegisterWrite(0, resultMask);
    }

private:
    uint32_t _pins[16];
    uint32_t _mods[16];
};