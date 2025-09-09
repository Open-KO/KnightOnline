// LOGIC_ELSE.cpp: implementation of the LOGIC_ELSE class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ebenezer.h"
#include "Define.h"
#include "LOGIC_ELSE.h"

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

void LOGIC_ELSE::Parse_and(char* pBuf)
{
	int index = 0, i = 0;
	char temp[1024];

	index += ParseSpace(temp, pBuf + index);

	if (0 == strcmp(temp, "CHECK_UNDER_WEIGHT"))
	{
		m_LogicElse = LOGIC_CHECK_UNDER_WEIGHT;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Weight & Empty Slot
	}
	else if (0 == strcmp(temp, "CHECK_OVER_WEIGHT"))
	{
		m_LogicElse = LOGIC_CHECK_OVER_WEIGHT;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Weight & Empty Slot
	}
	else if (0 == strcmp(temp, "CHECK_SKILL_POINT"))
	{
		m_LogicElse = LOGIC_CHECK_SKILL_POINT;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// SkillPoint

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Below

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Above
	}
	else if (0 == strcmp(temp, "CHECK_EXIST_ITEM"))
	{
		m_LogicElse = LOGIC_CHECK_EXIST_ITEM;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Item no.

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Item count
	}
	else if (0 == strcmp(temp, "CHECK_NOEXIST_ITEM"))
	{
		m_LogicElse = LOGIC_CHECK_NOEXIST_ITEM;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Item no.

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Item count
	}
	else if (0 == strcmp(temp, "CHECK_CLASS"))
	{
		m_LogicElse = LOGIC_CHECK_CLASS;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Class 1

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Class 2

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Class 3

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Class 4

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Class 5

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Class 6
	}
	else if (0 == strcmp(temp, "CHECK_WEIGHT"))
	{
		m_LogicElse = LOGIC_CHECK_WEIGHT;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Weight & Empty Slot

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Weight & Empty Slot
	}
	// 비러머글 복권!!!
	else if (0 == strcmp(temp, "CHECK_EDITBOX"))
	{
		m_LogicElse = LOGIC_CHECK_EDITBOX;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "RAND"))
	{
		m_LogicElse = LOGIC_RAND;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Chances of you hitting the jackpot		
	}
	else if (0 == strcmp(temp, "CHECK_NOAH"))
	{
		m_LogicElse = LOGIC_CHECK_NOAH;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Minimum

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Maximum
	}
	else if (0 == strcmp(temp, "CHECK_LV"))
	{
		m_LogicElse = LOGIC_CHECK_LV;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Minimum

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Maximum
	}
	else if (0 == strcmp(temp, "HOWMUCH_ITEM"))
	{
		m_LogicElse = LOGIC_HOWMUCH_ITEM;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Minimum

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Maximum
	}
	else if (0 == strcmp(temp, "NOEXIST_COM_EVENT"))
	{
		m_LogicElse = LOGIC_NOEXIST_COM_EVENT;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "CHECK_NATION"))
	{
		m_LogicElse = LOGIC_CHECK_NATION;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Nation
	}
	else if (0 == strcmp(temp, "EXIST_COM_EVENT"))
	{
		m_LogicElse = LOGIC_EXIST_COM_EVENT;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "CHECK_PROMOTION_ELIGIBLE"))
	{
		m_LogicElse = LOGIC_CHECK_PROMOTION_ELIGIBLE;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "CHECK_MONSTER_CHALLENGE_TIME"))
	{
		m_LogicElse = LOGIC_CHECK_MONSTER_CHALLENGE_TIME;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "CHECK_PPCARD_SERIAL"))
	{
		m_LogicElse = LOGIC_CHECK_PPCARD_SERIAL;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "CHECK_NOEXIST_ITEM"))
	{
		m_LogicElse = LOGIC_CHECK_NOEXIST_ITEM;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Item

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// ???
	}
	else if (0 == strcmp(temp, "CHECK_EXIST_EVENT"))
	{
		m_LogicElse = LOGIC_CHECK_EXIST_EVENT;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// Event

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);		// 0 = not started, 1 = started but not finished, 2 = finished
	}
	else if (0 == strcmp(temp, "CHECK_ITEMCHANGE_NUM"))
	{
		m_LogicElse = LOGIC_CHECK_ITEMCHANGE_NUM;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "CHECK_NOCLASS"))
	{
		m_LogicElse = LOGIC_CHECK_NOCLASS;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}
	else if (0 == strcmp(temp, "CHECK_LOYALTY"))
	{
		m_LogicElse = LOGIC_CHECK_LOYALTY;

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);

		index += ParseSpace(temp, pBuf + index);
		m_LogicElseInt[i++] = atoi(temp);
	}

////////////////////////////////////////////////////////////////////////////

	m_bAnd = TRUE;
}

void LOGIC_ELSE::Parse_or(char* pBuf)
{
	int index = 0, i = 0;
	char temp[1024];

	index += ParseSpace(temp, pBuf + index);
	m_bAnd = FALSE;
}
