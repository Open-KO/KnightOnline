#include "StdAfx.h"
#include "ItemManagerReadQueueThread.h"
#include "ItemManagerDlg.h"

ItemManagerReadQueueThread::ItemManagerReadQueueThread(CItemManagerDlg* main)
	: ReadQueueThread(main->m_LoggerRecvQueue),
	_main(main)
{
}

void ItemManagerReadQueueThread::process_packet(const char* buffer, int len)
{
	int index = 0;
	uint8_t command = GetByte(buffer, index);
	switch (command)
	{
		case WIZ_ITEM_LOG:
			_main->ItemLogWrite(buffer + index);
			break;

		case WIZ_DATASAVE:
			_main->ExpLogWrite(buffer + index);
			break;
	}
}
