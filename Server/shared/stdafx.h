#pragma once

#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include <Windows.h>
#include "DebugUtils.h"

// Ignore the warning "nonstandard extension used: enum '<enum name>' used in qualified name"
// Sometimes it's necessary to avoid collisions, but aside from that, specifying the enumeration helps make code intent clearer.
#pragma warning(disable: 4482)

#define STR(str) #str
#define STRINGIFY(str) STR(str)

// define compiler-specific types
#include "types.h"

#include "globals.h"
