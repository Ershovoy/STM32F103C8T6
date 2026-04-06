#include <OneWire.h>
#include <DallasTemperature.h>

#include "BaseDevice.h"

#define MAX_DS18B20_SENSORS 8

class Ds18b20Device : public BaseDevice
{
public:
    Ds18b20Device(uint32_t pin,
                  uint8_t resolution = 12,
                  uint32_t updatePeriodMs = 1000)
        : _pin(pin),
          _resolution(resolution),
          _updatePeriodMs(updatePeriodMs),
          _oneWire(pin),
          _sensors(&_oneWire)
    {
        setName("DS18B20");
        claimPin(pin);

        for (uint8_t i = 0; i < MAX_DS18B20_SENSORS; i++)
        {
            addInputRegister("Temperature x10 degC (int16)");
        }

        addDiscreteInput("Sensors found count");
        addDiscreteInput("Last conversion OK");
    }

    void initialize() override
    {
        _sensors.begin();
        _sensors.setResolution(_resolution);
        _sensors.setWaitForConversion(false);

        _sensorCount = min(_sensors.getDeviceCount(), (uint8_t)MAX_DS18B20_SENSORS);
        discreteInputWrite(0, _sensorCount);
        discreteInputWrite(1, 1);

        _sensors.requestTemperatures();
        _lastRequestMs = millis();
    }

    void update() override
    {
        uint32_t now = millis();

        if (now - _lastRequestMs >= _sensors.millisToWaitForConversion())
        {
            bool ok = true;
            for (uint8_t i = 0; i < _sensorCount; i++)
            {
                float temp = _sensors.getTempCByIndex(i);
                if (temp == DEVICE_DISCONNECTED_C)
                {
                    ok = false;
                    continue;
                }
                inputRegisterWrite(i, (uint16_t)(int16_t)(temp * 10.0f));
            }
            discreteInputWrite(1, ok ? 1 : 0);

            if (now - _lastRequestMs >= _updatePeriodMs)
            {
                _sensors.requestTemperatures();
                _lastRequestMs = now;
            }
        }
    }

private:
    uint32_t _pin;
    uint8_t _resolution;
    uint32_t _updatePeriodMs;
    uint8_t _sensorCount = 0;
    uint32_t _lastRequestMs = 0;
    OneWire _oneWire;
    DallasTemperature _sensors;
};