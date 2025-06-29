#include "stdafx.h"
#include "DBAgent.h"
#include "../shared/DateTime.h"
#include <sstream>
#include <vector>

void CUser::AdminLogCommand(const std::string& sOperatorCommandType, const std::string& sUserID)
{
	DateTime time;
	g_pMain->WriteChatLogFile(string_format("[ GAME MASTER - %d:%d:%d ] %s : %s %s ( Zone=%d, X=%d, Z=%d )\n", time.GetHour(), time.GetMinute(), time.GetSecond(), GetName().c_str(), sOperatorCommandType.c_str(), sUserID.c_str(), GetZoneID(), uint16_t(GetX()), uint16_t(GetZ())));
}

void CUser::AdminLogBadAccess()
{
	DateTime time;
	g_pMain->WriteChatLogFile(string_format("[ BAD ACCESS - %d:%d:%d ] %s \n", time.GetHour(), time.GetMinute(), time.GetSecond(), GetName().c_str()));
}


//cmd : give_item UserName ItemID [optional 1 or 0 for notice]
//TODO: give item by name together with ID by using db search query
//exp: _raptor_9_0 (raptor 9 type 0)
void CUser::AdminGiveItemHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sItemID, sShowNotice,
		sNoticeMessage;

	bool bIsOnline = false, bGivenItemName = false;

	pkt >> sUserID >> sItemID >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	//example raptor_9_0, (9 is upgrade,0 offset type)
	if (sItemID.find('_') != std::string::npos)
		bGivenItemName = true;

	int32_t iItemID = 0;
	std::string strItemName;

	//user entered item name like raptor_4
	//this does not work because selecting with LIKE from ITEM table is too slow
	//but can be used if GetItemByName implemented in CSTLMap
	if (bGivenItemName)
	{

		std::vector<std::string> parts;
		std::stringstream ss(sItemID);
		std::string part;

		//divide by '_'
		while (std::getline(ss, part, '_'))
		{
			parts.push_back(part);
		}

		if (parts.empty())
			return;

		std::string itemName;
		std::string levelStr;
		std::string typeStr;
		bool hasLevel = false;
		bool hasType = false;

		size_t partCount = parts.size();

		// Check if last member is number or not
		if (partCount >= 1 && std::all_of(parts[partCount - 1].begin(), parts[partCount - 1].end(), ::isdigit))
		{
			if (partCount >= 2 && std::all_of(parts[partCount - 2].begin(), parts[partCount - 2].end(), ::isdigit))
			{
				// Exp: raptor_9_1
				levelStr = parts[partCount - 2];
				typeStr = parts[partCount - 1];
				hasLevel = true;
				hasType = true;

				// get name
				for (size_t i = 0; i < partCount - 2; ++i)
				{
					if (!itemName.empty()) itemName += " ";
					itemName += parts[i];
				}
			}
			else
			{
				// Exp: raptor_9 (user forget to enter offset, assume it 0 )
				levelStr = parts[partCount - 1];
				typeStr = "0";
				hasLevel = true;
				hasType = true;

				for (size_t i = 0; i < partCount - 1; ++i)
				{
					if (!itemName.empty()) itemName += " ";
					itemName += parts[i];
				}
			}
		}
		else
		{
			//this is also wrong because in DB blessed upgrade scroll (+0)
			// only letters example: blessed_upgrade_scroll
			for (size_t i = 0; i < partCount; ++i)
			{
				if (!itemName.empty()) itemName += " ";
				itemName += parts[i];
			}
		}

		uint8_t iItemType = 0;
		if (hasLevel && hasType)
		{
			strItemName = itemName + " (+" + levelStr + ")";
			iItemType = std::stoi(typeStr);
		}
		else
		{
			strItemName = itemName;
		}


		//call DBAgent and run LIKE query to find ItemID
		//BUG: too slow, removed. Need to find item with name from CSTLMap
		//mssql blocks game
		/*
		if (!g_DBAgent.GetItemID(strItemName, iItemType, iItemID))
		{
			SendOutputMsg("Db query failed, item id not found", false);
			return;
		}
		*/

	}
	else //user entered item ID
	{
		try
		{
			iItemID = sItemID.empty() ? 0 : std::stoi(sItemID);
		}
		catch (...)
		{
			SendOutputMsg("Wrong number format, entry includes letters.", false);
			return;
		}
	}



	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;


	if (!bIsOnline)
	{
		SendOutputMsg("user is offline", false);
		return;
	}

	int8_t bDstPos = pUser->FindSlotForItem(iItemID, 1);

	if (bDstPos < 0)
	{
		SendOutputMsg("No slots for the item", false);
		return;
	}

	// Check weight
	if (!pUser->CheckWeight(iItemID, 1))
	{
		SendOutputMsg("user weight exceeded.", false);
		return;
	}

	_ITEM_TABLE* pTable = g_pMain->GetItemPtr(iItemID);

	if (pTable == nullptr)
	{
		SendOutputMsg("wrong item ID", false);
		return;
	}



	_ITEM_DATA* pDstItem = &pUser->m_sItemArray[bDstPos];
	pDstItem->nNum = iItemID;
	pDstItem->sCount += 1;

	if (pDstItem->sCount > MAX_ITEM_COUNT)
	{
		SendOutputMsg("adding item exceeds count limit.", false);
		return;
	}

	pDstItem->nSerialNum = g_pMain->GenerateItemSerial();
	pDstItem->sDuration = pTable->m_sDuration;


	if (pTable->m_bCountable == TRUE)
	{
		SendOutputMsg("Use give_countable_item [username] [itemid] [amount] [1:0 for notice] to add countable items", false);
		return;
	}



	Packet result(WIZ_ADMIN_GIVE_ITEM);

	result << pDstItem->nNum;
	result << pDstItem->sCount;
	result << uint8_t(bDstPos - SLOT_MAX);

	pUser->Send(&result);
	pUser->SetUserAbility(false);
	pUser->SendItemWeight();

	SendOutputMsg("Item has been added successfully.");
	std::string sOperatorCommandType = "OPERATOR_GIVE_ITEM";
	AdminLogCommand(sOperatorCommandType, sUserID);

	if (sShowNotice == "1")
	{
		sNoticeMessage = string_format("[Player: %s] has been awarded by [GM: %s] with an item %s.", sUserID.c_str(), GetName().c_str(), pTable->m_sName.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}


}

