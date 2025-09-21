#pragma once

#include <vector>
#include <string>
#include <string_view>

class CEbenezerDlg;
class CUser;
class OperationMessage
{
public:
	OperationMessage(CEbenezerDlg* main, CUser* srcUser);
	bool Process(const std::string_view command);

protected:
	void Pursue();
	void ActPursue();
	void MonPursue();
	void MonCatch();
	void Assault();
	void MonSummon();
	void MonSummonAll();
	void UnikMonster();
	void UserSeekReport();
	void MonKill();
	void Open();
	void Open2();
	void Open3();
	void MOpen();
	void ForbidUser();
	void ForbidConnect();
	void SnowOpen();
	void Close();
	void Captain();
	void TieBreak();
	void Auto();
	void AutoOff();
	void Down();
	void Discount();
	void FreeDiscount();
	void AllDiscount();
	void UnDiscount();
	void Santa();
	void Angel();
	void OffSanta();
	void LimitBattle();
	void OnSummonBlock();
	void OffSummonBlock();
	void ZoneChange();
	void SiegeWarfare();
	void ResetSiegeWar();
	void SiegeWarScheduleStart();
	void SiegeWarScheduleEnd();
	void SiegeWarBaseReport();
	void SiegeWarStatusReport();
	void SiegeWarCheckBase();
	void ServerTestMode();
	void ServerNormalMode();
	void MerchantMoney();
	void SiegeWarPunishKnights();
	void SiegeWarLoadTable();
	void MoneyAdd();
	void ExpAdd();
	void UserBonus();
	void Discount1();
	void Discount2();
	void Battle1();
	void Battle2();
	void Battle3();
	void BattleAuto();
	void BattleReport();
	void ChallengeOn();
	void ChallengeOff();
	void ChallengeKill();
	void ChallengeLevel();
	void RentalReport();
	void RentalStop();
	void RentalStart();
	void KingReport1();
	void KingReport2();
	void ReloadKing();
	void Kill();
	void ReloadNotice();
	void ReloadHacktool();
	void ServerDown();
	void WriteLog();
	void EventLog();
	void EventLogOff();
	void ItemDown();
	void ItemDownReset();
	void ChallengeStop();

	bool ParseCommand(const std::string_view command, size_t& key);

	// Returns the number of arguments, excluding the command name.
	size_t GetArgCount() const;
	int ParseInt(size_t argIndex) const;
	float ParseFloat(size_t argIndex) const;
	const std::string& ParseString(size_t argIndex) const;

protected:
	CEbenezerDlg* _main;
	CUser* _srcUser;
	std::string _command;
	std::vector<std::string> _args;
};
