#pragma once

#include <array>
#include <cstdint>

namespace api_socket
{

struct PayloadView
{
	const uint8_t* Data;
	int Size;
};

inline PayloadView ApplyMinimumUnencryptedPayloadCompatibility(
	const uint8_t* data, int size, std::array<uint8_t, 2>& storage) noexcept
{
	if (size == 1)
	{
		storage[0] = data[0];
		storage[1] = 0;
		return { storage.data(), static_cast<int>(storage.size()) };
	}

	return { data, size };
}

} // namespace api_socket
