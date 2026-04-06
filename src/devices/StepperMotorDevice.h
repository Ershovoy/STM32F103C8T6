#pragma once

#include <GyverStepper2.h>
#include "BaseDevice.h"

constexpr uint16_t BIT_ENABLE = 0b0000000000000001;
constexpr uint16_t BIT_GO = 0b0000000000000010;
constexpr uint16_t BIT_STOP = 0b0000000000000100;
constexpr uint16_t BIT_BRAKE = 0b0000000000001000;
constexpr uint16_t BIT_RESET = 0b0000000000010000;
constexpr uint16_t BIT_STATUS = 0b0000000000010000;

class StepperMotorDevice : public BaseDevice
{
public:
    StepperMotorDevice(uint8_t pinEn, uint8_t pinDir, uint8_t pinStep)
        : _stepper(200, pinStep, pinDir, pinEn)
    {
        setName("StepperMotor");
        claimPin(pinEn);
        claimPin(pinDir);
        claimPin(pinStep);

        addHoldingRegister("Target HIGH (int32)");
        addHoldingRegister("Target LOW  (int32)");
        addHoldingRegister("MaxSpeed steps/sec");
        addHoldingRegister("Acceleration steps/sec2");
        addHoldingRegister("Flags b0=en b1=go b2=stop b3=brake(pulse) b4=reset(pulse)");

        addInputRegister("Current pos HIGH");
        addInputRegister("Current pos LOW");
        addInputRegister("Flags b0=status(0-стоим, 1-едем, 2-едем к точке паузы, 3-крутимся со скоростью, 4-тормозим)");
    }

    void initialize() override
    {
        _stepper.setMaxSpeed(300);
        _stepper.setAcceleration(300);
        _stepper.enable();
    }

    void update() override
    {
        uint16_t flags = holdingRegisterRead(4);
        uint16_t newFlags = flags;

        if (flags & BIT_ENABLE)
        {
            _stepper.enable();
        }
        else
        {
            _stepper.disable();
        }

        uint16_t speed = holdingRegisterRead(2);
        if (speed)
        {
            _stepper.setMaxSpeed((float)speed);
        }

        uint16_t acceleration = holdingRegisterRead(3);
        if (acceleration)
        {
            _stepper.setAcceleration(acceleration);
        }

        if (flags & BIT_GO)
        {
            newFlags = newFlags & (~BIT_GO);
            int32_t target = ((int32_t)holdingRegisterRead(0) << 16) | holdingRegisterRead(1);
            _stepper.setTarget(target);
        }

        if (flags & BIT_STOP)
        {
            _stepper.stop();
        }

        if (flags & BIT_BRAKE)
        {
            newFlags = newFlags & (~BIT_BRAKE);
            _stepper.brake();
        }

        if (flags & BIT_RESET)
        {
            newFlags = newFlags & (~BIT_RESET);
            _stepper.reset();
        }
        
        if (newFlags != flags)
        {
            holdingRegisterWrite(4, newFlags);
        }
        
        _stepper.tick();
        
        int32_t currentPosition = _stepper.getCurrent();
        
        if (currentPosition != _lastPosition)
        {
            inputRegisterWrite(0, (uint16_t)((uint32_t)currentPosition >> 16));
            inputRegisterWrite(1, (uint16_t)(currentPosition & 0xFFFF));
            _lastPosition = currentPosition;
        }

        inputRegisterWrite(2, _stepper.getStatus());
    }

private:
    GStepper2<STEPPER2WIRE> _stepper;
    int32_t _lastPosition = 0;
};