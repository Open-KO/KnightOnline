#ifndef CLIENT_N3BASE_AL_WRAPPER_H
#define CLIENT_N3BASE_AL_WRAPPER_H

#pragma once

#include <AL/al.h>
#include <climits>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define AL_CHECK_ERROR()       al_check_error_impl(__FILE__, __LINE__)
#define AL_CLEAR_ERROR_STATE() alGetError()
// NOLINTEND(cppcoreguidelines-macro-usage)

bool al_check_error_impl(const char* file, int line);

inline constexpr int MAX_AUDIO_SOURCE_IDS             = 128;
inline constexpr int MAX_AUDIO_STREAM_SOURCES         = 6;
inline constexpr size_t MAX_AUDIO_STREAM_BUFFER_COUNT = 8;
inline constexpr uint32_t INVALID_AUDIO_SOURCE_ID     = std::numeric_limits<uint32_t>::max();
inline constexpr uint32_t INVALID_AUDIO_BUFFER_ID     = std::numeric_limits<uint32_t>::max();

#endif // CLIENT_N3BASE_AL_WRAPPER_H