//cmd : give_countable_item UserName ItemID Amount [optional 1 or 0 for notice]
void CUser::AdminGiveCountableItemHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sItemID, sCount, sShowNotice,
		sNoticeMessage;

	bool bIsOnline = false;

	pkt >> sUserID >> sItemID >> sCount >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	int32_t iItemID = 0;

	try
	{
		iItemID = sItemID.empty() ? 0 : std::stoi(sItemID);
	}
	catch (...)
	{
		SendOutputMsg("Wrong number format for item ID, entry includes letters.", false);
		return;
	}

	int32_t iCount = 0;

	try
	{
		iCount = sCount.empty() ? 0 : std::stoi(sCount);
	}
	catch (...)
	{
		SendOutputMsg("Wrong number format for item count, entry includes letters.", false);
		return;
	}


	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;


	if (!bIsOnline)
	{
		SendOutputMsg("Failed: User not Online.", false);
		return;
	}

	int8_t bDstPos = pUser->FindSlotForItem(iItemID, 1);

	if (bDstPos < 0)
	{
		SendOutputMsg("Failed: Appropriate slot could not be found for the Item.", false);
		return;
	}

	// Ensure there's enough room in this user's inventory.
	if (!pUser->CheckWeight(iItemID, iCount))
	{
		SendOutputMsg("Failed: User weight exceeded.", false);
		return;
	}

	_ITEM_TABLE* pTable = g_pMain->GetItemPtr(iItemID);

	if (pTable == nullptr)
	{
		SendOutputMsg("Failed: Wrong Item ID.", false);
		return;
	}



	_ITEM_DATA* pDstItem = &pUser->m_sItemArray[bDstPos];
	pDstItem->nNum = iItemID;
	pDstItem->sCount += iCount;

	if (pDstItem->sCount > MAX_ITEM_COUNT)
	{
		SendOutputMsg("Adding item has been failed because of count !", false);
		return;
	}

	pDstItem->nSerialNum = g_pMain->GenerateItemSerial();
	pDstItem->sDuration = pTable->m_sDuration;


	if (pTable->m_bCountable == FALSE)
	{
		SendOutputMsg("Use give_item [username] [itemid] [1:0 for notice] to add non countable items.", false);
		return;
	}



	Packet result(WIZ_ADMIN_GIVE_ITEM);

	result << pDstItem->nNum;
	result << pDstItem->sCount;
	result << uint8_t(bDstPos - SLOT_MAX);

	pUser->Send(&result);
	pUser->SetUserAbility(false);
	pUser->SendItemWeight();

	SendOutputMsg("Item has been added successfully.");
	std::string sOperatorCommandType = "OPERATOR_GIVE_ITEM";
	AdminLogCommand(sOperatorCommandType, sUserID);

	if (sShowNotice == "1")
	{
		sNoticeMessage = string_format("[Player: %s] has been awarded by [GM: %s] with %dx %s.", sUserID.c_str(), GetName().c_str(), iCount, pTable->m_sName.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}


}

