#include "pch.h"
#include <chrono>
#include <climits>
#include <mutex>
#include <random>

static std::mt19937 s_randomNumberGenerator;
static std::recursive_mutex s_rngLock;
static bool s_rngSeeded = false;

void SeedRNG()
{
	if (!s_rngSeeded)
	{
		uint32_t nowTime = static_cast<uint32_t>(
			std::chrono::high_resolution_clock::now().time_since_epoch().count() & UINT_MAX);
		s_randomNumberGenerator.seed(nowTime);
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

double TimeGet()
{
	using clock                           = std::chrono::high_resolution_clock;
	static const auto startTime           = clock::now();

	std::chrono::duration<double> elapsed = clock::now() - startTime;
	return elapsed.count(); // seconds as double
}
