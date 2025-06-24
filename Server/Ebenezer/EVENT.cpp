// EVENT.cpp: implementation of the EVENT class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Ebenezer.h"
#include "Define.h"
#include "EVENT.h"
#include "EVENT_DATA.h"
#include "EXEC.h"
#include "LOGIC_ELSE.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EVENT::EVENT()
{
}

EVENT::~EVENT()
{
	DeleteAll();
}

BOOL EVENT::LoadEvent(int zone)
{
	DWORD		length, count;
	CString		filename;
	CFile		pFile;
	BYTE		byte;
	char		buf[4096];
	char		first[1024];
	char		temp[1024];
	int			index = 0;
	int			t_index = 0;
	int			event_num = -1;

	EVENT_DATA* newData = nullptr;
	EVENT_DATA* eventData = nullptr;

	filename.Format(".\\MAP\\%d.evt", zone);

	m_Zone = zone;

	if (!pFile.Open(filename, CFile::modeRead))
		return TRUE;

	length = static_cast<DWORD>(pFile.GetLength());

	CArchive in(&pFile, CArchive::load);

	count = 0;

	while (count < length)
	{
		in >> byte;	count ++;

		if ((char) byte != '\r'
			&& (char) byte != '\n')
			buf[index++] = byte;

		if (((char) byte == '\n' || count == length)
			&& index > 1)
		{
			buf[index] = (BYTE) 0;

			t_index = 0;

			// 주석에 대한 처리
			if (buf[t_index] == ';'
				|| buf[t_index] == '/')
			{
				index = 0;
				continue;
			}

			t_index += ParseSpace(first, buf + t_index);

//			if (!strcmp( first, "QUEST"))
			if (!strcmp(first, "EVENT"))
			{
				t_index += ParseSpace(temp, buf + t_index);
				event_num = atoi(temp);

				if (newData != nullptr)
				{
					delete newData;
					goto cancel_event_load;
				}

				if (m_arEvent.GetData(event_num))
				{
					TRACE("Quest Double !!\n");
					goto cancel_event_load;
				}

				eventData = new EVENT_DATA;
				eventData->m_EventNum = event_num;
				if (!m_arEvent.PutData(eventData->m_EventNum, eventData))
				{
					delete eventData;
					eventData = nullptr;
				}
				newData = m_arEvent.GetData(event_num);
			}
			else if (!strcmp(first, "E"))
			{
				if (newData == nullptr)
					goto cancel_event_load;

				EXEC* newExec = new EXEC;
				newExec->Parse(buf + t_index);
				newData->m_arExec.push_back(newExec);
			}
			else if (!strcmp(first, "A"))
			{
				if (newData == nullptr)
					goto cancel_event_load;

				LOGIC_ELSE* newLogicElse = new LOGIC_ELSE;
				newLogicElse->Parse_and(buf + t_index);
				newData->m_arLogicElse.push_back(newLogicElse);
			}
			else if (!strcmp(first, "END"))
			{
				if (newData == nullptr)
					goto cancel_event_load;

				newData = nullptr;
			}

			index = 0;
		}
	}

	in.Close();
	pFile.Close();

	return TRUE;

cancel_event_load:
	CString str;
	str.Format("QUEST INFO READ FAIL (%d)(%d)", zone, event_num);
	AfxMessageBox(str);
	in.Close();
	pFile.Close();
	DeleteAll();
	return FALSE;
}

void EVENT::Init()
{
	DeleteAll();
}

void EVENT::Parsing(char* pBuf)
{
}

void EVENT::DeleteAll()
{
	if (!m_arEvent.IsEmpty())
		m_arEvent.DeleteAllData();
}
