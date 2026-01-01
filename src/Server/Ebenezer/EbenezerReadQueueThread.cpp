#include "pch.h"
#include "EbenezerReadQueueThread.h"
#include "EbenezerApp.h"
#include "User.h"

EbenezerReadQueueThread::EbenezerReadQueueThread()
	: ReadQueueThread(EbenezerApp::instance()->m_LoggerRecvQueue)
{
}

void EbenezerReadQueueThread::process_packet(const char* buffer, int len)
{
	EbenezerApp* appInstance = EbenezerApp::instance();

	int index = 0, uid = -1, send_index = 0, buff_length = 0;
	uint8_t command, result;
	char send_buff[1024] = {};
	CUser* pUser = nullptr;

	command = GetByte(buffer, index);
	uid = GetShort(buffer, index);

	if (command == (KNIGHTS_ALLLIST_REQ + 0x10)
		&& uid == -1)
	{
		appInstance->m_KnightsManager.RecvKnightsAllList(buffer + index);
		return;
	}

	pUser = appInstance->GetUserPtr(uid);
	if (pUser == nullptr)
		return;

	switch (command)
	{
		case WIZ_LOGIN:
			result = GetByte(buffer, index);
			if (result == 0xFF)
				memset(pUser->m_strAccountID, 0, sizeof(pUser->m_strAccountID));
			SetByte(send_buff, WIZ_LOGIN, send_index);
			SetByte(send_buff, result, send_index);					// 성공시 국가 정보
			pUser->Send(send_buff, send_index);
			break;

		case WIZ_SEL_NATION:
			SetByte(send_buff, WIZ_SEL_NATION, send_index);
			SetByte(send_buff, GetByte(buffer, index), send_index);	// 국가 정보
			pUser->Send(send_buff, send_index);
			break;

		case WIZ_NEW_CHAR:
			result = GetByte(buffer, index);
			SetByte(send_buff, WIZ_NEW_CHAR, send_index);
			SetByte(send_buff, result, send_index);					// 성공시 국가 정보
			pUser->Send(send_buff, send_index);
			break;

		case WIZ_DEL_CHAR:
			pUser->RecvDeleteChar(buffer + index);
		/*	result = GetByte( buffer, index );
			SetByte( send_buff, WIZ_DEL_CHAR, send_index );
			SetByte( send_buff, result, send_index );					// 성공시 국가 정보
			SetByte( send_buff, GetByte( buffer, index ), send_index );
			pUser->Send( send_buff, send_index );	*/
			break;

		case WIZ_SEL_CHAR:
			pUser->SelectCharacter(buffer + index);
			break;

		case WIZ_ALLCHAR_INFO_REQ:
			buff_length = GetShort(buffer, index);
			if (buff_length > len)
				break;

			SetByte(send_buff, WIZ_ALLCHAR_INFO_REQ, send_index);
			SetString(send_buff, buffer + index, buff_length, send_index);
			pUser->Send(send_buff, send_index);
			break;

		case WIZ_LOGOUT:
			if (pUser != nullptr
				&& strlen(pUser->m_pUserData->m_id) != 0)
			{
				spdlog::debug("EbenezerApp::ReadQueueThread: WIZ_LOGOUT [charId={}]",
					pUser->m_pUserData->m_id);
				pUser->Close();
			}
			break;

		case KNIGHTS_CREATE + 0x10:
		case KNIGHTS_JOIN + 0x10:
		case KNIGHTS_WITHDRAW + 0x10:
		case KNIGHTS_REMOVE + 0x10:
		case KNIGHTS_ADMIT + 0x10:
		case KNIGHTS_REJECT + 0x10:
		case KNIGHTS_CHIEF + 0x10:
		case KNIGHTS_VICECHIEF + 0x10:
		case KNIGHTS_OFFICER + 0x10:
		case KNIGHTS_PUNISH + 0x10:
		case KNIGHTS_DESTROY + 0x10:
		case KNIGHTS_MEMBER_REQ + 0x10:
		case KNIGHTS_STASH + 0x10:
		case KNIGHTS_LIST_REQ + 0x10:
		case KNIGHTS_ALLLIST_REQ + 0x10:
			appInstance->m_KnightsManager.ReceiveKnightsProcess(pUser, buffer + index, command);
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
	}
}
