#pragma once

#include <stdint.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// base64 encode & decode
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32_t base64_encode(const char *input, const uint32_t input_len,
                       char *output, uint32_t output_len);
uint32_t base64_decode(const char *input, const uint32_t input_len,
                       char *output, uint32_t output_len);
