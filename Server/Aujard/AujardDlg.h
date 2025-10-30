#pragma once

#include "DBAgent.h"
#include "Define.h"
#include "Resource.h"

#include <shared-server/logger.h>
#include <shared-server/SharedMemoryBlock.h>
#include <shared-server/SharedMemoryQueue.h>
#include <shared-server/STLMap.h>

using ItemtableArray = CSTLMap<model::Item>;

namespace recordset_loader
{
	struct Error;
}

/////////////////////////////////////////////////////////////////////////////
// CAujardDlg dialog

class ReadQueueThread;
class TimerThread;
class CAujardDlg : public CDialog
{
// Construction
public:
	static inline CAujardDlg* GetInstance() {
		return _instance;
	}

	/// \brief Updates the IDC_DB_PROCESS text with the DB Process Number
	/// \note I'm not sure how practical this is under load
	void DBProcessNumber(int number);

	/// \brief handles DB_HEARTBEAT requests
	/// \see DB_HEARTBEAT
	void HeartbeatReceived();

	/// \brief checks the time since the last received heartbeat and saves if
	//  enough time has passed.
	void CheckHeartbeat();

	/// \brief handles DB_COUPON_EVENT requests
	/// \todo related stored procedures are not implemented
	/// \see DB_COUPON_EVENT
	void CouponEvent(const char* data);
	
	/// \brief handles WIZ_BATTLE_EVENT requests
    /// \details contains which nation won the war and which charId killed the commander
    /// \see WIZ_BATTLE_EVENT
	void BattleEventResult(const char* data);

	/// \brief writes a packet summary line to the log file
	void WritePacketLog();

	/// \brief handles WIZ_KICKOUT requests
	/// \see WIZ_KICKOUT
	void UserKickOut(const char* buffer);

	/// \brief handles WIZ_LOGIN_INFO requests, updating CURRENTUSER for a user
	/// \see WIZ_LOGIN_INFO
	void SetLogInInfo(const char* buffer);

	/// \brief attempts to retrieve metadata for a knights clan
	/// \see KnightsPacket(), KNIGHTS_LIST_REQ
	void KnightsList(const char* buffer);

	/// \brief Called every 5min by _concurrentCheckThread
	/// \see _concurrentCheckThread
	void ConCurrentUserCount();

	/// \brief attempts to return a list of all knights members
	/// \see KnightsPacket(), KNIGHTS_MEMBER_REQ
	void AllKnightsMember(const char* buffer);

	/// \brief attempts to disband a knights clan
	/// \see KnightsPacket(), KNIGHTS_DESTROY
	void DestroyKnights(const char* buffer);

	/// \brief attempts to modify a knights character
	/// \see KnightsPacket(), KNIGHTS_REMOVE, KNIGHTS_ADMIT, KNIGHTS_REJECT, KNIGHTS_CHIEF,
	/// KNIGHTS_VICECHIEF, KNIGHTS_OFFICER, KNIGHTS_PUNISH
	void ModifyKnightsMember(const char* buffer, uint8_t command);

	/// \brief attempt to remove a character from a knights clan
	/// \see KnightsPacket(), KNIGHTS_WITHDRAW
	void WithdrawKnights(const char* buffer);

	/// \brief attempts to add a character to a knights clan
	/// \see KnightsPacket(), KNIGHTS_JOIN
	void JoinKnights(const char* buffer);

	/// \brief attempts to create a knights clan
	/// \see KnightsPacket(), KNIGHTS_CREATE
	void CreateKnights(const char* buffer);

	/// \brief handles WIZ_KNIGHTS_PROCESS and WIZ_CLAN_PROCESS requests
	/// \detail calls the appropriate method for the subprocess op-code
	/// \see "Knights Packet sub define" section in Define.h
	void KnightsPacket(const char* buffer);

	/// \brief attempts to find a UserData record for charId
	/// \param charId
	/// \param[out] userId UserData index of the user, if found
	/// \return pointer to UserData[userId] object if found, nullptr otherwise
	_USER_DATA* GetUserPtr(const char* charId, int& userId);
	
	/// \brief handles a WIZ_ALLCHAR_INFO_REQ request
	/// \details Loads all character information and sends it to the client
	/// \see WIZ_ALLCHAR_INFO_REQ
	void AllCharInfoReq(const char* buffer);

	/// \brief handles a WIZ_LOGIN request to a selected game server
	/// \see WIZ_LOGIN
	void AccountLogIn(const char* buffer);

	/// \brief handles a WIZ_DEL_CHAR request
	/// \todo not implemented, always returns an error to the client
	/// \see WIZ_DEL_CHAR
	void DeleteChar(const char* buffer);

