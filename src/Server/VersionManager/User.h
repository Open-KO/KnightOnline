#ifndef SERVER_VERSIONMANAGER_USER_H
#define SERVER_VERSIONMANAGER_USER_H

#pragma once

#include <shared-server/TcpServerSocket.h>

namespace VersionManager
{

class CVersionManagerDlg;
class CUser : public TcpServerSocket
{
public:
	CUser(test_tag);
	CUser(TcpServerSocketManager* socketManager);
	std::string_view GetImplName() const override;
	bool PullOutCore(char*& data, int& length) override;
	int Send(char* pBuf, int length) override;
	void Parsing(int len, char* pData) override;
	void NewsReq();
	void SendDownloadInfo(int version);
	void LogInReq(char* pBuf);
	void SendAuthNotFound();
};

} // namespace VersionManager

#endif // SERVER_VERSIONMANAGER_USER_H
