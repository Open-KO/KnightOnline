#ifndef SERVER_EBENEZER_BOTUSER_H
#define SERVER_EBENEZER_BOTUSER_H

#pragma once

#include "BotTypes.h"
#include "User.h"

namespace Ebenezer
{

class CBotUser final : public CUser
{
public:
	CBotUser();
	bool InitializeBot(const BotSpawnRequest& request);
	int Send(char* pBuf, int length) override;
	BotRuntime& Runtime();
	const BotRuntime& Runtime() const;

private:
	_USER_DATA _userData {};
	BotRuntime _runtime {};
	bool _initialized = false;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_BOTUSER_H
