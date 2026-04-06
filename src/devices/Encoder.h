#pragma once

#include <EncButton.h>
#include "BaseDevice.h"

class EncoderDevice : public BaseDevice
{
public:
    EncoderDevice(uint32_t encoderPinS1, uint32_t encoderPinS2,
                  uint32_t keyPin = INVALID_PIN)
        : _encoder(encoderPinS1, encoderPinS2),
          _hasButton(keyPin != INVALID_PIN)
    {
        setName("Encoder");

        claimPin(encoderPinS1);
        claimPin(encoderPinS2);

        addInputRegister("Counter (int32 high word)");
        addInputRegister("Counter (int32 low word)");
        addHoldingRegister("Reset counter (pusle)");

        if (_hasButton)
        {
            claimPin(keyPin);
            _encoder.init(encoderPinS1, encoderPinS2, keyPin);

            addDiscreteInput("Encoder button click");
        }
    }
    
    void initialize() override {}
    
    void update() override
    {
        _encoder.tick();

        if (holdingRegisterRead(0))
        {
            _encoder.counter = 0;
            holdingRegisterWrite(0, 0);
        }

        int32_t pos = _encoder.counter;
        inputRegisterWrite(0, (uint16_t)(pos >> 16));
        inputRegisterWrite(1, (uint16_t)(pos & 0xFFFF));

        if (_hasButton)
        {
            discreteInputWrite(0, _encoder.pressing() ? 1 : 0);
        }
    }

private:
    EncButton _encoder;
    bool _hasButton;
};