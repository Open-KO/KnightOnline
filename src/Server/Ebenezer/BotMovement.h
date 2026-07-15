#ifndef SERVER_EBENEZER_BOTMOVEMENT_H
#define SERVER_EBENEZER_BOTMOVEMENT_H

#pragma once

#include "BotTypes.h"

#include <cstdint>

namespace Ebenezer
{
class CBotUser;

class BotMovement
{
public:
	static BotSpawnPoint NextStep(
		const CBotUser& source, float targetX, float targetZ, float maxStep);
	static bool Move(CBotUser& source, const BotSpawnPoint& destination, int16_t speed);
};
} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTMOVEMENT_H
