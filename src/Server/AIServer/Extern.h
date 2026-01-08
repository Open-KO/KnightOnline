#ifndef SERVER_AISERVER_EXTERN_H
#define SERVER_AISERVER_EXTERN_H

#pragma once

#include <AIServer/model/AIServerModel.h>

namespace AIServer
{

namespace model = aiserver_model;

struct _PARTY_GROUP
{
	uint16_t wIndex;
	int16_t uid[8]; // 하나의 파티에 8명까지 가입가능

	_PARTY_GROUP()
	{
		wIndex = 0;
		for (int i = 0; i < 8; i++)
			uid[i] = -1;
	}
};

} // namespace AIServer

#endif // SERVER_AISERVER_EXTERN_H
