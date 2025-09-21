#include "StdAfx.h"
#include "OperationMessage.h"
#include "EbenezerDlg.h"
#include "User.h"

#include <djb2/djb2_hasher.h>
#include <shared/StringUtils.h>
#include <spdlog/spdlog.h>

#include <sstream>
#include <stdexcept>

extern BYTE g_serverdown_flag;

OperationMessage::OperationMessage(CEbenezerDlg* main, CUser* srcUser)
	: _main(main), _srcUser(srcUser)
{
}

bool OperationMessage::Process(const std::string_view command)
{
	size_t key = 0;
	if (!ParseCommand(command, key))
		return false;

	try
	{
		switch (key)
		{
#if 0 // TODO
			case "+pursue"_djb2:
				Pursue();
				break;

			case "+actpursue"_djb2:
				ActPursue();
				break;

			case "+monpursue"_djb2:
				MonPursue();
				break;

			case "+moncatch"_djb2:
				MonCatch();
				break;

			case "+assault"_djb2:
				Assault();
				break;

			case "+monsummon"_djb2:
				MonSummon();
				break;

			case "+monsummonall"_djb2:
				MonSummonAll();
				break;

			case "+unikmonster"_djb2:
				UnikMonster();
				break;

			case "+user_seek_report"_djb2:
				UserSeekReport();
				break;

			case "+monkill"_djb2:
				MonKill();
				break;
#endif

			case "/open"_djb2:
			case "+open"_djb2:
				Open();
				break;

#if 0 // TODO
			case "/open2"_djb2:
			case "+open2"_djb2:
				Open2();
				break;

			case "/open3"_djb2:
			case "+open3"_djb2:
				Open3();
				break;

			case "/mopen"_djb2:
			case "+mopen"_djb2:
				MOpen();
				break;

			case "+forbiduser"_djb2:
				ForbidUser();
				break;

			case "+forbidconnect"_djb2:
				ForbidConnect();
				break;
#endif

			case "/snowopen"_djb2:
			case "+snowopen"_djb2:
				SnowOpen();
				break;

			case "/close"_djb2:
			case "+close"_djb2:
				Close();
				break;

			case "/captain"_djb2:
			case "+captain"_djb2:
				Captain();
				break;

#if 0 // TODO
			case "/tiebreak"_djb2:
			case "+tiebreak"_djb2:
				TieBreak();
				break;

			case "/auto"_djb2:
			case "+auto"_djb2:
				Auto();
				break;

			case "/auto_off"_djb2:
			case "+auto_off"_djb2:
				AutoOff();
				break;
#endif

			case "/down"_djb2:
			case "+down"_djb2:
				Down();
				break;

			case "/discount"_djb2:
			case "+discount"_djb2:
				Discount();
				break;

#if 0 // TODO
			case "/freediscount"_djb2:
			case "+freediscount"_djb2:
				FreeDiscount();
				break;
#endif

			case "/alldiscount"_djb2:
			case "+alldiscount"_djb2:
				AllDiscount();
				break;

			case "/undiscount"_djb2:
			case "+undiscount"_djb2:
				UnDiscount();
				break;

			case "/santa"_djb2:
			case "+santa"_djb2:
				Santa();
				break;

			case "/angel"_djb2:
			case "+angel"_djb2:
				Angel();
				break;

			case "/offsanta"_djb2:
			case "+offsanta"_djb2:
				OffSanta();
				break;

#if 0 // TODO
			case "/limitbattle"_djb2:
			case "+limitbattle"_djb2:
				LimitBattle();
				break;

			case "/onsummonblock"_djb2:
			case "+onsummonblock"_djb2:
				OnSummonBlock();
				break;

			case "/offsummonblock"_djb2:
			case "+offsummonblock"_djb2:
				OffSummonBlock();
				break;
#endif

			// +zonechange: {int: zoneId} [float: x] [float: z]
			// NOTE: Coordinates are unofficial.
			case "+zonechange"_djb2:
				ZoneChange();
				break;

#if 0 // TODO
			case "+siegewarfare"_djb2:
				SiegeWarfare();
				break;

			case "+resetsiegewar"_djb2:
				ResetSiegeWar();
				break;

			case "+siegewarschedule-start"_djb2:
				SiegeWarScheduleStart();
				break;

			case "+siegewarschedule-end"_djb2:
				SiegeWarScheduleEnd();
				break;

			case "+siegewar_base_report"_djb2:
				SiegeWarBaseReport();
				break;

			case "+siegewar_status_report"_djb2:
				SiegeWarStatusReport();
				break;

			case "+siegewar_check_base"_djb2:
				SiegeWarCheckBase();
				break;

			case "/server_testmode"_djb2:
			case "+server_testmode"_djb2:
				ServerTestMode();
				break;

			case "/server_normalmode"_djb2:
			case "+server_normalmode"_djb2:
				ServerNormalMode();
				break;

			case "+merchant_money"_djb2:
				MerchantMoney();
				break;

			case "+siegewar_punish_knights"_djb2:
				SiegeWarPunishKnights();
				break;

			case "+siegewar_load_table"_djb2:
				SiegeWarLoadTable();
				break;

			case "/money_add"_djb2:
			case "+money_add"_djb2:
				MoneyAdd();
				break;

			case "/exp_add"_djb2:
			case "+exp_add"_djb2:
				ExpAdd();
				break;

			case "+user_bonus"_djb2:
				UserBonus();
				break;

			case "/discount1"_djb2:
			case "+discount1"_djb2:
				Discount1();
				break;

			case "/discount2"_djb2:
			case "+discount2"_djb2:
				Discount2();
				break;

			case "/battle1"_djb2:
			case "+battle1"_djb2:
				Battle1();
				break;

			case "/battle2"_djb2:
			case "+battle2"_djb2:
				Battle2();
				break;

			case "/battle3"_djb2:
			case "+battle3"_djb2:
				Battle3();
				break;

			case "/battle_auto"_djb2:
			case "+battle_auto"_djb2:
				BattleAuto();
				break;

			case "+battle_report"_djb2:
				BattleReport();
				break;

			case "/challenge_on"_djb2:
			case "+challenge_on"_djb2:
				ChallengeOn();
				break;

			case "/challenge_off"_djb2:
			case "+challenge_off"_djb2:
				ChallengeOff();
				break;

			case "/challenge_kill"_djb2:
			case "+challenge_kill"_djb2:
				ChallengeKill();
				break;

			case "/challenge_level"_djb2:
			case "+challenge_level"_djb2:
				ChallengeLevel();
				break;

			case "/rental_report"_djb2:
			case "+rental_report"_djb2:
				RentalReport();
				break;

			case "/rental_stop"_djb2:
			case "+rental_stop"_djb2:
				RentalStop();
				break;

			case "/rental_start"_djb2:
			case "+rental_start"_djb2:
				RentalStart();
				break;

			case "/king_report1"_djb2:
			case "+king_report1"_djb2:
				KingReport1();
				break;

			case "/king_report2"_djb2:
			case "+king_report2"_djb2:
				KingReport2();
				break;

			case "/reload_king"_djb2:
			case "+reload_king"_djb2:
				ReloadKing();
				break;
#endif

			case "/kill"_djb2:
				Kill();
				break;

#if 0
			case "/reload_notice"_djb2:
				ReloadNotice();
				break;

			case "/reload_hacktool"_djb2:
				ReloadHacktool();
				break;

			case "/serverdown"_djb2:
				ServerDown();
				break;

			case "/writelog"_djb2:
				WriteLog();
				break;

			case "/eventlog"_djb2:
				EventLog();
				break;

			case "/eventlog_off"_djb2:
				EventLogOff();
				break;

			case "/itemdown"_djb2:
				ItemDown();
				break;

			case "/itemdownreset"_djb2:
				ItemDownReset();
				break;

			case "/challengestop"_djb2:
				ChallengeStop();
				break;
#endif

			// Unhandled command.
			default:
				return false;

		}
	}
	catch (const std::invalid_argument& ex)
	{
		if (_srcUser != nullptr)
		{
			spdlog::warn(
				"OperationMessage::Process: argument could not be parsed from GM [charId={} command='{}' exception='{}']",
				_srcUser->m_pUserData->m_id, _command, ex.what());
		}
		else
		{
			spdlog::warn(
				"OperationMessage::Process: argument could not be parsed from server [command='{}' exception='{}']",
				_command, ex.what());
		}
	}
	catch (const std::out_of_range& ex)
	{
		if (_srcUser != nullptr)
		{
			spdlog::warn(
				"OperationMessage::Process: parsed argument out of range from GM [charId={} command='{}' exception='{}']",
				_srcUser->m_pUserData->m_id, _command, ex.what());
		}
		else
		{
			spdlog::warn(
				"OperationMessage::Process: parsed argument out of range from server [command='{}' exception='{}']",
				_command, ex.what());
		}
	}

	// Command was handled, even if it errored.
	return true;
}

