#ifndef SERVER_EBENEZER_KNIGHTSMANAGER_H
#define SERVER_EBENEZER_KNIGHTSMANAGER_H

#pragma once

namespace Ebenezer
{

class CUser;
class EbenezerApp;
class CKnightsManager
{
public:
	void RecvKnightsAllList(const char* pBuf);
	void SetKnightsUser(int knightsId, const char* charId);
	bool ModifyKnightsUser(int knightsId, const char* charId);
	bool RemoveKnightsUser(int knightsId, const char* charId);
	bool AddKnightsUser(int knightsId, const char* charId);
	void RecvKnightsList(const char* pBuf);
	void RecvDestroyKnights(CUser* pUser, const char* pBuf);
	void RecvModifyFame(CUser* pUser, const char* pBuf, uint8_t command);
	void RecvJoinKnights(CUser* pUser, const char* pBuf, uint8_t command);
	void RecvCreateKnights(CUser* pUser, const char* pBuf);
	void ReceiveKnightsProcess(CUser* pUser, const char* pBuf, uint8_t command);
	void CurrentKnightsMember(CUser* pUser, char* pBuf);
	void AllKnightsMember(CUser* pUser);
	void AllKnightsList(CUser* pUser, char* pBuf);
	void ModifyKnightsMember(CUser* pUser, char* pBuf, uint8_t command);
	void DestroyKnights(CUser* pUser);
	void WithdrawKnights(CUser* pUser);
	void JoinKnights(CUser* pUser, char* pBuf);
	void JoinKnightsReq(CUser* pUser, char* pBuf);
	int GetKnightsIndex(int nation);
	bool IsAvailableName(const char* strname) const;
	void CreateKnights(CUser* pUser, char* pBuf);
	void PacketProcess(CUser* pUser, char* pBuf);

	/// \brief Updates a Knights' Flag field and sends updates
	void UpdateKnightsGrade(int16_t knightsId, uint8_t flag);

	/// \brief Attempts to warp online Knights members to the given destination.  This only works
	/// on online members in a zone less than 30 (Moradon, EMC/LFC, Eslant)
	void ZoneChange(int16_t knightsId, int zoneId, float x, float z);

	CKnightsManager();
	virtual ~CKnightsManager();

	EbenezerApp* m_pMain;
};

} // namespace Ebenezer

#endif // SERVER_EBENEZER_KNIGHTSMANAGER_H
