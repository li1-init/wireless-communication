#include "device_uid.h"
#include "board_config.h"
#include "platform.h"

static uint32_t pairedUids[PAIRED_UID_CAPACITY];
static uint8_t pairedCount;
static uint8_t pairingValid;

static uint32_t Pairing_Read32(uint32_t address)
{
    return ((uint32_t)EEPROM_ByteRD(address)) |
           ((uint32_t)EEPROM_ByteRD(address + 1U) << 8) |
           ((uint32_t)EEPROM_ByteRD(address + 2U) << 16) |
           ((uint32_t)EEPROM_ByteRD(address + 3U) << 24);
}

static void Pairing_Write32(uint32_t address, uint32_t value)
{
    EEPROM_ByteWR(address, (uint8_t)value);
    EEPROM_ByteWR(address + 1U, (uint8_t)(value >> 8));
    EEPROM_ByteWR(address + 2U, (uint8_t)(value >> 16));
    EEPROM_ByteWR(address + 3U, (uint8_t)(value >> 24));
}

void Pairing_Init(void)
{
    uint8_t i;

    pairedCount = 0U;
    pairingValid = 0U;

    if (Pairing_Read32(EEPROM_UID_MAGIC_ADDR) != PAIRED_UID_MAGIC)
    {
        return;
    }

    pairedCount = EEPROM_ByteRD(EEPROM_UID_COUNT_ADDR);
    if (pairedCount > PAIRED_UID_CAPACITY)
    {
        pairedCount = 0U;
        return;
    }

    for (i = 0U; i < pairedCount; ++i)
    {
        pairedUids[i] =
            Pairing_Read32(EEPROM_UID_START_ADDR + (uint32_t)i * 4U);
    }
    pairingValid = 1U;
}

uint8_t Pairing_IsPaired(uint32_t uid)
{
    uint8_t i;

    if ((pairingValid == 0U) || (uid == 0U) || (uid == 0xFFFFFFFFUL))
    {
        return 0U;
    }

    for (i = 0U; i < pairedCount; ++i)
    {
        if (pairedUids[i] == uid)
        {
            return 1U;
        }
    }
    return 0U;
}

uint8_t Pairing_Add(uint32_t uid)
{
    uint32_t address;

    if ((uid == 0U) || (uid == 0xFFFFFFFFUL))
    {
        return 0U;
    }
    if (Pairing_IsPaired(uid) != 0U)
    {
        return 1U;
    }
    if (pairedCount >= PAIRED_UID_CAPACITY)
    {
        return 0U;
    }

    address = EEPROM_UID_START_ADDR + (uint32_t)pairedCount * 4U;
    Pairing_Write32(address, uid);
    pairedUids[pairedCount] = uid;
    ++pairedCount;

    EEPROM_ByteWR(EEPROM_UID_COUNT_ADDR, pairedCount);
    Pairing_Write32(EEPROM_UID_MAGIC_ADDR, PAIRED_UID_MAGIC);
    pairingValid = 1U;
    return 1U;
}