void OperationMessage::Pursue()
{
	// TODO
}

void OperationMessage::ActPursue()
{
	// TODO
}

void OperationMessage::MonPursue()
{
	// TODO
}

void OperationMessage::MonCatch()
{
	// TODO
}

void OperationMessage::Assault()
{
	// TODO
}

void OperationMessage::MonSummon()
{
	// TODO
}

void OperationMessage::MonSummonAll()
{
	// TODO
}

void OperationMessage::UnikMonster()
{
	// TODO
}

void OperationMessage::UserSeekReport()
{
	// TODO
}

void OperationMessage::MonKill()
{
	// TODO
}

void OperationMessage::Open()
{
	_main->BattleZoneOpen(BATTLEZONE_OPEN);
}

void OperationMessage::Open2()
{
	// TODO
}

void OperationMessage::Open3()
{
	// TODO
}

void OperationMessage::MOpen()
{
	// TODO
}

void OperationMessage::ForbidUser()
{
	// TODO
}

void OperationMessage::ForbidConnect()
{
	// TODO
}

void OperationMessage::SnowOpen()
{
	_main->BattleZoneOpen(SNOW_BATTLEZONE_OPEN);
}

void OperationMessage::Close()
{
	_main->m_byBanishFlag = 1;
	// _main->WithdrawUserOut();
}

