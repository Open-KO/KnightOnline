#include "pch.h"
#include "ItemManagerReadQueueThread.h"
#include "ItemManagerApp.h"

ItemManagerReadQueueThread::ItemManagerReadQueueThread()
	: ReadQueueThread(ItemManagerApp::instance()->LoggerRecvQueue)
{
}

void ItemManagerReadQueueThread::process_packet(const char* buffer, int /*len*/)
{
	ItemManagerApp* appInstance = ItemManagerApp::instance();

	int index = 0;
	uint8_t command = GetByte(buffer, index);
	switch (command)
	{
		case WIZ_ITEM_LOG:
			appInstance->ItemLogWrite(buffer + index);
			break;

		case WIZ_DATASAVE:
			appInstance->ExpLogWrite(buffer + index);
			break;
	}
}
