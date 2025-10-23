#pragma once

#include <stdint.h>

#define STENCIL(n) void __attribute((preserve_none)) n(uint8_t cell, uint8_t *tape)

