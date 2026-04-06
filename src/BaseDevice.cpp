#include "BaseDevice.h"

//
// Statics
//

uint8_t BaseDevice::_registerAddresses[MAX_REGISTERS];
RegisterType BaseDevice::_registerTypes[MAX_REGISTERS];
uint8_t BaseDevice::_registerOwners[MAX_REGISTERS];
const char *BaseDevice::_registerDescriptions[MAX_REGISTERS];
uint8_t BaseDevice::_registerCount = 0;

uint32_t BaseDevice::_claimedPins[MAX_CLAIMED_PINS];
uint8_t BaseDevice::_claimedOwners[MAX_CLAIMED_PINS];
uint8_t BaseDevice::_claimedCount = 0;

uint8_t BaseDevice::_nextCoil = 0;
uint8_t BaseDevice::_nextDiscreteInput = 0;
uint8_t BaseDevice::_nextInputRegister = 0;
uint8_t BaseDevice::_nextHoldingRegister = 0;

uint8_t BaseDevice::_nextDeviceId = 0;
const char *BaseDevice::_deviceNames[MAX_DEVICES];

jmp_buf BaseDevice::updateJumpBuffer;
bool BaseDevice::inUpdateLoop = false;

const char *BaseDevice::_errors[MAX_ERRORS];
const char *BaseDevice::_errorDeviceNames[MAX_ERRORS];
uint32_t BaseDevice::_errorPins[MAX_ERRORS];
uint32_t BaseDevice::_errorIndices[MAX_ERRORS];
uint8_t BaseDevice::_errorCount = 0;


//
// Public methods
//

BaseDevice::BaseDevice()
{
    _id = _nextDeviceId++;
    setName("DefaultDeviceName");
}

void BaseDevice::setName(const char *name)
{
    _deviceNames[_id] = name;
}

void BaseDevice::claimPin(uint32_t rawPin)
{
    if (rawPin == INVALID_PIN)
    {
        return;
    }

    if (_claimedCount >= MAX_CLAIMED_PINS)
    {
        _addError("Pin registry full", INVALID_PIN, INVALID_REGISTER_INDEX);

        return;
    }

    for (uint8_t i = 0; i < _claimedCount; i++)
    {
        if (_claimedPins[i] == rawPin)
        {
            _addError("Pin already claimed", rawPin, INVALID_REGISTER_INDEX);

            return;
        }
    }

    _claimedPins[_claimedCount] = rawPin;
    _claimedOwners[_claimedCount] = _id;
    _claimedCount++;
}

uint32_t BaseDevice::getPin(uint32_t rawPin)
{
    if (rawPin != INVALID_PIN)
    {
        for (uint8_t i = 0; i < _claimedCount; i++)
        {
            if (_claimedPins[i] == rawPin)
            {
                if (_claimedOwners[i] == _id)
                {
                    return rawPin;
                }

                break;
            }
        }
        _addError("Pin is not claimed", rawPin, INVALID_REGISTER_INDEX);
    }

    return INVALID_PIN;
}

void BaseDevice::addCoil(const char *description)
{
    _addRegister(description, RegisterType::COIL);
}

void BaseDevice::addDiscreteInput(const char *description)
{
    _addRegister(description, RegisterType::DISCRETE_INPUT);
}

void BaseDevice::addInputRegister(const char *description)
{
    _addRegister(description, RegisterType::INPUT_REGISTER);
}

void BaseDevice::addHoldingRegister(const char *description)
{
    _addRegister(description, RegisterType::HOLDING_REGISTER);
}

uint8_t BaseDevice::coilRead(uint8_t localIndex)
{
    uint8_t index = _getRegisterIndex(RegisterType::COIL, localIndex);
    return (uint8_t)_readRegister(index, RegisterType::COIL);
}

void BaseDevice::coilWrite(uint8_t localIndex, uint8_t value)
{
    uint8_t index = _getRegisterIndex(RegisterType::COIL, localIndex);
    _writeRegister(index, RegisterType::COIL, value);
}

void BaseDevice::discreteInputWrite(uint8_t localIndex, uint8_t value)
{
    uint8_t index = _getRegisterIndex(RegisterType::DISCRETE_INPUT, localIndex);
    _writeRegister(index, RegisterType::DISCRETE_INPUT, value);
}

void BaseDevice::inputRegisterWrite(uint8_t localIndex, uint16_t value)
{
    uint8_t index = _getRegisterIndex(RegisterType::INPUT_REGISTER, localIndex);
    _writeRegister(index, RegisterType::INPUT_REGISTER, value);
}

uint16_t BaseDevice::holdingRegisterRead(uint8_t localIndex)
{
    uint8_t index = _getRegisterIndex(RegisterType::HOLDING_REGISTER, localIndex);
    return _readRegister(index, RegisterType::HOLDING_REGISTER);
}

void BaseDevice::holdingRegisterWrite(uint8_t localIndex, uint16_t value)
{
    uint8_t index = _getRegisterIndex(RegisterType::HOLDING_REGISTER, localIndex);
    _writeRegister(index, RegisterType::HOLDING_REGISTER, value);
}

//
// Public static methods
//

uint8_t BaseDevice::getCoilCount()
{
    return _nextCoil;
}

uint8_t BaseDevice::getDiscreteInputCount()
{
    return _nextDiscreteInput;
}

uint8_t BaseDevice::getInputRegisterCount()
{
    return _nextInputRegister;
}

uint8_t BaseDevice::getHoldingRegisterCount()
{
    return _nextHoldingRegister;
}

