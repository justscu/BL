#pragma once

#include <stdint.h>

uint16_t crc16(const uint8_t *str, const int32_t len);
uint32_t crc32(const uint8_t *str, const int32_t len);