//cmd : give_exp UserName Amount [optional 1 or 0 for notice]
void CUser::AdminGiveExpHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sAmount, sShowNotice;

	bool bIsOnline = false;

	std::string sNoticeMessage;

	pkt >> sUserID >> sAmount >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	int32_t iAmount = 0;

	try
	{
		iAmount = sAmount.empty() ? 0 : std::stoi(sAmount);
	}
	catch (...)
	{
		SendOutputMsg("Wrong number format, entry includes letters.", false);
		return;
	}


	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;


	if (!bIsOnline)
	{
		SendOutputMsg("Failed: User not Online.", false);
		return;
	}

	uint8_t iUserLevel = pUser->GetLevel();

	if (iAmount < 0) //negative exp
	{
		int64_t iUserTotalExp = 0;

		for (size_t i = 1; i < iUserLevel; i++)
		{
			iUserTotalExp += g_pMain->GetExpByLevel(i);
		}

		if (iUserTotalExp == 0)
		{
			SendOutputMsg("User has minimum experience.", false);
			return;
		}

		iUserTotalExp += pUser->m_iExp;

		if (abs(iAmount) > iUserTotalExp)
			iAmount = -iUserTotalExp;

		int64_t iRemovedExp = -iAmount;
		int64_t iRemainingExp = 0;
		int8_t iDownedLevel = 1;

		if (iRemovedExp > pUser->m_iExp)
		{ //removed exp bigger than level, calculate how many level user lost
			for (size_t i = pUser->GetLevel(); i >= 1; i--)
			{
				if (pUser->GetLevel() == i) //at first consider m_iExp
				{
					iRemovedExp -= pUser->m_iExp;
					continue;
				}
				else
				{
					iRemovedExp -= g_pMain->GetExpByLevel(i);
				}

				if (iRemovedExp <= 0)
				{
					iRemainingExp = -iRemovedExp;
					break;
				}

				iDownedLevel++;
			}

			for (size_t i = 0; i < iDownedLevel; i++)
			{
				pUser->m_bLevel--;
				pUser->LevelChange(pUser->m_bLevel, false);
			}

			pUser->m_iExp = iRemainingExp;

		}
		else //removed exp smaller than remaining level exp, user won't leveldown
		{
			pUser->m_iExp -= iRemovedExp;
		}

	}
	else //entered amount is bigger than 0 
	{
		uint64_t iUserTotalExp = 0;

		for (size_t i = 1; i < iUserLevel; i++)
		{
			iUserTotalExp += g_pMain->GetExpByLevel(i);
		}

		iUserTotalExp += pUser->m_iExp;

		uint64_t iMaxObtainableExp = 0;

		for (size_t i = 1; i <= MAX_LEVEL; i++)
		{
			iMaxObtainableExp += g_pMain->GetExpByLevel(i);
		}

		int64_t iMaxPossibleExp = iMaxObtainableExp - iUserTotalExp;

		if (iMaxPossibleExp <= 0)
		{
			SendOutputMsg("User has maximum possible exp", false);
			return;
		}

		if (iAmount > iMaxPossibleExp)
			iAmount = iMaxPossibleExp;

		int64_t iGivenExp = iAmount;
		int64_t iRemainingExp = 0;
		uint8_t iJumpedLevel = 0;


		if (iAmount >= g_pMain->GetExpByLevel(pUser->GetLevel()) - pUser->m_iExp)
		{ //given exp bigger than level, calculate how many level we jumped
			for (size_t i = pUser->GetLevel(); i <= MAX_LEVEL; i++)
			{
				if (pUser->GetLevel() == i) //at first consider m_iExp
				{

					if (i == MAX_LEVEL)
					{
						if (iGivenExp > g_pMain->GetExpByLevel(pUser->GetLevel()) - pUser->m_iExp)
							iRemainingExp = g_pMain->GetExpByLevel(pUser->GetLevel());
						else
							iRemainingExp = iGivenExp;
					}
					else
					{
						iGivenExp -= (g_pMain->GetExpByLevel(pUser->GetLevel()) - pUser->m_iExp);
					}

				}
				else
				{
					iGivenExp -= g_pMain->GetExpByLevel(i);
				}

				if (iGivenExp < 0)
				{
					iRemainingExp = g_pMain->GetExpByLevel(i) + iGivenExp;
					break;
				}

				iJumpedLevel++;
			}

			for (size_t i = 1; i <= iJumpedLevel; i++)
			{
				if (pUser->m_bLevel == MAX_LEVEL)
					break;

				pUser->m_bLevel++;
				pUser->LevelChange(pUser->m_bLevel, true);
			}

			pUser->m_iExp = iRemainingExp;

		}
		else //exp smaller than current level' exp requirement so user won't levelup
		{
			pUser->m_iExp += iGivenExp;
		}





	}

	Packet result(WIZ_EXP_CHANGE);
	result << uint32_t(pUser->m_iExp);
	pUser->Send(&result);


	//pUser->GiveExp(iAmount);

	if (sShowNotice == "1")
	{

		if (iAmount > 0)
			sNoticeMessage = string_format("Player: %s has been awarded with %d experience.", sUserID.c_str(), iAmount);
		else
			sNoticeMessage = string_format("%d Experience has been removed from player %s.", abs(iAmount), sUserID.c_str());

		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}


	std::string sOperatorCommandType = "OPERATOR_GIVE_EXP";
	AdminLogCommand(sOperatorCommandType, sUserID);

	std::string sOutputMessage;

	if (iAmount > 0)
		sOutputMessage = string_format("%d Experience has been given to user.", iAmount);
	else
		sOutputMessage = string_format("%d Experience has been removed from user.", abs(iAmount));

	SendOutputMsg(sOutputMessage.c_str());
}

