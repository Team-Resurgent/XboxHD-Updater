#include "HDHelper.h"
#include "External.h"
#include "TerminalBuffer.h"
#include "CRC32.h"

#include <xtl.h>

uint32_t HDHelper::ReadVersion()
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    DWORD v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION1, &v1) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION2, &v2) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION3, &v3) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION4, &v4) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    return ((uint32_t)(v1 & 0xFF) << 24) |
           ((uint32_t)(v2 & 0xFF) << 16) |
           ((uint32_t)(v3 & 0xFF) << 8)  |
           (uint32_t)(v4 & 0xFF);
}

uint8_t HDHelper::GetMode()
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    DWORD v;
    HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_MODE, &v);
    return (uint8_t)v;
}

void HDHelper::ChangeMode(uint8_t mode)
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_SET_MODE, mode);
    while (true)
    {
        Sleep(500);

        if (GetMode() == mode)
        {
            break;
        }
    }
}

uint32_t HDHelper::ReadPageChecksum(uint8_t page)
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_READ_PAGE, page);
    Sleep(250);

    DWORD v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC1, &v1) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC2, &v2) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC3, &v3) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC4, &v4) != 0) {
        return 0xFFFFFFFF;
    }
    Sleep(1);

    return ((uint32_t)(v1 & 0xFF) << 24) |
           ((uint32_t)(v2 & 0xFF) << 16) |
           ((uint32_t)(v3 & 0xFF) << 8)  |
           (uint32_t)(v4 & 0xFF);
}

void HDHelper::WritePage(uint8_t page, uint8_t* buffer)
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_RAM_BANK, 0);
    Sleep(1);

    for (uint32_t i = 0; i < 1024; i++)
    {
        HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_RAM, buffer[i]);
        Sleep(1);
    }

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_RAM_APPLY, page);
    Sleep(250);
}

bool HDHelper::FlashApplication(uint8_t* firmware, uint32_t firmwareSize)
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    uint32_t firmwareOffset = XHD_BOOTLOADER_SIZE;
    uint32_t bytesRemaining = firmwareSize - XHD_BOOTLOADER_SIZE;

    uint8_t* buffer = (uint8_t*)malloc(1024);
    if (buffer == NULL)
    {
        TerminalBuffer::Write("Allocate Memory Failed\n");
        return false;
    }

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_RAM_BANK, 0);
    Sleep(1);

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_APP_FLASH_MODE, 1);
    Sleep(1);

    uint8_t page = XHD_BOOTLOADER_BANK_START;
    while (bytesRemaining > 0)
    {
        uint32_t chunkSize = min(bytesRemaining, 1024);

        memset(buffer, 0xff, 1024);
        memcpy(buffer, firmware + firmwareOffset, chunkSize);
        uint32_t checksumBuffer = CRC32::Calculate(buffer, 1024);

        while (true)
        {
            TerminalBuffer::Write("Writing Page: %i (%08x)", page, checksumBuffer);
            WritePage(page, buffer);
            uint32_t checksumRam = ReadPageChecksum(page);
            if (checksumRam != checksumBuffer)
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

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_APP_FLASH_MODE, 0);
    Sleep(1);

    free(buffer);
    return true;
}

bool HDHelper::FlashBootloader(uint8_t* firmware, uint32_t firmwareSize)
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    uint32_t firmwareOffset = 0;
    uint32_t bytesRemaining = XHD_BOOTLOADER_SIZE;

    uint8_t* buffer = (uint8_t*)malloc(1024);
    if (buffer == NULL)
    {
        TerminalBuffer::Write("Allocate Memory Failed\n");
        return false;
    }

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_RAM_BANK, 0);
    Sleep(1);

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_APP_FLASH_MODE, 1);
    Sleep(1);

    uint8_t page = 0;
    while (bytesRemaining > 0)
    {
        uint32_t chunkSize = min(bytesRemaining, 1024);

        memset(buffer, 0xff, 1024);
        memcpy(buffer, firmware + firmwareOffset, chunkSize);
        uint32_t checksumBuffer = CRC32::Calculate(buffer, 1024);

        while (true)
        {
            TerminalBuffer::Write("Writing Page: %i (%08x)", page, checksumBuffer);
            WritePage(page, buffer);
            uint32_t checksumRam = ReadPageChecksum(page);
            if (checksumRam != checksumBuffer)
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

    HalWriteSMBusByte(slaveAddr, I2C_HDMI_COMMAND_WRITE_APP_FLASH_MODE, 0);
    Sleep(1);

    free(buffer);
    return true;
}