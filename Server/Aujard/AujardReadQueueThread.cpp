#include "StdAfx.h"
#include "AujardReadQueueThread.h"
#include "AujardDlg.h"

AujardReadQueueThread::AujardReadQueueThread(CAujardDlg* main)
	: ReadQueueThread(main->LoggerRecvQueue),
	_main(main)
{
}

void AujardReadQueueThread::process_packet(const char* buffer, int len)
{
	int index = 0;
	uint8_t command = GetByte(buffer, index);
	switch (command)
	{
		case WIZ_LOGIN:
			_main->AccountLogIn(buffer + index);
			break;

		case WIZ_NEW_CHAR:
			_main->CreateNewChar(buffer + index);
			break;

		case WIZ_DEL_CHAR:
			_main->DeleteChar(buffer + index);
			break;

		case WIZ_SEL_CHAR:
			_main->SelectCharacter(buffer + index);
			break;

		case WIZ_SEL_NATION:
			_main->SelectNation(buffer + index);
			break;

		case WIZ_ALLCHAR_INFO_REQ:
			_main->AllCharInfoReq(buffer + index);
			break;

		case WIZ_LOGOUT:
			_main->UserLogOut(buffer + index);
			break;

		case WIZ_DATASAVE:
			_main->UserDataSave(buffer + index);
			break;

		case WIZ_KNIGHTS_PROCESS:
			_main->KnightsPacket(buffer + index);
			break;

		case WIZ_CLAN_PROCESS:
			_main->KnightsPacket(buffer + index);
			break;

		case WIZ_LOGIN_INFO:
			_main->SetLogInInfo(buffer + index);
			break;

		case WIZ_KICKOUT:
			_main->UserKickOut(buffer + index);
			break;

		case WIZ_BATTLE_EVENT:
			_main->BattleEventResult(buffer + index);
			break;

		case DB_COUPON_EVENT:
			_main->CouponEvent(buffer + index);
			break;

		case DB_HEARTBEAT:
			_main->HeartbeatReceived();
			break;
	}
}