//cmd : arrestuser Username, (this basically teleports to the user)
void CUser::AdminArrestUserHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID;

	bool bIsOnline = false;

	pkt >> sUserID;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;

	if (bIsOnline)
	{
		ZoneChange(pUser->GetZoneID(), pUser->m_curx, pUser->m_curz);
		SendOutputMsg("teleported to user by success.");
	}
	else
	{
		SendOutputMsg("User offline.", false);
		return;
	}

	std::string sOperatorCommandType = "OPERATOR_ARREST";
	AdminLogCommand(sOperatorCommandType, sUserID);
}

//cmd : summonuser UserName
void CUser::AdminSummonUserHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID;

	bool bIsOnline = false;

	pkt >> sUserID;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;

	if (bIsOnline)
	{
		pUser->ZoneChange(GetZoneID(), m_curx, m_curz);
		SendOutputMsg("User has been summoned successfully.");
	}
	else
	{
		SendOutputMsg("User cannot be summoned because not online.", false);
		return;
	}

	std::string sOperatorCommandType = "OPERATOR_SUMMON";
	AdminLogCommand(sOperatorCommandType, sUserID);

}

//cmd : cutoff Username
void CUser::AdminCutoffUserHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID;


	bool bIsOnline = false;

	pkt >> sUserID;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;



	if (bIsOnline)
	{

		if (pUser->m_bAuthority == AUTHORITY_GAME_MASTER)
		{
			SendOutputMsg("Game master cannot be disconnected.", false);
			return;
		}

		SendOutputMsg("User has been disconnected successfully.");
		pUser->Disconnect();
	}
	else
	{
		SendOutputMsg("Disconnecting user failed because user is not in game.", false);
		return;
	}

	std::string sOperatorCommandType = "OPERATOR_CUTOFF";
	AdminLogCommand(sOperatorCommandType, sUserID);

}

