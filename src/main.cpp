#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "AMS_AS5600.h"

#include "BaseDevice.h"

#include "devices/AnalogInput.h"
#include "devices/AnalogOutput.h"
#include "devices/DigitalInput.h"
#include "devices/DigitalOutput.h"

#include "devices/MyLed.h"

#include "devices/Tc1508a.h"
#include "devices/As5600Device.h"
#include "devices/Pca9685.h"
#include "devices/StepperMotorDevice.h"
#include "devices/Encoder.h"

// Порядок создания объектов определяет карту регистров, изменение порядка меняет адреса Modbus.
BaseDevice *devices[MAX_DEVICES] = {
    // new MyLed(PB15),
    new MyLed(PC13),
    // new MyButton(PA5),
    // new As5600Device(*new AS5600(&Wire)),
    // new Tc1508aDevice(PB6, PB7, INVALID_PIN, INVALID_PIN),
    // new Pca9685(0, 0, 0, 0),
    // new StepperMotorDevice(PA6, PA5, PA4),
    new EncoderDevice(PB3, PB4, PB5),
};
uint8_t deviceCount = 0;

#define SLAVE_ID 1
#define BAUD_RATE 9600

HardwareSerial Serial3(PA3, PA2);

void setup()
{
    Serial3.begin(BAUD_RATE);
    delay(5000);

    deviceCount = 0;
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (devices[i])
        {
            deviceCount++;
        }
    }

    RS485.setPins(PA9, PA1, PA1);
    RS485.begin(BAUD_RATE);

    ModbusRTUServer.begin(SLAVE_ID, BAUD_RATE);

    uint8_t coilCount = BaseDevice::getCoilCount();
    uint8_t discreteInputCount = BaseDevice::getDiscreteInputCount();
    uint8_t inputRegisterCount = BaseDevice::getInputRegisterCount();
    uint8_t holdingRegisterCount = BaseDevice::getHoldingRegisterCount();

    if (coilCount)
    {
        ModbusRTUServer.configureCoils(0, coilCount);
    }
    if (discreteInputCount)
    {
        ModbusRTUServer.configureDiscreteInputs(0, discreteInputCount);
    }
    if (inputRegisterCount)
    {
        ModbusRTUServer.configureInputRegisters(0, inputRegisterCount);
    }
    if (holdingRegisterCount)
    {
        ModbusRTUServer.configureHoldingRegisters(0, holdingRegisterCount);
    }

    for (int i = 0; i < deviceCount; i++)
    {
        if (devices[i])
        {
            if (devices[i]->isValid())
            {
                devices[i]->initialize();
            }
        }
    }

    BaseDevice::printErrors(Serial3);
    BaseDevice::printRegisterMap(Serial3);
}

void loop()
{
    ModbusRTUServer.poll();
    for (int i = 0; i < deviceCount; i++)
    {
        if (devices[i])
        {
            if (devices[i]->isValid())
            {
                // setjmp/longjmp — защита от краша в update():
                // если устройство обратится к несуществующему регистру или пину,
                // _addError() вызовет longjmp и мы окажемся здесь (setjmp вернёт != 0),
                // пропустив оставшуюся часть update() данного устройства.
                BaseDevice::inUpdateLoop = true;
                if (setjmp(BaseDevice::updateJumpBuffer) == 0)
                {
                    devices[i]->update();
                }

                BaseDevice::inUpdateLoop = false;
            }
        }
    }

    // Выводим только новые ошибки (не спамим при каждом вызове)
    BaseDevice::printErrors(Serial3);
}
