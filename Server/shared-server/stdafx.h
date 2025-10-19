#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include <Windows.h>
#endif

#include <stdint.h>
#include <inttypes.h>

#include <shared/DebugUtils.h>
#include <shared/globals.h>

#include <asio.hpp>
