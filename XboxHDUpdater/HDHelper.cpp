#include "HDHelper.h"

uint32_t HDHelper::ReadVersion()
{
    const UCHAR slaveAddr = (UCHAR)(I2C_SLAVE_ADDR << 1);

    DWORD v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION1, &v1) != 0)
        return 0xFFFFFFFF;
    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION2, &v2) != 0)
        return 0xFFFFFFFF;
    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION3, &v3) != 0)
        return 0xFFFFFFFF;
    if (HalReadSMBusByte(slaveAddr, I2C_HDMI_COMMAND_READ_VERSION4, &v4) != 0)
        return 0xFFFFFFFF;

    return ((uint32_t)(v1 & 0xFF) << 24) |
           ((uint32_t)(v2 & 0xFF) << 16) |
           ((uint32_t)(v3 & 0xFF) << 8)  |
           (uint32_t)(v4 & 0xFF);
}
