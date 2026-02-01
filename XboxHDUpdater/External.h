#pragma once

#include "Integers.h"
#include <xtl.h>

typedef LONG NTSTATUS;

extern "C" {

    VOID WINAPI HalReturnToFirmware(unsigned int value);
    NTSTATUS WINAPI HalWriteSMBusValue(UCHAR devddress, UCHAR offset, UCHAR writedw, DWORD data);
    NTSTATUS WINAPI HalReadSMBusValue(UCHAR devddress, UCHAR offset, UCHAR readdw, DWORD* pdata);

}

#define HalReadSMBusByte(SlaveAddress, CommandCode, DataValue)                                                         \
    HalReadSMBusValue(SlaveAddress, CommandCode, FALSE, DataValue)
#define HalWriteSMBusByte(SlaveAddress, CommandCode, DataValue)                                                        \
    HalWriteSMBusValue(SlaveAddress, CommandCode, FALSE, DataValue)