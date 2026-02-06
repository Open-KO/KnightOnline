#include "pch.h"
#include "EbenezerReadQueueThread.h"
#include "EbenezerApp.h"
#include "User.h"

namespace Ebenezer
{

EbenezerReadQueueThread::EbenezerReadQueueThread() :
	ReadQueueThread(EbenezerApp::instance()->m_LoggerRecvQueue)
{
}

void EbenezerReadQueueThread::process_packet(const char* buffer, int len)
{
	EbenezerApp* appInstance = EbenezerApp::instance();

	int index = 0, uid = -1, sendIndex = 0, buff_length = 0;
	uint8_t command = 0, result = 0;
	char sendBuffer[1024] {};

	command = GetByte(buffer, index);
	uid     = GetShort(buffer, index);

	if (command == DB_KNIGHTS_ALLLIST_REQ && uid == -1)
	{
		appInstance->m_KnightsManager.RecvKnightsAllList(buffer + index);
		return;
	}

	auto pUser = appInstance->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	switch (command)
	{
		case WIZ_LOGIN:
			result = GetByte(buffer, index);
			if (result == 0xFF)
				memset(pUser->m_strAccountID, 0, sizeof(pUser->m_strAccountID));
			SetByte(sendBuffer, WIZ_LOGIN, sendIndex);
			SetByte(sendBuffer, result, sendIndex); // 성공시 국가 정보
			pUser->Send(sendBuffer, sendIndex);
			break;

		case WIZ_SEL_NATION:
			SetByte(sendBuffer, WIZ_SEL_NATION, sendIndex);
			SetByte(sendBuffer, GetByte(buffer, index), sendIndex); // 국가 정보
			pUser->Send(sendBuffer, sendIndex);
			break;

		case WIZ_NEW_CHAR:
			result = GetByte(buffer, index);
			SetByte(sendBuffer, WIZ_NEW_CHAR, sendIndex);
			SetByte(sendBuffer, result, sendIndex); // 성공시 국가 정보
			pUser->Send(sendBuffer, sendIndex);
			break;

		case WIZ_DEL_CHAR:
			pUser->RecvDeleteChar(buffer + index);
			/*	result = GetByte( buffer, index );
			SetByte( sendBuffer, WIZ_DEL_CHAR, sendIndex );
			SetByte( sendBuffer, result, sendIndex );					// 성공시 국가 정보
			SetByte( sendBuffer, GetByte( buffer, index ), sendIndex );
			pUser->Send( sendBuffer, sendIndex );	*/
			break;

		case WIZ_SEL_CHAR:
			pUser->SelectCharacter(buffer + index);
			break;

		case WIZ_ALLCHAR_INFO_REQ:
			buff_length = GetShort(buffer, index);
			if (buff_length > len)
				break;

			SetByte(sendBuffer, WIZ_ALLCHAR_INFO_REQ, sendIndex);
			SetString(sendBuffer, buffer + index, buff_length, sendIndex);
			pUser->Send(sendBuffer, sendIndex);
			break;

		case WIZ_LOGOUT:
			if (pUser != nullptr && strlen(pUser->m_pUserData->m_id) != 0)
			{
				spdlog::debug("EbenezerApp::ReadQueueThread: WIZ_LOGOUT [charId={}]",
					pUser->m_pUserData->m_id);
				pUser->Close();
			}
			break;

		case DB_KNIGHTS_CREATE:
		case DB_KNIGHTS_JOIN:
		case DB_KNIGHTS_WITHDRAW:
		case DB_KNIGHTS_REMOVE:
		case DB_KNIGHTS_ADMIT:
		case DB_KNIGHTS_REJECT:
		case DB_KNIGHTS_CHIEF:
		case DB_KNIGHTS_VICECHIEF:
		case DB_KNIGHTS_OFFICER:
		case DB_KNIGHTS_PUNISH:
		case DB_KNIGHTS_DESTROY:
		case DB_KNIGHTS_MEMBER_REQ:
		case DB_KNIGHTS_STASH:
		case DB_KNIGHTS_LIST_REQ:
		case DB_KNIGHTS_ALLLIST_REQ:
			appInstance->m_KnightsManager.ReceiveKnightsProcess(
				pUser.get(), buffer + index, command);
			break;

		case DB_LOGIN_INFO:
			result = GetByte(buffer, index);
			if (result == 0x00)
				pUser->Close();
			break;

		case DB_COUPON_EVENT:
			if (pUser != nullptr)
				pUser->CouponEvent(buffer + index);
			break;

		default:
			spdlog::error(
				"EbenezerReadQueueThread::process_packet: Unhandled opcode {:02X}", command);
			break;
	}
}

} // namespace Ebenezer