void OperationMessage::Captain()
{
	_main->LoadKnightsRankTable();
}

void OperationMessage::TieBreak()
{
	// TODO
}

void OperationMessage::Auto()
{
	// TODO
}

void OperationMessage::AutoOff()
{
	// TODO
}

void OperationMessage::Down()
{
	g_serverdown_flag = TRUE;
	SuspendThread(_main->m_Iocport.m_hAcceptThread);
	_main->KickOutAllUsers();
}

void OperationMessage::Discount()
{
	_main->m_sDiscount = 1;
}

void OperationMessage::FreeDiscount()
{
	// TODO
}

void OperationMessage::AllDiscount()
{
	_main->m_sDiscount = 2;
}

void OperationMessage::UnDiscount()
{
	_main->m_sDiscount = 0;
}

void OperationMessage::Santa()
{
	_main->m_bySanta = 1;		// Make Motherfucking Santa Claus FLY!!!
}

void OperationMessage::Angel()
{
	_main->m_bySanta = 2;
}

void OperationMessage::OffSanta()
{
	_main->m_bySanta = 0;		// SHOOT DOWN Motherfucking Santa Claus!!!
}

void OperationMessage::LimitBattle()
{
	// TODO
}

void OperationMessage::OnSummonBlock()
{
	// TODO
}

void OperationMessage::OffSummonBlock()
{
	// TODO
}

// +zonechange: {int: zoneId} [float: x] [float: z]
// NOTE: Coordinates are unofficial.
void OperationMessage::ZoneChange()
{
	// Requires a user.
	if (_srcUser == nullptr
		|| GetArgCount() < 1)
		return;

	int zoneId = ParseInt(0);
	float x = _srcUser->m_pUserData->m_curx;
	float z = _srcUser->m_pUserData->m_curz;

	if (GetArgCount() >= 3)
	{
		x = ParseFloat(1);
		z = ParseFloat(2);
	}

	_srcUser->ZoneChange(zoneId, x, z);
}

void OperationMessage::SiegeWarfare()
{
	// TODO
}

void OperationMessage::ResetSiegeWar()
{
	// TODO
}

void OperationMessage::SiegeWarScheduleStart()
{
	// TODO
}

