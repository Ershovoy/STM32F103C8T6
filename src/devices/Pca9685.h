#pragma once

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "BaseDevice.h"

#define PCA9685_CHANNEL_UNUSED 0xFFFF

class Pca9685Device : public BaseDevice
{
public:
    Pca9685Device(
        TwoWire  &wire,
        float     defaultFrequencyHz = 50.0f,
        uint32_t  defaultOscillatorcHz  = 25000000UL,
        uint8_t   i2cAddress    = PCA9685_I2C_ADDRESS,
        uint16_t min0  = PCA9685_CHANNEL_UNUSED, uint16_t max0  = PCA9685_CHANNEL_UNUSED,
        uint16_t min1  = PCA9685_CHANNEL_UNUSED, uint16_t max1  = PCA9685_CHANNEL_UNUSED,
        uint16_t min2  = PCA9685_CHANNEL_UNUSED, uint16_t max2  = PCA9685_CHANNEL_UNUSED,
        uint16_t min3  = PCA9685_CHANNEL_UNUSED, uint16_t max3  = PCA9685_CHANNEL_UNUSED,
        uint16_t min4  = PCA9685_CHANNEL_UNUSED, uint16_t max4  = PCA9685_CHANNEL_UNUSED,
        uint16_t min5  = PCA9685_CHANNEL_UNUSED, uint16_t max5  = PCA9685_CHANNEL_UNUSED,
        uint16_t min6  = PCA9685_CHANNEL_UNUSED, uint16_t max6  = PCA9685_CHANNEL_UNUSED,
        uint16_t min7  = PCA9685_CHANNEL_UNUSED, uint16_t max7  = PCA9685_CHANNEL_UNUSED,
        uint16_t min8  = PCA9685_CHANNEL_UNUSED, uint16_t max8  = PCA9685_CHANNEL_UNUSED,
        uint16_t min9  = PCA9685_CHANNEL_UNUSED, uint16_t max9  = PCA9685_CHANNEL_UNUSED,
        uint16_t min10 = PCA9685_CHANNEL_UNUSED, uint16_t max10 = PCA9685_CHANNEL_UNUSED,
        uint16_t min11 = PCA9685_CHANNEL_UNUSED, uint16_t max11 = PCA9685_CHANNEL_UNUSED,
        uint16_t min12 = PCA9685_CHANNEL_UNUSED, uint16_t max12 = PCA9685_CHANNEL_UNUSED,
        uint16_t min13 = PCA9685_CHANNEL_UNUSED, uint16_t max13 = PCA9685_CHANNEL_UNUSED,
        uint16_t min14 = PCA9685_CHANNEL_UNUSED, uint16_t max14 = PCA9685_CHANNEL_UNUSED,
        uint16_t min15 = PCA9685_CHANNEL_UNUSED, uint16_t max15 = PCA9685_CHANNEL_UNUSED)
        : _pwm(i2cAddress, wire),
          _frequencyHz(defaultFrequencyHz),
          _oscillatorHz(defaultOscillatorcHz)
    {
        setName("PCA9685");

        _min[0]  = min0;  _max[0]  = max0;
        _min[1]  = min1;  _max[1]  = max1;
        _min[2]  = min2;  _max[2]  = max2;
        _min[3]  = min3;  _max[3]  = max3;
        _min[4]  = min4;  _max[4]  = max4;
        _min[5]  = min5;  _max[5]  = max5;
        _min[6]  = min6;  _max[6]  = max6;
        _min[7]  = min7;  _max[7]  = max7;
        _min[8]  = min8;  _max[8]  = max8;
        _min[9]  = min9;  _max[9]  = max9;
        _min[10] = min10; _max[10] = max10;
        _min[11] = min11; _max[11] = max11;
        _min[12] = min12; _max[12] = max12;
        _min[13] = min13; _max[13] = max13;
        _min[14] = min14; _max[14] = max14;
        _min[15] = min15; _max[15] = max15;

        for (uint8_t i = 0; i < 16; i++)
        {
            if (_isActive(i))
            {
                addHoldingRegister("Channel: [15]=enable [11:0]=value 0-4095");
            }
        }

        addHoldingRegister("PWM frequency Hz");
        addHoldingRegister("Oscillator freq high word");
        addHoldingRegister("Oscillator freq low word");
    }

    void initialize() override
    {
        uint8_t frequencyRegister = _activeCount();
        holdingRegisterWrite(frequencyRegister, (uint16_t)_frequencyHz);
        holdingRegisterWrite(frequencyRegister + 1, (uint16_t)(_oscillatorHz >> 16));
        holdingRegisterWrite(frequencyRegister + 2, (uint16_t)(_oscillatorHz & 0xFFFF));

        _pwm.begin();
        _pwm.setOscillatorFrequency(_oscillatorHz);
        _pwm.setPWMFreq(_frequencyHz);

        for (uint8_t i = 0; i < 16; i++)
        {
            _pwm.setPWM(i, 0, 4096);
        }
    }

    void update() override
    {
        uint8_t frequencyRegister = _activeCount();

        // TODO: Возможно избыточно и они задаются единожды при создании.
        float newFrequency = (float)holdingRegisterRead(frequencyRegister);
        uint32_t newOscillator = ((uint32_t)holdingRegisterRead(frequencyRegister + 1) << 16) | (uint32_t)holdingRegisterRead(frequencyRegister + 2);

        if (newFrequency != _frequencyHz || newOscillator != _oscillatorHz)
        {
            _frequencyHz = newFrequency;
            _oscillatorHz = newOscillator;
            _pwm.setOscillatorFrequency(_oscillatorHz);
            _pwm.setPWMFreq(_frequencyHz);
        }

        uint8_t registerIndex = 0;
        for (uint8_t channel = 0; channel < 16; channel++)
        {
            if (!_isActive(channel))
            {
                continue;
            }

            uint16_t reg = holdingRegisterRead(registerIndex++);
            bool enable = (reg >> 15) & 1;
            uint16_t value = reg & 0x0FFF;

            if (!enable)
            {
                _pwm.setPWM(channel, 0, 4096);
            }
            else
            {
                uint16_t pwm = (uint16_t)map(value, 0, 4095, _min[channel], _max[channel]);
                _pwm.setPWM(channel, 0, pwm);
            }
        }
    }

private:
    Adafruit_PWMServoDriver _pwm;
    float _frequencyHz;
    uint32_t _oscillatorHz;
    uint16_t _min[16];
    uint16_t _max[16];

    bool _isActive(uint8_t channel) const
    {
        return _min[channel] != PCA9685_CHANNEL_UNUSED && _max[channel] != PCA9685_CHANNEL_UNUSED;
    }

    uint8_t _activeCount() const
    {
        uint8_t count = 0;
        for (uint8_t i = 0; i < 16; i++)
        {
            if (_isActive(i))
            {
                count++;
            }
        }
        return count;
    }
};
