#include "device_uid.h"
#include "board_config.h"
#include "platform.h"

static uint32_t cachedUid;

static uint32_t DeviceUID_ReadEeprom(void)
{
    return ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_ADDR)) |
           ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_ADDR + 1U) << 8) |
           ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_ADDR + 2U) << 16) |
           ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_ADDR + 3U) << 24);
}

static uint32_t DeviceUID_ReadMagic(void)
{
    return ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_MAGIC_ADDR)) |
           ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_MAGIC_ADDR + 1U) << 8) |
           ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_MAGIC_ADDR + 2U) << 16) |
           ((uint32_t)EEPROM_ByteRD(EEPROM_DEVICE_UID_MAGIC_ADDR + 3U) << 24);
}

static void DeviceUID_Write(uint32_t uid)
{
    EEPROM_ByteWR(EEPROM_DEVICE_UID_ADDR, (uint8_t)uid);
    EEPROM_ByteWR(EEPROM_DEVICE_UID_ADDR + 1U, (uint8_t)(uid >> 8));
    EEPROM_ByteWR(EEPROM_DEVICE_UID_ADDR + 2U, (uint8_t)(uid >> 16));
    EEPROM_ByteWR(EEPROM_DEVICE_UID_ADDR + 3U, (uint8_t)(uid >> 24));

    EEPROM_ByteWR(EEPROM_DEVICE_UID_MAGIC_ADDR,
                  (uint8_t)DEVICE_UID_MAGIC);
    EEPROM_ByteWR(EEPROM_DEVICE_UID_MAGIC_ADDR + 1U,
                  (uint8_t)(DEVICE_UID_MAGIC >> 8));
    EEPROM_ByteWR(EEPROM_DEVICE_UID_MAGIC_ADDR + 2U,
                  (uint8_t)(DEVICE_UID_MAGIC >> 16));
    EEPROM_ByteWR(EEPROM_DEVICE_UID_MAGIC_ADDR + 3U,
                  (uint8_t)(DEVICE_UID_MAGIC >> 24));
}

uint32_t DeviceUID_Get(void)
{
    uint32_t unique0;
    uint32_t unique1;
    uint32_t unique2;

    if (cachedUid != 0U)
    {
        return cachedUid;
    }

    if (DeviceUID_ReadMagic() == DEVICE_UID_MAGIC)
    {
        cachedUid = DeviceUID_ReadEeprom();
        if ((cachedUid != 0U) && (cachedUid != 0xFFFFFFFFUL))
        {
            return cachedUid;
        }
    }

    unique0 = *(volatile uint32_t *)0x1FF80050U;
    unique1 = *(volatile uint32_t *)0x1FF80054U;
    unique2 = *(volatile uint32_t *)0x1FF80058U;

    cachedUid = unique0 ^ (unique1 + 0x9E3779B9UL) ^
                (unique2 + 0x85EBCA6BUL);
    if ((cachedUid == 0U) || (cachedUid == 0xFFFFFFFFUL))
    {
        cachedUid = 0x5A17C3E1UL;
    }

    DeviceUID_Write(cachedUid);
    return cachedUid;
}
