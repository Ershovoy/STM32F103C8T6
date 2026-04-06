#pragma once

#include "BaseDevice.h"

class DeviceTemplate : public BaseDevice
{
public:
    DeviceTemplate()
    {
        setName("TemplateName");
        // claimPin();
        // addHoldingRegister();
    }

    void initialize() override
    {
    }
    
    void update() override
    {
    }
};