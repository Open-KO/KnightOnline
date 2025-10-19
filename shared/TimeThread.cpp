#include "stdafx.h"
#include "TimeThread.h"

#include <time.h>

time_t UNIXTIME; // update this routinely to avoid the expensive time() syscall!
tm g_localTime;

static HANDLE s_hTimeThread = nullptr;
static bool s_bRunningThread;

DWORD WINAPI TimeThread(void* lpParam);

void StartTimeThread()
{
	UNIXTIME = time(nullptr); // update it first, just to ensure it's set when we need to use it.
	localtime_s(&g_localTime, &UNIXTIME);

	DWORD dwThreadId;
	s_hTimeThread = CreateThread(nullptr, 0, TimeThread, nullptr, 0, &dwThreadId); 
}

void CleanupTimeThread()
{
	printf("Waiting for time thread to shutdown...");
	s_bRunningThread = false;

	if (s_hTimeThread != nullptr)
	{
		WaitForSingleObject(s_hTimeThread, INFINITE);
		CloseHandle(s_hTimeThread);
	}

	printf(" done.\n");
}

DWORD WINAPI TimeThread(void* lpParam)
{
	s_bRunningThread = true;
	while (s_bRunningThread)
	{
		time_t t = time(nullptr);
		if (UNIXTIME != t)
		{
			UNIXTIME = t;
			localtime_s(&g_localTime, &t);
		}

		Sleep(1000); // might need to run it twice a second 
	}

	return 0;
}
