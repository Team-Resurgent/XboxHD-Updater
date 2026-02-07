#pragma once

#include "External.h"
#include "Defines.h"
#include "Integers.h"

#include <string>

#define HDPLUS_I2C_HDMI_ADDRESS1 0x44
#define HDPLUS_I2C_HDMI_ADDRESS2 0x43

#define HDPLUS_I2C_HDMI_COMMAND_READ_READY1 0x37
#define HDPLUS_I2C_HDMI_COMMAND_READ_READY2 0x38
#define HDPLUS_I2C_HDMI_COMMAND_READ_FLASH_OK 0x3C
#define HDPLUS_I2C_HDMI_COMMAND_READ_VERSION1 0x57
#define HDPLUS_I2C_HDMI_COMMAND_READ_VERSION2 0x58
#define HDPLUS_I2C_HDMI_COMMAND_READ_VERSION3 0x59 
#define HDPLUS_I2C_HDMI_COMMAND_READ_MODE 0x20

#define HDPLUS_I2C_HDMI_COMMAND_WRITE_SET_MODE 0x29
#define HDPLUS_I2C_HDMI_COMMAND_WRITE_SET_PAGE 0x2E
#define HDPLUS_I2C_HDMI_COMMAND_WRITE_SET_FLASH 0x2F
#define HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC1 0x38
#define HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC2 0x39
#define HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC3 0x3A 
#define HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC4 0x3B

#define HDPLUS_II2C_HDMI_MODE_BOOTLOADER 0x40
#define HDPLUS_II2C_HDMI_MODE_APPLICATION 0x00
#define HDPLUS_II2C_HDMI_MODE_ENABLE_FLASH 0x02
#define HDPLUS_II2C_HDMI_MODE_ENABLE_APPLICATION 0x60

class HDPlusHelper
{
public:

    static uint32_t ReadVersion();

    static uint8_t GetMode();
    static void ChangeMode(uint8_t mode, uint8_t modeCheck);
    static void WaitFlashReady();
    static bool FlashOk();
    static void WritePage(uint8_t page, uint8_t* buffer);
    static bool FlashApplication(uint8_t* firmware, uint32_t firmwareSize);
};
