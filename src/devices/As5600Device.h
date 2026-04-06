#pragma once

#include "AMS_AS5600.h"

#include "BaseDevice.h"

class As5600Device : public BaseDevice
{
public:
    As5600Device(AS5600 &sensor) : _sensor(sensor)
    {
        setName("AS5600");
        addInputRegister("Raw angle 0-4095");
        addInputRegister("Degrees, 0-360");
        addDiscreteInput("Magnet detected");
        addDiscreteInput("Magnet too weak");
        addDiscreteInput("Magnet too strong");
    }

    void initialize() override
    {
        _sensor.begin();
        _sensor.setClock();
    }

    void update() override
    {
        bool detected = _sensor.isMagnetDetected();
        bool tooWeak = _sensor.isMagnetTooWeak();
        bool tooStrong = _sensor.isMagnetTooStrong();

        discreteInputWrite(0, detected ? 1 : 0);
        discreteInputWrite(1, tooWeak ? 1 : 0);
        discreteInputWrite(2, tooStrong ? 1 : 0);

        if (detected)
        {
            inputRegisterWrite(0, _sensor.getRawAngle());
            inputRegisterWrite(1, (uint16_t)(_sensor.getDegreesAngle()));
        }
    }

private:
    AS5600 &_sensor;
};