//cmd : forbidconnection Username [optional 1 or 0 for notice]
void CUser::AdminBanUserHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sShowNotice, sNoticeMessage;


	bool bIsOnline = false;

	pkt >> sUserID >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;



	if (bIsOnline)
	{

		if (pUser->m_bAuthority == AUTHORITY_GAME_MASTER)
		{
			SendOutputMsg("Game master authority cannot be changed.", false);
			return;
		}

		pUser->m_bAuthority = AUTHORITY_BANNED;
		pUser->Disconnect();
		SendOutputMsg("User has been banned successfully.");
	}
	else
	{
		g_DBAgent.UpdateUserAuthority(sUserID, AUTHORITY_BANNED);
		SendOutputMsg("User has been banned successfully [was offline].", false);
	}

	if (sShowNotice == "1")
	{
		sNoticeMessage = string_format("%s is currently blocked for illegal activity.", sUserID.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}

	std::string sOperatorCommandType = "OPERATOR_BAN";
	AdminLogCommand(sOperatorCommandType, sUserID);


}

//cmd : permitconnection Username [optional 1 or 0 for notice]
void CUser::AdminUnbanUserHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sShowNotice, sNoticeMessage;

	pkt >> sUserID >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	//BUG:this should not change game master authority

	g_DBAgent.UpdateUserAuthority(sUserID, AUTHORITY_PLAYER);
	SendOutputMsg("Ban has been removed successfully.");

	if (sShowNotice == "1")
	{
		sNoticeMessage = string_format("Ban of %s has been removed.", sUserID.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}

	std::string sOperatorCommandType = "OPERATOR_UNBAN";
	AdminLogCommand(sOperatorCommandType, sUserID);

}

//cmd : forbidchat Username [optional 1 or 0 for notice]
void CUser::AdminMuteUserHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sShowNotice, sNoticeMessage;


	bool bIsOnline = false;

	pkt >> sUserID >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;



	if (bIsOnline)
	{
		if (pUser->m_bAuthority == AUTHORITY_GAME_MASTER)
		{
			SendOutputMsg("Game master authority cannot be changed.", false);
			return;
		}

		pUser->m_bAuthority = AUTHORITY_MUTED;
		SendOutputMsg("User has been muted successfully [was online].");
	}
	else
	{
		g_DBAgent.UpdateUserAuthority(sUserID, AUTHORITY_MUTED);
		SendOutputMsg("User has been muted successfully [was offline].");
	}


	if (sShowNotice == "1")
	{
		sNoticeMessage = string_format("%s is currently muted.", sUserID.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}

	std::string sOperatorCommandType = "OPERATOR_MUTE";
	AdminLogCommand(sOperatorCommandType, sUserID);

}

