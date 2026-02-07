#pragma once

#include "External.h"

class Utility
{
public:
    enum EncoderEnum {
        EncoderConexant,
        EncoderFocus,
        EncoderXcalibur
    };
    enum HdmiDeviceEnum {
        HdmiDeviceNone,
        HdmiDeviceXHD,
        HdmiDeviceHDPlus,
    };
    static Utility::EncoderEnum GetEncoder();
    static Utility::HdmiDeviceEnum GetHdmiDevice();
    static void Utility::Reboot();
    static uint8_t* LoadFirmware(uint32_t* firmwareSize);
};
