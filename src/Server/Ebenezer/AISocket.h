#ifndef SERVER_EBENEZER_AISOCKET_H
#define SERVER_EBENEZER_AISOCKET_H

#pragma once

#include <shared-server/TcpClientSocket.h>

#include "MagicProcess.h"

namespace Ebenezer
{

class EbenezerApp;
class CAISocket : public TcpClientSocket
{
private:
	EbenezerApp* _main          = nullptr;
	CMagicProcess _magicProcess = {};

public:
	CAISocket(TcpClientSocketManager* socketManager);
	std::string_view GetImplName() const override;

	bool PullOutCore(char*& data, int& length) override;
	int Send(char* pBuf, int length) override;

	void Parsing(int len, char* pData) override;
	void CloseProcess() override;

	// Packet recv
	void LoginProcess(char* pBuf);
	void RecvCheckAlive();
	void RecvServerInfo(char* pBuf);
	void RecvNpcInfoAll(char* pBuf);
	void RecvNpcMoveResult(char* pBuf);
	void RecvNpcAttack(char* pBuf);
	void RecvMagicAttackResult(char* pBuf);
	void RecvNpcInfo(char* pBuf);
	void RecvUserHP(char* pBuf);
	void RecvUserExp(char* pBuf);
	void RecvSystemMsg(char* pBuf);
	void RecvNpcGiveItem(char* pBuf);
	void RecvUserFail(char* pBuf);
	void RecvCompressedData(char* pBuf);
	void RecvGateDestroy(char* pBuf);
	void RecvNpcDead(char* pBuf);
	void RecvNpcInOut(char* pBuf);
	void RecvBattleEvent(char* pBuf);
	void RecvNpcEventItem(char* pBuf);
	void RecvGateOpen(char* pBuf);
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_AISOCKET_H