//cmd : permitchat Username [optional 1 or 0 for notice]
void CUser::AdminUnMuteUserHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sShowNotice, sNoticeMessage;


	bool bIsOnline = false;

	pkt >> sUserID >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;



	if (bIsOnline)
	{
		if (pUser->m_bAuthority == AUTHORITY_GAME_MASTER)
		{
			SendOutputMsg("Game master authority cannot be changed.", false);
			return;
		}

		pUser->m_bAuthority = AUTHORITY_PLAYER;
		SendOutputMsg("User has been unmuted successfully [was online].");
	}
	else
	{
		g_DBAgent.UpdateUserAuthority(sUserID, AUTHORITY_PLAYER);
		SendOutputMsg("User has been unmuted successfully [was offline].");
	}


	if (sShowNotice == "1")
	{
		sNoticeMessage = string_format("Mute of %s has been removed.", sUserID.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}

	std::string sOperatorCommandType = "OPERATOR_UNMUTE";
	AdminLogCommand(sOperatorCommandType, sUserID);

}


//NOTE this is not working, not controlled in Attackhandler.cpp
//cmd : disableattack UserName [optional 1 or 0 for notice]
void CUser::AdminDisableAttackHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sShowNotice, sNoticeMessage;
	bool bIsOnline = false;

	pkt >> sUserID >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;

	if (bIsOnline)
	{

		if (pUser->m_bAuthority == AUTHORITY_GAME_MASTER)
		{
			SendOutputMsg("Game master authority cannot be changed.", false);
			return;
		}

		pUser->m_bAuthority = AUTHORITY_ATTACK_DISABLED;
		SendOutputMsg("Attack has been disabled successfully [was online].");
	}
	else
	{
		//TODO: return if target user is GM
		g_DBAgent.UpdateUserAuthority(sUserID, AUTHORITY_ATTACK_DISABLED);
		SendOutputMsg("Attack has been disabled successfully [was offline].");
	}

	if (sShowNotice == "1")
	{
		sNoticeMessage = string_format("%s is currently attack disabled.", sUserID.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}

	std::string sOperatorCommandType = "OPERATOR_DISABLE_ATTACK";
	AdminLogCommand(sOperatorCommandType, sUserID);
}

//this is written wrong in Texts_us.tbl so command /attakcenable
//cmd : attakcenable UserName [optional 1 or 0 for notice]
void CUser::AdminEnableAttackHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, showNotice, sNoticeMessage;

	bool bIsOnline = false;

	pkt >> sUserID >> showNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;




	if (bIsOnline)
	{

		if (pUser->m_bAuthority == AUTHORITY_GAME_MASTER)
		{
			SendOutputMsg("Game master authority cannot be changed.", false);
			return;
		}

		if (pUser->isAttackDisabled())
		{
			pUser->m_bAuthority = AUTHORITY_PLAYER;
			SendOutputMsg("Attack has been enabled successfully [user is online].");
		}
		else
		{
			SendOutputMsg("User was already able to attack, wrong user.", false);
		}


	}
	else
	{
		//TODO: return if user is GM before update
		g_DBAgent.UpdateUserAuthority(sUserID, AUTHORITY_PLAYER);
		SendOutputMsg("Attack has been enabled successfully [was offline].");
	}


	if (showNotice == "1")
	{
		sNoticeMessage = string_format("%s is currently attack enabled.", sUserID.c_str());
		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);
	}

	std::string sOperatorCommandType = "OPERATOR_ATTACK_ENABLE";
	AdminLogCommand(sOperatorCommandType, sUserID);
}

//cmd : zonechange UserName zoneID
//bug: written non existing zone returns success message but dont change zone
//bug: zoneID 91 not working
void CUser::AdminZoneChangeHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sZoneID;
	uint8_t iZoneID;

	bool bIsOnline = false;

	pkt >> sUserID >> sZoneID;

	TRACE("sZoneID\n %s", sZoneID);

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	if (sZoneID.empty())
	{
		SendOutputMsg("Zone ID is empty", false);
		return;
	}

	try
	{
		iZoneID = std::stoi(sZoneID);
	}
	catch (...)
	{
		SendOutputMsg("Zone ID is in wrong format, entry includes letters.", false);
		return;
	}

	TRACE("sZoneID\n %d", iZoneID);


	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;

	TRACE("bIsOnline\n %d", bIsOnline);


	if (bIsOnline)
	{
		if (pUser->isGM() && sUserID != GetName()) //A gm cannot change other gm's zone
		{
			SendOutputMsg("You cannot change zone of other game masters.", false);
			return;
		}

		ZoneChange(iZoneID, 0, 0);
		SendOutputMsg("User zone has been changed successfully.");
	}
	else
	{
		//TODO: db query to update offline user
		SendOutputMsg("User is offline, zone change failed.", false);
		return;
	}

	std::string sOperatorCommandType = "OPERATOR_ZONE_CHANGE";
	AdminLogCommand(sOperatorCommandType, sUserID);
}

