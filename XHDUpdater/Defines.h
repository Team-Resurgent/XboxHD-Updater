#pragma once

#define I2C_SLAVE_ADDR 0x69
#define I2C_WRITE_BIT 0x80

// Read Actions
#define I2C_HDMI_COMMAND_READ_CONFIG 0 // Read value from config at current bank + index (post increments)
#define I2C_HDMI_COMMAND_READ_VERSION1 1 // Read version byte 1
#define I2C_HDMI_COMMAND_READ_VERSION2 2 // Read version byte 2
#define I2C_HDMI_COMMAND_READ_VERSION3 3 // Read version byte 3
#define I2C_HDMI_COMMAND_READ_VERSION4 4 // Read version byte 4
#define I2C_HDMI_COMMAND_READ_MODE 5 // Read current mode (1=Bootloader 2=Application)

#define I2C_HDMI_COMMAND_READ_RAM 6 // Read value from ram buffer at bank + index (post increments)
#define I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC1 7 // Read crc byte 1 (from ram buffer)
#define I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC2 8 // Read crc byte 2 (from ram buffer)
#define I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC3 9 // Read crc byte 3 (from ram buffer)
#define I2C_HDMI_COMMAND_READ_RAM_PAGE_CRC4 10 // Read crc byte 4 (from ram buffer)

// Write Actions
#define I2C_HDMI_COMMAND_WRITE_CONFIG 128 // Write value to config buffer at current bank + index (post increments)
#define I2C_HDMI_COMMAND_WRITE_CONFIG_BANK 129 // Write config buffer bank (sets index to 0)
#define I2C_HDMI_COMMAND_WRITE_CONFIG_INDEX 130 // Write config buffer index
#define I2C_HDMI_COMMAND_WRITE_CONFIG_APPLY 131 // Applies config when value is 1
#define I2C_HDMI_COMMAND_WRITE_SET_MODE 132 // Reboots to Bootloader or Application (1=Bootloader 2=Application) if not already in that mode

#define I2C_HDMI_COMMAND_WRITE_READ_PAGE 133 // Reads page using given value into ram buffer
#define I2C_HDMI_COMMAND_WRITE_RAM 134 // Write value to ram buffer at bank + index (post increments)
#define I2C_HDMI_COMMAND_WRITE_RAM_BANK 135 // Write ram buffer bank (sets index to 0)
#define I2C_HDMI_COMMAND_WRITE_RAM_INDEX 136 // Write ram buffer index
#define I2C_HDMI_COMMAND_WRITE_RAM_APPLY 137 // Applies ram buffer to page with value alseo erases + updates crc (validates page for current mode)

#define I2C_HDMI_COMMAND_WRITE_APP_FLASH_MODE 138 // 0 to mark flash ended else to mark flashing began

#define I2C_HDMI_VERSION1 1
#define I2C_HDMI_VERSION2 2
#define I2C_HDMI_VERSION3 3
#define I2C_HDMI_VERSION4 4

#define I2C_HDMI_MODE_BOOTLOADER 1
#define I2C_HDMI_MODE_APPLICATION 2

#define XHD_BOOTLOADER_BANK_START 20
#define XHD_BOOTLOADER_SIZE (XHD_BOOTLOADER_BANK_START * 1024)

#define SMBDEV_PIC16L 0x20
#define PIC16L_CMD_POWER 0x02
#define POWER_SUBCMD_CYCLE 0x40