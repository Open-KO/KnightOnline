#pragma once

#include <network/TcpClientSocket.h>

#include "MagicProcess.h"

class CEbenezerDlg;
class CAISocket : public TcpClientSocket
{
private:
	CEbenezerDlg*	_main;
	CMagicProcess	_magicProcess;
	int				_zoneNum;

public:
	int GetZoneNumber() const
	{
		return _zoneNum;
	}

	CAISocket(SocketManager* socketManager, int zoneNum);

	void Initialize() override;
	bool PullOutCore(char*& data, int& length) override;
	int Send(char* pBuf, int length) override;

	void Parsing(int len, char* pData) override;
	void CloseProcess() override;

	void InitEventMonster(int index);
	// Packet recv
	void LoginProcess(char* pBuf);
	void RecvCheckAlive(char* pBuf);
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
