#pragma once

#include <network/TcpServerSocket.h>

class CVersionManagerDlg;
class CUser : public TcpServerSocket
{
public:
	CUser(CVersionManagerDlg* main, SocketManager* socketManager);
	bool PullOutCore(char*& data, int& length) override;
	int Send(char* pBuf, int length) override;
	void Parsing(int len, char* pData) override;
	void NewsReq(char* pBuf);
	void SendDownloadInfo(int version);
	void LogInReq(char* pBuf);

protected:
	CVersionManagerDlg* _main;
};
