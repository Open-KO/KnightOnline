// EXEC.cpp: implementation of the EXEC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ebenezer.h"
#include "Define.h"
#include "EXEC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EXEC::EXEC()
{
}

EXEC::~EXEC()
{
}

void EXEC::Parse(char* pBuf)
{
	int index = 0, i = 0;
	char temp[1024];

	index += ParseSpace(temp, pBuf + index);


	if (!strcmp(temp, "SAY"))
	{
		m_Exec = EXEC_SAY;

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호1
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호2
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호3
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호4
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호5
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호6
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호7
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호8
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호9
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호10
	}
	else if (!strcmp(temp, "SELECT_MSG"))
	{
		m_Exec = EXEC_SELECT_MSG;

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 직업
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 지문 번호

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 1
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 1

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 2
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 2

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 3
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 3

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 4
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 4

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 5
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 5
		// 비러머글 퀘스트 >.<
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 6
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 6

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 7
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 7

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 8
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 8

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 9
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 9

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 선택문 10
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);		// 이벤트 10		
		//
	}
	else if (!strcmp(temp, "RUN_EVENT"))
	{
		m_Exec = EXEC_RUN_EVENT;
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);
	}
	else if (!strcmp(temp, "GIVE_ITEM"))
	{
		m_Exec = EXEC_GIVE_ITEM;

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Item no
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Item count
	}
	else if (!strcmp(temp, "ROB_ITEM"))
	{
		m_Exec = EXEC_ROB_ITEM;

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Item no.
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Item count
	}
	// 비러머글 퀘스트 >.<
	else if (!strcmp(temp, "OPEN_EDITBOX"))
	{
		m_Exec = EXEC_OPEN_EDITBOX;

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Npc id.
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Input message.
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Next Event.		
	}
	else if (!strcmp(temp, "GIVE_NOAH"))
	{
		m_Exec = EXEC_GIVE_NOAH;

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Amount of Noah
	}
	else if (!strcmp(temp, "LOG_COUPON_ITEM"))
	{
		m_Exec = EXEC_LOG_COUPON_ITEM;

		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Item ID.
		index += ParseSpace(temp, pBuf + index);	m_ExecInt[i++] = atoi(temp);	// Number of Item.
	}
	//
	else if (!strcmp(temp, "RETURN"))
	{
		m_Exec = EXEC_RETURN;
	}
}

void EXEC::Init()
{
	for (int i = 0; i < MAX_EXEC_INT; i++)
		m_ExecInt[i] = -1;
}
