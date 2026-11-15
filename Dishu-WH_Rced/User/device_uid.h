#ifndef DEVICE_UID_H
#define DEVICE_UID_H

#include <stdint.h>

#define PAIRED_UID_CAPACITY   50U

uint8_t Pairing_IsPaired(uint32_t uid);
uint8_t Pairing_Add(uint32_t uid);
void Pairing_Init(void);

#endif /* DEVICE_UID_H */
