#include "pch.h"
#include "ZoneEventThread.h"
#include "Map.h"
#include "NpcThread.h"
#include "AIServerApp.h"

ZoneEventThread::ZoneEventThread()
	: Thread()
{
	_main = AIServerApp::instance();
}

void ZoneEventThread::thread_loop()
{
	while (_canTick)
	{
		double fCurrentTime = TimeGet();
		for (MAP* pMap : _main->_zones)
		{
			if (pMap == nullptr)
				continue;

			// 현재의 존이 던젼담당하는 존이 아니면 리턴..
			if (pMap->m_byRoomEvent == 0)
				continue;

			// 전체방이 클리어 되었다면
			if (pMap->IsRoomStatusCheck())
				continue;

			// 방번호는 1번부터 시작
			for (auto& [_, pRoom] : pMap->m_arRoomEventArray)
			{
				if (pRoom == nullptr)
					continue;

				// 1:init, 2:progress, 3:clear
				if (pRoom->m_byStatus == 1
					|| pRoom->m_byStatus == 3)
					continue;

				// 여기서 처리하는 로직...
				pRoom->MainRoom(fCurrentTime);
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