void BaseDevice::printErrors(HardwareSerial &serial)
{
    if (_errorCount != 0)
    {
        static uint8_t lastReportedError = 0;
        if (lastReportedError != _errorCount)
        {
            serial.println("\n=== Errors ===");
            for (uint8_t i = lastReportedError; i < _errorCount; i++)
            {
                serial.print("\t");
                serial.print(_errorDeviceNames[i]);
                serial.print(": ");
                serial.print(_errors[i]);
                if (_errorPins[i] != INVALID_PIN)
                {
                    serial.print(": pin = ");
                    serial.print(_errorPins[i]);
                }
                if (_errorIndices[i] != INVALID_REGISTER_INDEX)
                {
                    serial.print(": index = 0x");
                    if (_errorIndices[i] < 0x10)
                    {
                        serial.print('0');
                    }
                    serial.print(_errorIndices[i], HEX);
                }
                serial.println();
            }
            lastReportedError = _errorCount;
            serial.println("==============");
        }
    }
}

void BaseDevice::printRegisterMap(HardwareSerial &serial)
{
    serial.println("\n=== Register Map ===");

    const char *titles[] = {"COILS (FC01/FC05)", "DISCRETE INPUTS (FC02)", "INPUT REGISTERS (FC04)", "HOLDING REGISTERS (FC03/FC06)"};
    RegisterType types[] = {RegisterType::COIL, RegisterType::DISCRETE_INPUT, RegisterType::INPUT_REGISTER, RegisterType::HOLDING_REGISTER};
    constexpr uint8_t TYPE_COUNT = sizeof(types) / sizeof(types[0]);

    for (uint8_t i = 0; i < TYPE_COUNT; i++)
    {
        serial.println(titles[i]);
        bool any = false;

        for (uint8_t j = 0; j < _registerCount; j++)
        {
            if (_registerTypes[j] == types[i])
            {
                any = true;
                serial.print("\t0x");
                if (_registerAddresses[j] < 0x10)
                {
                    serial.print('0');
                }
                serial.print(_registerAddresses[j], HEX);
                serial.print(":\t");
                serial.print(_deviceNames[_registerOwners[j]]);
                serial.print(":\t");
                serial.println(_registerDescriptions[j]);
            }
        }

        if (!any)
        {
            serial.println("\t(empty)");
        }
    }
    serial.println("==============");
}

//
// Private methods
//

uint8_t BaseDevice::_getRegisterIndex(RegisterType type, uint8_t localIndex)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < _registerCount; i++)
    {
        if (_registerTypes[i] == type && _registerOwners[i] == _id)
        {
            if (count == localIndex)
            {
                return i;
            }
            count++;
        }
    }
    _addError("Register index out of range", INVALID_PIN, localIndex);

    if (inUpdateLoop)
    {
        longjmp(updateJumpBuffer, 1);
    }

    return INVALID_REGISTER_INDEX;
}

void BaseDevice::_writeRegister(uint8_t index, RegisterType type, uint16_t value)
{
    if (_valid && index != INVALID_REGISTER_INDEX)
    {
        uint8_t address = _registerAddresses[index];
        switch (type)
        {
        case RegisterType::COIL:
        {
            ModbusRTUServer.coilWrite(address, value);
            break;
        }
        case RegisterType::DISCRETE_INPUT:
        {
            ModbusRTUServer.discreteInputWrite(address, value);
            break;
        }
        case RegisterType::INPUT_REGISTER:
        {
            ModbusRTUServer.inputRegisterWrite(address, value);
            break;
        }
        case RegisterType::HOLDING_REGISTER:
        {
            ModbusRTUServer.holdingRegisterWrite(address, value);
            break;
        }
        }
    }
}

uint16_t BaseDevice::_readRegister(uint8_t index, RegisterType type)
{
    if (!_valid || index == INVALID_REGISTER_INDEX)
    {
        return 0;
    }

    uint8_t addr = _registerAddresses[index];
    uint16_t value = 0;
    switch (type)
    {
    case RegisterType::COIL:
    {
        value = ModbusRTUServer.coilRead(addr);
        break;
    }
    case RegisterType::HOLDING_REGISTER:
    {
        value = ModbusRTUServer.holdingRegisterRead(addr);
        break;
    }
    default:
    {
        return 0;
    }
    }
    return value;
}

void BaseDevice::_addRegister(const char *description, RegisterType type)
{
    if (_registerCount >= MAX_REGISTERS)
    {
        _addError("Descriptor table full", INVALID_PIN, INVALID_REGISTER_INDEX);

        return;
    }

    uint8_t registerAddress = 0;
    switch (type)
    {
    case RegisterType::COIL:
    {
        registerAddress = _nextCoil++;
        break;
    }
    case RegisterType::DISCRETE_INPUT:
    {
        registerAddress = _nextDiscreteInput++;
        break;
    }
    case RegisterType::HOLDING_REGISTER:
    {
        registerAddress = _nextHoldingRegister++;
        break;
    }
    case RegisterType::INPUT_REGISTER:
    {
        registerAddress = _nextInputRegister++;
        break;
    }
    }

    _registerAddresses[_registerCount] = registerAddress;
    _registerTypes[_registerCount] = type;
    _registerOwners[_registerCount] = _id;
    _registerDescriptions[_registerCount] = description;
    _registerCount++;
}

void BaseDevice::_addError(const char *message, uint32_t pin, uint8_t index)
{
    _valid = false;

    if (_errorCount < MAX_ERRORS)
    {
        for (uint8_t i = 0; i < _errorCount; i++)
        {
            if (_errors[i] == message && _errorDeviceNames[i] == _deviceNames[_id] &&
                _errorPins[i] == pin && _errorIndices[i] == index)
            {
                return;
            }
        }

        _errors[_errorCount] = message;
        _errorDeviceNames[_errorCount] = _deviceNames[_id];
        _errorPins[_errorCount] = pin;
        _errorIndices[_errorCount] = index;
        _errorCount++;
    }
}