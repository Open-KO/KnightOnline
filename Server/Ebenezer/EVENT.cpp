#include "pch.h"
#include "Define.h"
#include "EVENT.h"
#include "EVENT_DATA.h"
#include "EXEC.h"
#include "LOGIC_ELSE.h"

#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>

EVENT::EVENT()
{
}

EVENT::~EVENT()
{
	DeleteAll();
}

bool EVENT::LoadEvent(int zone, const std::filesystem::path& questsDir)
{
	uintmax_t	length, count;
	uint8_t		byte;
	char		buf[4096];
	char		first[1024];
	char		temp[1024];
	int			index = 0;
	int			t_index = 0;
	int			event_num = -1;

	EVENT_DATA* newData = nullptr;
	EVENT_DATA* eventData = nullptr;
	std::error_code ec;

	std::filesystem::path questPath = questsDir;
	questPath /= std::to_string(zone) + ".evt";

	// Doesn't exist but this isn't a problem; we don't expect it to exist.
	if (!std::filesystem::exists(questPath))
		return true;

	// Resolve it to strip the relative references (to be nice).
	// NOTE: Requires the file to exist.
	questPath = std::filesystem::canonical(questPath);

	length = std::filesystem::file_size(questPath, ec);
	if (ec)
		return false;

	m_Zone = zone;

	std::ifstream file(questPath, std::ios::in | std::ios::binary);
	if (!file)
		return false;

	std::u8string filenameUtf8 = questPath.u8string();

	// NOTE: spdlog is a C++11 library that doesn't support std::filesystem or std::u8string
	// This just ensures the path is always explicitly UTF-8 in a cross-platform way.
	std::string filename(filenameUtf8.begin(), filenameUtf8.end());

	int lineNumber = 0;
	count = 0;

	while (count < length)
	{
		file.read(reinterpret_cast<char*>(&byte), 1);
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

			buf[index] = (uint8_t) 0;

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
				newExec->Parse(buf + t_index, filename, lineNumber);
				newData->m_arExec.push_back(newExec);
			}
			else if (0 == strcmp(first, "A"))
			{
				if (newData == nullptr)
					goto cancel_event_load;

				LOGIC_ELSE* newLogicElse = new LOGIC_ELSE;
				newLogicElse->Parse_and(buf + t_index, filename, lineNumber);
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
				spdlog::warn("EVENT::LoadEvent({}): unhandled opcode '{}' ({}:{})",
					zone, first, filename, lineNumber);
			}
			index = 0;
		}
	}

	file.close();

	return true;

cancel_event_load:
	spdlog::error("QUEST INFO READ FAIL ({})({})", zone, event_num);
	file.close();
	DeleteAll();
	return false;
}

void EVENT::Init()
{
	DeleteAll();
}

void EVENT::DeleteAll()
{
	if (!m_arEvent.IsEmpty())
		m_arEvent.DeleteAllData();
}