	/// \brief handles a WIZ_NEW_CHAR request
	/// \see WIZ_NEW_CHAR
	void CreateNewChar(const char* buffer);

	/// \brief handles a WIZ_SEL_NATION request to a selected game server
	/// \see WIZ_SEL_NATION
	void SelectNation(const char* buffer);

	/// \brief loads information needed from the ITEM table to a cache map
	bool LoadItemTable();
	
	/// \brief writes a recordset_loader::Error to an error pop-up
	void ReportTableLoadError(const recordset_loader::Error& err, const char* source);

	/// \brief handles a WIZ_DATASAVE request
	/// \see WIZ_DATASAVE
	/// \see HandleUserUpdate()
	void UserDataSave(const char* buffer);
	
	/// \brief Handles a WIZ_LOGOUT request when logging out of the game
	/// \details Updates USERDATA and WAREHOUSE as part of logging out, then resets the UserData entry for re-use
	/// \see WIZ_LOGOUT, HandleUserLogout()
	void UserLogOut(const char* buffer);
	
	/// \brief handling for when OnTimer fails a PROCESS_CHECK with ebenezer
	/// \details Logs ebenezer outage, attempts to save all UserData, and resets all UserData[userId] objects
	/// \see OnTimer(), HandleUserLogout()
	void AllSaveRoutine();

	/// \brief adds a message to the application's output box and updates scrollbar position
	/// \see _outputList
	void AddOutputMessage(const std::string& msg);

	/// \brief adds a message to the application's output box and updates scrollbar position
	/// \see _outputList
	void AddOutputMessage(const std::wstring& msg);

	CAujardDlg(CWnd* parent = nullptr);	// standard constructor
	~CAujardDlg();

	/// \brief initializes shared memory with other server applications
	bool InitSharedMemory();

	/// \brief loads and sends data after a character is selected
	void SelectCharacter(const char* buffer);

	SharedMemoryQueue	LoggerSendQueue;
	SharedMemoryQueue	LoggerRecvQueue;

	ItemtableArray		ItemArray;

	// Dialog Data
	//{{AFX_DATA(CAujardDlg)
	enum { IDD = IDD_AUJARD_DIALOG };
	CStatic	    DBProcessNum;
	//}}AFX_DATA

protected:
	static CAujardDlg*	_instance;

	CDBAgent			_dbAgent;
	SharedMemoryBlock	_userDataBlock;

	int					_serverId;
	int					_zoneId;

	int					_packetCount;		// packet의 수를 체크
	int					_sendPacketCount;	// packet의 수를 체크
	int					_recvPacketCount;	// packet의 수를 체크

	HICON				_icon;

	logger::Logger		_logger;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAujardDlg)

	/// \brief destroys all resources associated with the dialog window
	BOOL DestroyWindow() override;
	
	BOOL PreTranslateMessage(MSG* msg) override;

protected:
	/// \brief handles user logout functions
	/// \param userId user index for UserData
	/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE
	/// \param forceLogout should be set to true in panic situations
	/// \see UserLogOut(), AllSaveRoutine(), HandleUserUpdate()
	bool HandleUserLogout(int userId, uint8_t saveType, bool forceLogout = false);

	/// \brief handles user update functions and retry logic
	/// \param userId user index for UserData
	/// \param user reference to user object
	/// \param saveType one of: UPDATE_LOGOUT, UPDATE_ALL_SAVE, UPDATE_PACKET_SAVE
	/// \see UserDataSave(), HandleUserLogout()
	bool HandleUserUpdate(int userId, const _USER_DATA& user, uint8_t saveType);

	/// \brief performs MFC data exchange
	/// \see https://learn.microsoft.com/en-us/cpp/mfc/dialog-data-exchange?view=msvc-170
	void DoDataExchange(CDataExchange* data) override;	// DDX/DDV support

	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CAujardDlg)
	BOOL OnInitDialog() override;
	afx_msg void OnPaint();

	/// \brief The system calls this to obtain the cursor to display while the user drags
	///  the minimized window.
	afx_msg HCURSOR OnQueryDragIcon();

	/// \brief triggered when the Exit button is clicked. Will ask user to confirm intent to close the program.
	void OnOK() override;
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	/// \brief output message box for the application
	CListBox							_outputList;
										
	time_t								_heartbeatReceivedTime;
	std::unique_ptr<TimerThread>		_dbPoolCheckThread;
	std::unique_ptr<TimerThread>		_heartbeatCheckThread;
	std::unique_ptr<TimerThread>		_concurrentCheckThread;
	std::unique_ptr<TimerThread>		_packetCheckThread;
										
	std::unique_ptr<ReadQueueThread>	_readQueueThread;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