void OperationMessage::SiegeWarScheduleEnd()
{
	// TODO
}

void OperationMessage::SiegeWarBaseReport()
{
	// TODO
}

void OperationMessage::SiegeWarStatusReport()
{
	// TODO
}

void OperationMessage::SiegeWarCheckBase()
{
	// TODO
}

void OperationMessage::ServerTestMode()
{
	// TODO
}

void OperationMessage::ServerNormalMode()
{
	// TODO
}

void OperationMessage::MerchantMoney()
{
	// TODO
}

void OperationMessage::SiegeWarPunishKnights()
{
	// TODO
}

void OperationMessage::SiegeWarLoadTable()
{
	// TODO
}

void OperationMessage::MoneyAdd()
{
	// TODO
}

void OperationMessage::ExpAdd()
{
	// TODO
}

void OperationMessage::UserBonus()
{
	// TODO
}

void OperationMessage::Discount1()
{
	// TODO
}

void OperationMessage::Discount2()
{
	// TODO
}

void OperationMessage::Battle1()
{
	// TODO
}

void OperationMessage::Battle2()
{
	// TODO
}

void OperationMessage::Battle3()
{
	// TODO
}

void OperationMessage::BattleAuto()
{
	// TODO
}

void OperationMessage::BattleReport()
{
	// TODO
}

void OperationMessage::ChallengeOn()
{
	// TODO
}

void OperationMessage::ChallengeOff()
{
	// TODO
}

void OperationMessage::ChallengeKill()
{
	// TODO
}

void OperationMessage::ChallengeLevel()
{
	// TODO
}

void OperationMessage::RentalReport()
{
	// TODO
}

void OperationMessage::RentalStop()
{
	// TODO
}

void OperationMessage::RentalStart()
{
	// TODO
}

void OperationMessage::KingReport1()
{
	// TODO
}

void OperationMessage::KingReport2()
{
	// TODO
}

void OperationMessage::ReloadKing()
{
	// TODO
}

void OperationMessage::Kill()
{
	if (GetArgCount() < 1)
		return;

	const std::string& charId = ParseString(0);
	_main->KillUser(charId.c_str());
}

void OperationMessage::ReloadNotice()
{
	// TODO
}

void OperationMessage::ReloadHacktool()
{
	// TODO
}

void OperationMessage::ServerDown()
{
	// TODO
}

void OperationMessage::WriteLog()
{
	// TODO
}

void OperationMessage::EventLog()
{
	// TODO
}

void OperationMessage::EventLogOff()
{
	// TODO
}

void OperationMessage::ItemDown()
{
	// TODO
}

void OperationMessage::ItemDownReset()
{
	// TODO
}

void OperationMessage::ChallengeStop()
{
	// TODO
}

bool OperationMessage::ParseCommand(const std::string_view command, size_t& key)
{
	_command.assign(command.data(), command.length());
	_args.clear();

	// Split string into parts.
	// Delimit by whitespace.
	// Empty spaces are ignored.
	// This:
	// +cmd arg1    arg2     arg3
	// Will become:
	// [0] = +cmd, [1] = arg1, [2] = arg3
	std::istringstream ss(_command);
	std::string part;
	while (ss >> part)
		_args.push_back(part);

	// Expect at least one "argument" (the command name).
	if (_args.empty())
		return false;

	// Extract and transform the command name to lowercase.
	std::string& commandNameLowercase = _args.front();
	strtolower(commandNameLowercase);

	// Hash the lowercase key name for returning.
	key = hashing::djb2::hash(commandNameLowercase);

	// Strip it from the args list for consistency; we don't need it anymore.
	_args.erase(_args.begin());

	return true;
}

// Returns the number of arguments, excluding the command name.
size_t OperationMessage::GetArgCount() const
{
	return _args.size();
}

int OperationMessage::ParseInt(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return std::stoi(_args[argIndex]);
}

float OperationMessage::ParseFloat(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return std::stof(_args[argIndex]);
}

const std::string& OperationMessage::ParseString(size_t argIndex) const
{
	if (argIndex >= _args.size())
		throw std::invalid_argument(fmt::format("argument {} not supplied", argIndex));

	return _args[argIndex];
}
