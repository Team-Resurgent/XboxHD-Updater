#pragma once

#include "External.h"
#include "Defines.h"
#include "Integers.h"

#include <string>

class HDHelper
{
public:
    static uint32_t ReadVersion();
    static uint8_t* LoadFirmware(uint32_t* firmwareSize);
    static uint8_t GetMode();
    static void ChangeMode(uint8_t mode);
    static uint32_t ReadPageChecksum(uint8_t page);
    static void WritePage(uint8_t page, uint8_t* buffer);
    static bool FlashApplication(uint8_t* firmware, uint32_t firmwareSize);
    static bool FlashBootloader(uint8_t* firmware, uint32_t firmwareSize);
};
