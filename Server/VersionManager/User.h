#pragma once

#include <shared-server/TcpServerSocket.h>

class CVersionManagerDlg;
class CUser : public TcpServerSocket
{
public:
	CUser(SocketManager* socketManager);
	bool PullOutCore(char*& data, int& length) override;
	int Send(char* pBuf, int length) override;
	void Parsing(int len, char* pData) override;
	void NewsReq();
	void SendDownloadInfo(int version);
	void LogInReq(char* pBuf);
};