//cmd : give_coin UserName Amount [optional 1 or 0 for notice]
void CUser::AdminGiveCoinHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sAmount, sShowNotice;
	std::string sNoticeMessage;

	bool bIsOnline = false;

	pkt >> sUserID >> sAmount >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	if (sAmount.empty())
	{
		SendOutputMsg("Amount of coins did not included", false);
		return;
	}

	int iAmount;
	try
	{
		iAmount = std::stoi(sAmount);
	}
	catch (...)
	{
		SendOutputMsg("Amount of coins are in wrong format.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;

	if (bIsOnline)
	{
		uint8_t opcodeReturn;

		(iAmount >= 0) ? opcodeReturn = CoinGain : opcodeReturn = CoinLoss;

		int64_t tempGold = static_cast<int64_t>(pUser->m_iGold) + static_cast<int64_t>(iAmount);

		if (tempGold < 0)
			pUser->m_iGold = 0;
		else if (tempGold > INT32_MAX)
			pUser->m_iGold = INT32_MAX;
		else
			pUser->m_iGold = static_cast<int32_t>(tempGold);

		SendOutputMsg("Coins have been added to user with success.");

		Packet result(WIZ_GOLD_CHANGE);
		result << uint8_t(opcodeReturn) << iAmount << pUser->m_iGold;
		pUser->Send(&result);

	}
	else
	{
		//TODO: db query to update offline user
		SendOutputMsg("User is offline.", false);
		return;
	}

	if (sShowNotice == "1")
	{

		if (iAmount >= 0)
			sNoticeMessage = string_format("%s is awarded with %d coins.", sUserID.c_str(), iAmount);
		else
			sNoticeMessage = string_format("%d coins have been removed from %s.", iAmount, sUserID.c_str());

		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);

	}

	std::string sOperatorCommandType = "OPERATOR_GIVE_COIN";
	AdminLogCommand(sOperatorCommandType, sUserID);
}

//cmd : give_coin UserName Amount [optional 1 or 0 for notice]
void CUser::AdminGiveNationalPointHandler(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	std::string sUserID, sAmount, sShowNotice;
	std::string sNoticeMessage;

	bool bIsOnline = false;

	pkt >> sUserID >> sAmount >> sShowNotice;

	if (sUserID.empty() || sUserID.size() > MAX_ID_SIZE)
	{
		SendOutputMsg("Invalid username.", false);
		return;
	}

	if (sAmount.empty())
	{
		SendOutputMsg("Amount of national points did not included", false);
		return;
	}

	int iAmount;
	try
	{
		iAmount = std::stoi(sAmount);
	}
	catch (...)
	{
		SendOutputMsg("Amount of national points is in wrong format.", false);
		return;
	}

	CUser* pUser = g_pMain->GetUserPtr(sUserID, TYPE_CHARACTER);

	(pUser == nullptr) ? bIsOnline = false : bIsOnline = true;

	if (bIsOnline)
	{
		uint8_t opcodeReturn;

		(iAmount >= 0) ? opcodeReturn = CoinGain : opcodeReturn = CoinLoss;

		int64_t tempLoyalty = static_cast<int64_t>(pUser->m_iLoyalty) + static_cast<int64_t>(iAmount);

		if (tempLoyalty < 0)
		{
			pUser->m_iLoyalty = 0;
		}
		else if (tempLoyalty > LOYALTY_MAX)
		{
			pUser->m_iLoyalty = LOYALTY_MAX;
		}
		else
		{
			pUser->m_iLoyalty = static_cast<int32_t>(tempLoyalty);
		}

		pUser->m_iLoyaltyMonthly = pUser->m_iLoyalty;

		SendOutputMsg("National points have been added to user with success.");

		Packet result(WIZ_LOYALTY_CHANGE);
		result << uint8_t(LOYALTY_NATIONAL_POINTS) << pUser->m_iLoyalty << pUser->m_iLoyaltyMonthly;
		pUser->Send(&result);

	}
	else
	{
		//TODO: db query to update offline user
		SendOutputMsg("User is offline.", false);
		return;
	}

	if (sShowNotice == "1")
	{

		if (iAmount >= 0)
		{
			sNoticeMessage = string_format("%s is awarded with %d national points.", sUserID.c_str(), iAmount);
		}
		else
		{
			sNoticeMessage = string_format("%d national points have been removed from %s.", iAmount, sUserID.c_str());
		}

		g_pMain->SendNotice(sNoticeMessage.c_str(), Nation::ALL);

	}

	std::string sOperatorCommandType = "OPERATOR_GIVE_NP";
	AdminLogCommand(sOperatorCommandType, sUserID);

}

