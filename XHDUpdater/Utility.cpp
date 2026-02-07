#include "Utility.h"
#include "HDHelper.h"
#include "HDPlusHelper.h"

Utility::EncoderEnum Utility::GetEncoder() 
{
    DWORD temp = 0;
    if (HalReadSMBusByte(0x8A, 0x00, &temp) == 0) {
        return EncoderConexant;
    } else if (HalReadSMBusByte(0xD4, 0x00, &temp) == 0) {
        return EncoderFocus;
    }
    return EncoderXcalibur;
}

Utility::HdmiDeviceEnum Utility::GetHdmiDevice()
{
    if (HDHelper::ReadVersion() != 0xffffffff) {
        return HdmiDeviceXHD;
    } else if (HDPlusHelper::ReadVersion() != 0xffffffff) {
        return HdmiDeviceHDPlus;
    }
    return HdmiDeviceNone;
}

void Utility::Reboot() 
{
    HalWriteSMBusValue(SMBDEV_PIC16L, PIC16L_CMD_POWER, 0, POWER_SUBCMD_CYCLE);
}

uint8_t* Utility::LoadFirmware(uint32_t* firmwareSize)
{
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iostatusBlock;
    HANDLE fileHandle;
    NTSTATUS status;
    FILE_NETWORK_OPEN_INFORMATION fileInfo;

    char appPath[500];
	memset(&appPath, 0, sizeof(appPath));
	strncpy(appPath, XeImageFileName->Buffer, XeImageFileName->Length);
	strrchr(appPath, '\\')[0] = 0;
     
    std::string firmwarePath = appPath;
    EncoderEnum encoder = GetEncoder();
    if (encoder = EncoderConexant) {
        firmwarePath += "\\firmware_conexant.bin";
    } else if (encoder = EncoderFocus) {
        firmwarePath += "\\firmware_focus.bin";
    } else if (encoder = EncoderXcalibur) {
        firmwarePath += "\\firmware_xcalibur.bin";
    }

    STRING firmwarePathString;
    firmwarePathString.Buffer = (PSTR)firmwarePath.c_str();
    firmwarePathString.Length = strlen(firmwarePathString.Buffer);
    firmwarePathString.MaximumLength = firmwarePathString.Length + 1;

    objectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
    objectAttributes.ObjectName = &firmwarePathString;
    objectAttributes.RootDirectory = 0;

    status = NtOpenFile(&fileHandle, GENERIC_READ | SYNCHRONIZE, &objectAttributes, &iostatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (status != 0) 
    {
        return NULL;
    }

    status = NtQueryInformationFile(fileHandle, &iostatusBlock, &fileInfo, sizeof(fileInfo), FileNetworkOpenInformation);
    if (status != 0) 
    {
        NtClose(fileHandle);
        return NULL;
    }

    *firmwareSize = fileInfo.EndOfFile.LowPart;
    if (fileInfo.EndOfFile.HighPart > 0) 
    {
        NtClose(fileHandle);
        return NULL;
    }

    uint8_t* firmwareData = (uint8_t*)malloc(*firmwareSize);
    status = NtReadFile(fileHandle, NULL, NULL, NULL, &iostatusBlock, firmwareData, *firmwareSize, NULL);
    NtClose(fileHandle);

    if (status != 0)
    {
        free(firmwareData);
        return NULL;
    }

    return firmwareData;
}
