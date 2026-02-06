#ifndef SERVER_AISERVER_AISOCKETMANAGER_H
#define SERVER_AISERVER_AISOCKETMANAGER_H

#pragma once

#include "Define.h"

#include <shared-server/TcpServerSocketManager.h>

namespace AIServer
{

struct _SEND_DATA
{
	int16_t sCurZone;           // 현재의 존
	int16_t sLength;            // 패킷의 길이
	char pBuf[MAX_PACKET_SIZE]; // 패킷의 내용..
};

class CGameSocket;
class SendThreadMain;
class AISocketManager : public TcpServerSocketManager
{
public:
	AISocketManager();
	~AISocketManager() override;

	std::shared_ptr<CGameSocket> GetSocket(int socketId) const;
	std::shared_ptr<CGameSocket> GetSocketUnchecked(int socketId) const;

	void QueueSendData(_SEND_DATA* sendData);

protected:
	SendThreadMain* _sendThreadMain;
};

} // namespace AIServer

#endif // SERVER_AISERVER_AISOCKETMANAGER_H