// cmd: visible
void CUser::AdminMakeVisible(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	m_bAbnormalType = ABNORMAL_NORMAL;

	SendOutputMsg("State changed as visible.");

	Packet inResult(WIZ_USER_INOUT);
	GetInOut(inResult, INOUT_IN);
	SendToRegion(&inResult, this);

	Packet result(WIZ_STATE_CHANGE);
	result << GetID() << uint8_t(5) << uint32_t(ABNORMAL_NORMAL);
	Send(&result);

	std::string sOperatorCommandType = "OPERATOR_VISIBLE";
	AdminLogCommand(sOperatorCommandType, GetName());
}

// cmd: invisible
void CUser::AdminMakeInvisible(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	m_bAbnormalType = ABNORMAL_INVISIBLE;

	SendOutputMsg("State changed as invisible.");

	Packet inResult(WIZ_USER_INOUT);
	GetInOut(inResult, INOUT_OUT); // type out - exit
	SendToRegion(&inResult, this);

	Packet result(WIZ_STATE_CHANGE);
	result << GetID() << uint8_t(5) << uint32_t(ABNORMAL_INVISIBLE);
	Send(&result);

	std::string sOperatorCommandType = "OPERATOR_INVISIBLE";
	AdminLogCommand(sOperatorCommandType, GetName());
}

// cmd: gm_help
void CUser::AdminHelp(Packet& pkt)
{
	if (!isGM())
	{
		AdminLogBadAccess();
		return;
	}

	SendOutputMsg("GM Command List:");
	SendOutputMsg("/give_item UserName ItemID [1:0 Notice]");
	SendOutputMsg("/give_countable_item UserName ItemID Amount [1:0 Notice, optional]");
	SendOutputMsg("/give_exp UserName Amount|-Amount [1:0 Notice, optional]");
	SendOutputMsg("/give_coin UserName Amount|-Amount [1:0 Notice, optional]");
	SendOutputMsg("/give_np UserName Amount|-Amount [1:0 Notice, optional]");
	SendOutputMsg("/arrest UserName");
	SendOutputMsg("/summonuser UserName");
	SendOutputMsg("/cutoff UserName");
	SendOutputMsg("/forbidconnect UserName [1:0 Notice, optional]");
	SendOutputMsg("/permitconnect UserName [1:0 Notice, optional]");
	SendOutputMsg("/forbidchat UserName [1:0 Notice, optional]");
	SendOutputMsg("/permitchat UserName [1:0 Notice, optional]");
	SendOutputMsg("/disableattack UserName [1:0 Notice, optional]");
	SendOutputMsg("/enableattakc UserName [1:0 Notice, optional]");
	SendOutputMsg("/zonechange UserName ZoneID");
	SendOutputMsg("/visible");
	SendOutputMsg("/invisible");


	std::string sOperatorCommandType = "OPERATOR_HELP";
	AdminLogCommand(sOperatorCommandType, GetName());
}
