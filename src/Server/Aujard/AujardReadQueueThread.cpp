#include "pch.h"
#include "AujardReadQueueThread.h"
#include "AujardApp.h"

namespace Aujard
{

AujardReadQueueThread::AujardReadQueueThread() :
	ReadQueueThread(AujardApp::instance()->LoggerRecvQueue)
{
}

void AujardReadQueueThread::process_packet(const char* buffer, int /*len*/)
{
	AujardApp* appInstance = AujardApp::instance();

	int index              = 0;
	uint8_t command        = GetByte(buffer, index);
	switch (command)
	{
		case WIZ_LOGIN:
			appInstance->AccountLogIn(buffer + index);
			break;

		case WIZ_NEW_CHAR:
			appInstance->CreateNewChar(buffer + index);
			break;

		case WIZ_DEL_CHAR:
			appInstance->DeleteChar(buffer + index);
			break;

		case WIZ_SEL_CHAR:
			appInstance->SelectCharacter(buffer + index);
			break;

		case WIZ_SEL_NATION:
			appInstance->SelectNation(buffer + index);
			break;

		case WIZ_ALLCHAR_INFO_REQ:
			appInstance->AllCharInfoReq(buffer + index);
			break;

		case WIZ_LOGOUT:
			appInstance->UserLogOut(buffer + index);
			break;

		case WIZ_DATASAVE:
			appInstance->UserDataSave(buffer + index);
			break;

		case WIZ_KNIGHTS_PROCESS:
			appInstance->KnightsPacket(buffer + index);
			break;

		case WIZ_LOGIN_INFO:
			appInstance->SetLogInInfo(buffer + index);
			break;

		case WIZ_KICKOUT:
			appInstance->UserKickOut(buffer + index);
			break;

		case WIZ_BATTLE_EVENT:
			appInstance->BattleEventResult(buffer + index);
			break;

		case DB_COUPON_EVENT:
			appInstance->CouponEvent(buffer + index);
			break;

		case DB_HEARTBEAT:
			appInstance->HeartbeatReceived();
			break;

		default:
			spdlog::error(
				"AujardReadQueueThread::process_packet: Unhandled opcode {:02X}", command);
			break;
	}
}

} // namespace Aujard
