#include "HDPlusHelper.h"

#include "External.h"
#include "TerminalBuffer.h"
#include "CRC32.h"

uint32_t HDPlusHelper::ReadVersion()
{
    const UCHAR slaveAddr = (UCHAR)(HDPLUS_I2C_HDMI_ADDRESS1 << 1);

    DWORD v1 = 0, v2 = 0, v3 = 0;
    if (HalReadSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_READ_VERSION1, &v1) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_READ_VERSION2, &v2) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_READ_VERSION3, &v3) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    return ((uint32_t)(v1 & 0xFF) << 24) |
           ((uint32_t)(v2 & 0xFF) << 16) |
           ((uint32_t)(v3 & 0xFF) << 8);
}


uint8_t HDPlusHelper::GetMode()
{
    const UCHAR slaveAddr = (UCHAR)(HDPLUS_I2C_HDMI_ADDRESS1 << 1);

    DWORD v;
    HalReadSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_READ_MODE, &v);
    return (uint8_t)v;
}

void HDPlusHelper::ChangeMode(uint8_t mode, uint8_t modeCheck)
{
    const UCHAR slaveAddr = (UCHAR)(HDPLUS_I2C_HDMI_ADDRESS1 << 1);

    HalWriteSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_WRITE_SET_MODE, mode);
    while (true)
    {
        Sleep(500);

        if (GetMode() == modeCheck)
        {
            break;
        }
    }
}

void HDPlusHelper::WaitFlashReady()
{
    const UCHAR slaveAddr = (UCHAR)(HDPLUS_I2C_HDMI_ADDRESS1 << 1);

    DWORD v1 = 0, v2 = 0;

    while (true)
    {
        if (HalReadSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_READ_READY1, &v1) != 0) {
            continue;
        }
        Sleep(1);

        if (HalReadSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_READ_READY2, &v2) != 0) {
            continue;
        }
        Sleep(1);

        break;
    }
}

bool HDPlusHelper::FlashOk()
{
    const UCHAR slaveAddr = (UCHAR)(HDPLUS_I2C_HDMI_ADDRESS1 << 1);

    DWORD v = 0;
    if (HalReadSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_READ_FLASH_OK, &v) != 0) {
        return false;
    }
    Sleep(1);

    return v == 0;
}

void HDPlusHelper::WritePage(uint8_t page, uint8_t* buffer)
{
    const UCHAR slaveAddr = (UCHAR)(HDPLUS_I2C_HDMI_ADDRESS1 << 1);

    HalWriteSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_WRITE_SET_PAGE, page);
    Sleep(1);

    for (uint32_t i = 0; i < 1024; i++)
    {
        HalWriteSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_WRITE_SET_FLASH, buffer[i]);
        Sleep(1);
    }

    Sleep(250);
}

bool HDPlusHelper::FlashApplication(uint8_t* firmware, uint32_t firmwareSize)
{
    const UCHAR slaveAddr = (UCHAR)(HDPLUS_I2C_HDMI_ADDRESS1 << 1);

    uint32_t customFirmwareSize = (firmwareSize - XHD_BOOTLOADER_SIZE) + 0x2800;
    uint8_t* customFirmware = (uint8_t*)malloc(customFirmwareSize);
    if (customFirmware == NULL)
    {
        TerminalBuffer::Write("Allocate Memory Failed\n");
        return false;
    }

    uint32_t firmwareOffset = 0;
    uint32_t bytesRemaining = customFirmwareSize;
    memset(customFirmware, 0xff, customFirmwareSize);
    memcpy(customFirmware, firmware + XHD_BOOTLOADER_SIZE, 0x100);
    memcpy(customFirmware + 0x2800, firmware + XHD_BOOTLOADER_SIZE, firmwareSize - XHD_BOOTLOADER_SIZE);

    uint8_t* buffer = (uint8_t*)malloc(1024);
    if (buffer == NULL)
    {
        TerminalBuffer::Write("Allocate Memory Failed\n");
        return false;
    }

    uint8_t page = 0;
    while (bytesRemaining > 0)
    {
        uint32_t chunkSize = min(bytesRemaining, 1024);

        memset(buffer, 0xff, 1024);
        memcpy(buffer, customFirmware + firmwareOffset, chunkSize);
        uint32_t checksumBuffer = CRC32::Calculate(buffer, 1024);

        while (true)
        {
            TerminalBuffer::Write("Writing Page: %i (%08x)", page + 10, checksumBuffer);

            HalWriteSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC1, (DWORD)(checksumBuffer & 0xFF));
            Sleep(1);
            HalWriteSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC2, (DWORD)((checksumBuffer >> 8) & 0xFF));
            Sleep(1);
            HalWriteSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC3, (DWORD)((checksumBuffer >> 16) & 0xFF));
            Sleep(1);
            HalWriteSMBusByte(slaveAddr, HDPLUS_I2C_HDMI_COMMAND_WRITE_PAGE_CRC4, (DWORD)((checksumBuffer >> 24) & 0xFF));
            Sleep(1);

            WritePage(page, buffer);
            WaitFlashReady();

            bool flashOk = FlashOk();
            if (flashOk == false)
            {
                TerminalBuffer::Write(" - Failed (Retrying)\n");
                continue;
            }        
            TerminalBuffer::Write(" - OK\n");
            break;
        }

        firmwareOffset += chunkSize;
        bytesRemaining -= chunkSize;
        page++;
    }

    free(buffer);
    return true;
}