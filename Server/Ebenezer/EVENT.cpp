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

#include <filesystem>
#include <spdlog/spdlog.h>

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

bool EVENT::LoadEvent(int zone)
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

	// Build the base MAP directory
	std::filesystem::path evtPath(GetProgPath().GetString());
	evtPath /= QUESTS_DIR;
	evtPath /= std::to_wstring(zone) + L".evt";

	// Doesn't exist but this isn't a problem; we don't expect it to exist.
	if (!std::filesystem::exists(evtPath))
		return true;

	// Resolve it to strip the relative references to be nice.
	// NOTE: Requires the file to exist.
	evtPath = std::filesystem::canonical(evtPath);

	filename.Format(_T("%ls"), evtPath.c_str());

	m_Zone = zone;

	if (!pFile.Open(filename, CFile::modeRead))
		return false;

	std::wstring filenameWide = evtPath.wstring();

	length = static_cast<DWORD>(pFile.GetLength());

	CArchive in(&pFile, CArchive::load);
	int lineNumber = 0;
	count = 0;

	while (count < length)
	{
		in >> byte;
		++count;

		if ((char) byte != '\r'
			&& (char) byte != '\n')
			buf[index++] = byte;

		if ((char) byte == '\n'
			|| count == length)
		{
			++lineNumber;

			if (index <= 1)
				continue;

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

//			if (0 == strcmp(first, "QUEST"))
			if (0 == strcmp(first, "EVENT"))
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
					spdlog::error("EVENT::LoadEvent: duplicate definition [eventId={}]", event_num);
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
			else if (0 == strcmp(first, "E"))
			{
				if (newData == nullptr)
					goto cancel_event_load;

				EXEC* newExec = new EXEC;
				newExec->Parse(buf + t_index, filenameWide, lineNumber);
				newData->m_arExec.push_back(newExec);
			}
			else if (0 == strcmp(first, "A"))
			{
				if (newData == nullptr)
					goto cancel_event_load;

				LOGIC_ELSE* newLogicElse = new LOGIC_ELSE;
				newLogicElse->Parse_and(buf + t_index, filenameWide, lineNumber);
				newData->m_arLogicElse.push_back(newLogicElse);
			}
			else if (0 == strcmp(first, "END"))
			{
				if (newData == nullptr)
					goto cancel_event_load;

				newData = nullptr;
			}
			else if (isalnum(first[0]))
			{
				spdlog::warn(
					"EVENT::LoadEvent({}): unhandled opcode '{}' ({}:{})",
					zone, first, WideToUtf8(filenameWide), lineNumber);
			}
			index = 0;
		}
	}

	in.Close();
	pFile.Close();

	return true;

cancel_event_load:
	CString str;
	str.Format(_T("QUEST INFO READ FAIL (%d)(%d)"), zone, event_num);
	AfxMessageBox(str);
	in.Close();
	pFile.Close();
	DeleteAll();
	return false;
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
