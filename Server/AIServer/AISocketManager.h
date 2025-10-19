#pragma once

#include <shared-server/SocketManager.h>
#include "Define.h"

struct _SEND_DATA
{
	int16_t	sCurZone;				// 현재의 존
	int16_t	sLength;				// 패킷의 길이
	char	pBuf[MAX_PACKET_SIZE];	// 패킷의 내용..
};

class CGameSocket;
class SendThreadMain;
class AISocketManager : public SocketManager
{
public:
	AISocketManager();
	~AISocketManager() override;

	CGameSocket* GetServerSocket(int socketId) const;
	CGameSocket* GetServerSocketUnchecked(int socketId) const;

	void QueueSendData(_SEND_DATA* sendData);

protected:
	void StartUserThreads() override;
	void ShutdownUserThreads() override;

protected:
	SendThreadMain* _sendThreadMain;
};
