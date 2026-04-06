#pragma once

// BaseDevice
//
// Абстрактный базовый класс для всех устройств в системе.
//
// Принцип работы:
//   1. Наследник в конструкторе вызывает claimPin() и addXxxRegister() —
//      пины и регистры резервируются в статических (общих для всех) таблицах.
//   2. В setup() вызывается initialize() — однократная инициализация железа.
//   3. В loop() вызывается update() — чтение сенсоров / управление актуаторами.
//
// Все таблицы статические: данные хранятся не в каждом объекте, а в одном
// общем месте. Порядок создания объектов определяет адреса регистров.

#include <Arduino.h>
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>
#include <stdint.h>
#include <setjmp.h>

#define MAX_CLAIMED_PINS 64
#define MAX_REGISTERS 256
#define MAX_ERRORS 64
#define MAX_DEVICES 64

#define INVALID_PIN 0xFFFFFFFFu
#define INVALID_MODE 0xFFFFFFFFu

#define INVALID_REGISTER_INDEX 0xFFu

enum class RegisterType : uint8_t
{
    COIL,
    DISCRETE_INPUT,
    INPUT_REGISTER,
    HOLDING_REGISTER
};

class BaseDevice
{
public:
    BaseDevice();

    void setName(const char *name);
    const char *getDeviceName() const { return _deviceNames[_id]; }
    uint8_t getDeviceId() const { return _id; }
    bool isValid() const { return _valid; }

    uint8_t coilRead(uint8_t localIndex);
    void coilWrite(uint8_t localIndex, uint8_t value);
    void discreteInputWrite(uint8_t localIndex, uint8_t value);
    void inputRegisterWrite(uint8_t localIndex, uint16_t value);
    uint16_t holdingRegisterRead(uint8_t localIndex);
    void holdingRegisterWrite(uint8_t localIndex, uint16_t value);

    static uint8_t getCoilCount();
    static uint8_t getDiscreteInputCount();
    static uint8_t getInputRegisterCount();
    static uint8_t getHoldingRegisterCount();

    // Если внутри update() возникает критическая ошибка, вызывается longjmp и выполнение текущего update() прерывается.
    static jmp_buf updateJumpBuffer;
    static bool inUpdateLoop;

    static void printRegisterMap(HardwareSerial &serial);
    static void printErrors(HardwareSerial &serial);

    virtual void initialize() = 0;
    virtual void update() = 0;

protected:
    void addCoil(const char *description = "Default register description");
    void addDiscreteInput(const char *description = "Default register description");
    void addInputRegister(const char *description = "Default register description");
    void addHoldingRegister(const char *description = "Default register description");

    // Резервирует пин за данным устройством. Повторный claim приводит к ошибке.
    // Возвращает rawPin, если он принадлежит этому устройству.
    void claimPin(uint32_t rawPin);
    uint32_t getPin(uint32_t rawPin);

private:
    void _addRegister(const char *description, RegisterType type);
    uint8_t _getRegisterIndex(RegisterType type, uint8_t localIndex);
    void _writeRegister(uint8_t index, RegisterType type, uint16_t value);
    uint16_t _readRegister(uint8_t index, RegisterType type);

    static uint8_t _registerAddresses[MAX_REGISTERS];
    static RegisterType _registerTypes[MAX_REGISTERS];
    static uint8_t _registerOwners[MAX_REGISTERS];
    static const char *_registerDescriptions[MAX_REGISTERS];
    static uint8_t _registerCount;

    static uint32_t _claimedPins[MAX_CLAIMED_PINS];
    static uint8_t _claimedOwners[MAX_CLAIMED_PINS];
    static uint8_t _claimedCount;

    static uint8_t _nextCoil;
    static uint8_t _nextDiscreteInput;
    static uint8_t _nextInputRegister;
    static uint8_t _nextHoldingRegister;

    static const char *_deviceNames[MAX_DEVICES];
    static uint8_t _nextDeviceId;

    void _addError(const char *message, uint32_t pin = INVALID_PIN, uint8_t index = INVALID_REGISTER_INDEX);

    static const char *_errors[MAX_ERRORS];
    static const char *_errorDeviceNames[MAX_ERRORS];
    static uint32_t _errorPins[MAX_ERRORS];
    static uint32_t _errorIndices[MAX_ERRORS];
    static uint8_t _errorCount;

    uint8_t _id;
    bool _valid = true;
};