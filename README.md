

# Создание собственного устройства

```cpp
#pragma once
#include "BaseDevice.h"

class MyDevice : public BaseDevice
{
public:
    MyDevice(uint32_t pin) : _pin(pin)
    {
        setName("MyDevice");
        claimPin(pin);              // Зарезервировать пин
        addInputRegister("Value");  // Зарегистрировать регистры
        addHoldingRegister("Config");
    }

    void initialize() override
    {
        pinMode(getPin(_pin), INPUT_ANALOG);
        holdingRegisterWrite(0, 100); // Значение по умолчанию
    }

    void update() override
    {
        uint16_t config = holdingRegisterRead(0);
        uint16_t raw = analogRead(getPin(_pin));
        inputRegisterWrite(0, raw);
    }

private:
    uint32_t _pin;
};
```

- Вызывайте `claimPin()` и `addXxxRegister()` только в **конструкторе**.
- Используйте `getPin()` вместо прямого обращения к номеру пина — он проверяет владение.
- Если `getPin()` или операция с регистром завершается ошибкой, устройство помечается `_valid = false` и исключается из цикла обновлений.
- Индекс в методах `coilWrite`, `inputRegisterWrite` и т.д. локальные: 0 = первый регистр данного типа у данного устройства, перевод локального индекса в глобальный Modbus-адрес происходит автоматически.
- Адреса назначаются в порядке регистрации: первый `addInputRegister()` получает адрес 0, второй — 1 и т.д., независимо от того, какому устройству он принадлежит. Порядок определяется порядком вызова конструкторов в массиве `devices[]`
- Ошибки накапливаются в статическом буфере. Устройство с ошибкой помечается невалидным и пропускается в `initialize()` и `update()`.
- Порядок создания объектов в `devices[]` определяет карту регистров — изменение порядка меняет адреса