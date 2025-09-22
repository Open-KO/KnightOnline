// LOGIC_ELSE.cpp: implementation of the LOGIC_ELSE class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ebenezer.h"
#include "Define.h"
#include "LOGIC_ELSE.h"

#include <djb2/djb2_hasher.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LOGIC_ELSE::LOGIC_ELSE()
{
	m_LogicElse = FALSE;
	m_bAnd = TRUE;
	for (int i = 0; i < MAX_LOGIC_ELSE_INT; i++)
		m_LogicElseInt[i] = -1;
}

LOGIC_ELSE::~LOGIC_ELSE()
{
}

void LOGIC_ELSE::Init()
{
	for (int i = 0; i < MAX_LOGIC_ELSE_INT; i++)
		m_LogicElseInt[i] = -1;

	m_bAnd = TRUE;
}

void LOGIC_ELSE::Parse_and(const char* line, const std::wstring& filename, int lineNumber)
{
	int index = 0, argsToParse = 0;
	char temp[1024];

	index += ParseSpace(temp, line + index);

	size_t opcode = hashing::djb2::hash(std::string_view(temp));
	switch (opcode)
	{
		// A CHECK_UNDER_WEIGHT
		case "CHECK_UNDER_WEIGHT"_djb2:
			m_LogicElse = LOGIC_CHECK_UNDER_WEIGHT;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_OVER_WEIGHT
		case "CHECK_OVER_WEIGHT"_djb2:
			m_LogicElse = LOGIC_CHECK_OVER_WEIGHT;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_SKILL_POINT {skill category} {minimum points} {maximum points}
		case "CHECK_SKILL_POINT"_djb2:
			m_LogicElse = LOGIC_CHECK_SKILL_POINT;
			argsToParse = 3;
			break;

		// A CHECK_EXIST_ITEM {item ID} {minimum item count}
		case "CHECK_EXIST_ITEM"_djb2:
			m_LogicElse = LOGIC_CHECK_EXIST_ITEM;
			argsToParse = 2;
			break;

		// A CHECK_NOEXIST_ITEM {item ID} {minimum item count}
		case "CHECK_NOEXIST_ITEM"_djb2:
			m_LogicElse = LOGIC_CHECK_NOEXIST_ITEM;
			argsToParse = 2;
			break;

		// A CHECK_CLASS {class 1} {class 2} {class 3} {class 4} {class 5} {class 6}
		case "CHECK_CLASS"_djb2:
			m_LogicElse = LOGIC_CHECK_CLASS;
			argsToParse = 6;
			break;

		// A CHECK_WEIGHT {item ID} {item count}
		case "CHECK_WEIGHT"_djb2:
			m_LogicElse = LOGIC_CHECK_WEIGHT;
			argsToParse = 2;
			break;

		// A CHECK_EDITBOX
		case "CHECK_EDITBOX"_djb2:
			m_LogicElse = LOGIC_CHECK_EDITBOX;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A RAND {percentage}
		case "RAND"_djb2:
			m_LogicElse = LOGIC_RAND;
			argsToParse = 1;
			break;

		// A CHECK_NOAH {minimum} {maximum}
		case "CHECK_NOAH"_djb2:
			m_LogicElse = LOGIC_CHECK_NOAH;
			argsToParse = 2;
			break;
		
		// A CHECK_LV {minimum} {maximum}
		case "CHECK_LV"_djb2:
			m_LogicElse = LOGIC_CHECK_LV;
			argsToParse = 2;
			break;

		// A HOWMUCH_ITEM {item ID} {minimum} {maximum}
		case "HOWMUCH_ITEM"_djb2:
			m_LogicElse = LOGIC_HOWMUCH_ITEM;
			argsToParse = 3;
			break;

		// A NOEXIST_COM_EVENT {com event ID}
		case "NOEXIST_COM_EVENT"_djb2:
			m_LogicElse = LOGIC_NOEXIST_COM_EVENT;
			argsToParse = 1;
			break;

		// A CHECK_NATION {nation}
		case "CHECK_NATION"_djb2:
			m_LogicElse = LOGIC_CHECK_NATION;
			argsToParse = 1;
			break;
			
		// A EXIST_COM_EVENT {com event ID}
		case "EXIST_COM_EVENT"_djb2:
			m_LogicElse = LOGIC_EXIST_COM_EVENT;
			argsToParse = 1;
			break;

#if 0 // TODO
		// A CHECK_PROMOTION_ELIGIBLE
		case "CHECK_PROMOTION_ELIGIBLE"_djb2:
			m_LogicElse = LOGIC_CHECK_PROMOTION_ELIGIBLE;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_MONSTER_CHALLENGE_TIME {Forgotten Temple type}
		case "CHECK_MONSTER_CHALLENGE_TIME"_djb2:
			m_LogicElse = LOGIC_CHECK_MONSTER_CHALLENGE_TIME;
			argsToParse = 1;
			break;

		// A CHECK_PPCARD_SERIAL
		case "CHECK_PPCARD_SERIAL"_djb2:
			m_LogicElse = LOGIC_CHECK_PPCARD_SERIAL;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_EXIST_EVENT {quest ID} {quest state}
		case "CHECK_EXIST_EVENT"_djb2:
			m_LogicElse = LOGIC_CHECK_EXIST_EVENT;
			argsToParse = 2;
			break;

		// A CHECK_ITEMCHANGE_NUM {last slot rewarded by exchange - 1..5 [nExchangeItemNum1..5]}
		case "CHECK_ITEMCHANGE_NUM"_djb2:
			m_LogicElse = LOGIC_CHECK_ITEMCHANGE_NUM;
			argsToParse = 1;
			break;

		// A CHECK_NOCLASS {class 1} {class 2} {class 3} {class 4} {class 5} {class 6}
		case "CHECK_NOCLASS"_djb2:
			m_LogicElse = LOGIC_CHECK_NOCLASS;
			argsToParse = 6;
			break;
#endif

		// A CHECK_LOYALTY {minimum} {maximum}
		case "CHECK_LOYALTY"_djb2:
			m_LogicElse = LOGIC_CHECK_LOYALTY;
			argsToParse = 2;
			break;

		// A CHECK_CHIEF
		case "CHECK_CHIEF"_djb2:
			m_LogicElse = LOGIC_CHECK_CHIEF;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_NO_CHIEF
		case "CHECK_NO_CHIEF"_djb2:
			m_LogicElse = LOGIC_CHECK_NO_CHIEF;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_CLAN_GRADE {minimum} {maximum}
		case "CHECK_CLAN_GRADE"_djb2:
			m_LogicElse = LOGIC_CHECK_CLAN_GRADE;
			argsToParse = 2;
			break;

#if 0 // TODO
		// A CHECK_KNIGHT
		case "CHECK_KNIGHT"_djb2:
			m_LogicElse = LOGIC_CHECK_KNIGHT;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_MIDDLE_STATUE_NOCAPTURE
		case "CHECK_MIDDLE_STATUE_NOCAPTURE"_djb2:
			m_LogicElse = LOGIC_CHECK_MIDDLE_STATUE_NOCAPTURE;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_MIDDLE_STATUE_CAPTURE
		case "CHECK_MIDDLE_STATUE_CAPTURE"_djb2:
			m_LogicElse = LOGIC_CHECK_MIDDLE_STATUE_CAPTURE;
			argsToParse = 1; // officially it always parses 1 even though it doesn't use it
			break;

		// A CHECK_EMPTY_SLOT {required number of empty slots}
		case "CHECK_EMPTY_SLOT"_djb2:
			m_LogicElse = LOGIC_CHECK_EMPTY_SLOT;
			argsToParse = 1;
			break;

		// A CHECK_NO_CASTLE
		case "CHECK_NO_CASTLE"_djb2:
			m_LogicElse = LOGIC_CHECK_NO_CASTLE;
			argsToParse = 1;
			break;

		// A CHECK_CASTLE
		case "CHECK_CASTLE"_djb2:
			m_LogicElse = LOGIC_CHECK_CASTLE;
			argsToParse = 1;
			break;

		// A CHECK_MONSTER_CHALLENGE_USERCOUNT {current number of users in Forgotten Temple}
		case "CHECK_MONSTER_CHALLENGE_USERCOUNT"_djb2:
			m_LogicElse = LOGIC_CHECK_MONSTER_CHALLENGE_USERCOUNT;
			argsToParse = 1;
			break;

		// A CHECK_STAT_TOTAL {minimum} {maximum}
		case "CHECK_STAT_TOTAL"_djb2:
			m_LogicElse = LOGIC_CHECK_STAT_TOTAL;
			argsToParse = 2;
			break;

		// A CHECK_SKILL_TOTAL {minimum} {maximum}
		case "CHECK_SKILL_TOTAL"_djb2:
			m_LogicElse = LOGIC_CHECK_SKILL_TOTAL;
			argsToParse = 2;
			break;
#endif // 0

		default:
			spdlog::warn("LOGIC_ELSE::Parse_And: unhandled opcode '{}' ({}:{})", temp, WideToUtf8(filename), lineNumber);
			break;
	}

	_ASSERT(argsToParse >= 0 && argsToParse <= MAX_LOGIC_ELSE_INT);
	for (int i = 0; i < argsToParse; i++)
	{
		index += ParseSpace(temp, line + index);
		m_LogicElseInt[i] = atoi(temp);
	}

	m_bAnd = TRUE;
}
