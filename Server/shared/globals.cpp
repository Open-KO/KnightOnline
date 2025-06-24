#include "stdafx.h"
#include <random>
#include <mutex>

static std::mt19937 s_randomNumberGenerator;
static std::recursive_mutex s_rngLock;
static bool s_rngSeeded = false;

void SeedRNG()
{
	if (!s_rngSeeded)
	{
		s_randomNumberGenerator.seed((uint32_t) getMSTime());
		s_rngSeeded = true;
	}
}

uint64_t RandUInt64()
{
	std::lock_guard<std::recursive_mutex> lock(s_rngLock);
	SeedRNG();
	std::uniform_int_distribution<uint64_t> dist;
	return dist(s_randomNumberGenerator);
}

time_t getMSTime()
{
#ifdef _WIN32
#if WINVER >= 0x0600
	typedef ULONGLONG(WINAPI* GetTickCount64_t)(void);
	static GetTickCount64_t pGetTickCount64 = nullptr;

	if (!pGetTickCount64)
	{
		HMODULE hModule = LoadLibraryA("KERNEL32.DLL");
		pGetTickCount64 = (GetTickCount64_t) GetProcAddress(hModule, "GetTickCount64");
		if (!pGetTickCount64)
			pGetTickCount64 = (GetTickCount64_t) GetTickCount;
		FreeLibrary(hModule);
	}

	return pGetTickCount64();
#else
	return GetTickCount();
#endif
#else
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (tv.tv_sec * SECOND) + (tv.tv_usec / SECOND);
#endif
}
