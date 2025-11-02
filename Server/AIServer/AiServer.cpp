// AiServer.cpp : contains the main() function to start the server
//

#include "stdafx.h"
#include "ServerDlg.h"

#include <iostream>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int main()
{
	CServerDlg serverInstance = CServerDlg();

	// TODO:
	// 1. Need some sort of polling/wait mechanism on the serverInstance.
	// Server currently closes when SocketManager::AsyncAccept times out.
	// Polling func should catch this somehow and reset the socket instead of
	// crashing to desktop.
	// 2. Need a way to gracefully close the server.  Have to look at catching
	// kill/close signals.
	// while (serverInstance.isRunning())
	// {
	// 	
	// }

	return 0;
}
