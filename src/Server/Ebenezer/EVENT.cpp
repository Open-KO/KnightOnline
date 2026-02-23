#include "pch.h"
#include "Define.h"
#include "EVENT.h"
#include "EVENT_DATA.h"
#include "EXEC.h"
#include "LOGIC_ELSE.h"

#include <filesystem>
#include <FileIO/FileReader.h>

#include <spdlog/spdlog.h>

namespace Ebenezer
{

EVENT::EVENT()
{
}

EVENT::~EVENT()
{
	DeleteAll();
}

bool EVENT::LoadEvent(int zone, const std::filesystem::path& questsDir)
{
	if (!LoadEventImpl(zone, questsDir))
	{
		DeleteAll();
		return false;
	}

	return true;
}

bool EVENT::LoadEventImpl(int zone, const std::filesystem::path& questsDir)
{
	uintmax_t length = 0, count = 0;
	uint8_t byte = 0;
	int index = 0, t_index = 0, event_num = -1;
	EVENT_DATA* newData   = nullptr;
	EVENT_DATA* eventData = nullptr;

	char buf[4096] {}, first[1024] {}, temp[1024] {};

	std::error_code ec;

	std::string baseFilename        = std::to_string(zone) + ".evt";
	std::filesystem::path questPath = questsDir / baseFilename;

	// Doesn't exist but this isn't a problem; we don't expect it to exist.
	if (!std::filesystem::exists(questPath, ec))
		return true;

	// Resolve it to strip the relative references (to be nice).
	// NOTE: Requires the file to exist.
	questPath = std::filesystem::canonical(questPath, ec);
	if (ec)
		return false;

	length = std::filesystem::file_size(questPath, ec);
	if (ec)
		return false;

	m_Zone = zone;

	FileReader file;
	if (!file.OpenExisting(questPath))
		return false;

	std::u8string filenameUtf8 = questPath.u8string();

	// NOTE: spdlog is a C++11 library that doesn't support std::filesystem or std::u8string
	// This just ensures the path is always explicitly UTF-8 in a cross-platform way.
	std::string filename(filenameUtf8.begin(), filenameUtf8.end());

	int lineNumber = 0;
	count          = 0;

	while (count < length)
	{
		file.Read(&byte, 1);
		++count;

		if ((char) byte != '\r' && (char) byte != '\n')
			buf[index++] = byte;

		if ((char) byte == '\n' || count == length)
		{
			++lineNumber;

			if (index <= 1)
				continue;

			buf[index] = (uint8_t) 0;

			t_index    = 0;

			// 주석에 대한 처리
			if (buf[t_index] == ';' || buf[t_index] == '/')
			{
				index = 0;
				continue;
			}

			ParseSpace(first, buf, t_index);

			//			if (0 == strcmp(first, "QUEST"))
			if (0 == strcmp(first, "EVENT"))
			{
				ParseSpace(temp, buf, t_index);
				event_num = atoi(temp);

				if (newData != nullptr)
				{
					delete newData;

					spdlog::error("EVENT::LoadEvent: parsing failed - already within EVENT block "
								  "[zoneId={} eventId={} lineNumber={}]",
						zone, event_num, lineNumber);
					return false;
				}

				if (m_arEvent.GetData(event_num))
				{
					spdlog::error("EVENT::LoadEvent: parsing failed - duplicate definition "
								  "[zoneId={} eventId={} lineNumber={}]",
						zone, event_num, lineNumber);
					return false;
				}

				eventData             = new EVENT_DATA;
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
				{
					spdlog::error("EVENT::LoadEvent: parsing failed - '{}' missing existing event "
								  "[zoneId={} eventId={} lineNumber={}]",
						first, zone, event_num, lineNumber);
					return false;
				}

				EXEC* newExec = new EXEC;
				if (!newExec->Parse(buf + t_index, filename, lineNumber))
				{
					ParseSpace(temp, buf, t_index);

					newData->_unhandledOpcodes.emplace_back(
						fmt::format("{}:{} {} {}", baseFilename, lineNumber, first, temp));
				}
				newData->m_arExec.push_back(newExec);
			}
			else if (0 == strcmp(first, "A"))
			{
				if (newData == nullptr)
				{
					spdlog::error("EVENT::LoadEvent: parsing failed - '{}' missing existing event "
								  "[zoneId={} eventId={} lineNumber={}]",
						first, zone, event_num, lineNumber);
					return false;
				}

				LOGIC_ELSE* newLogicElse = new LOGIC_ELSE;
				if (!newLogicElse->Parse_and(buf + t_index, filename, lineNumber))
				{
					ParseSpace(temp, buf, t_index);

					newData->_unhandledOpcodes.emplace_back(
						fmt::format("{}:{} {} {}", baseFilename, lineNumber, first, temp));
				}
				newData->m_arLogicElse.push_back(newLogicElse);
			}
			else if (0 == strcmp(first, "END"))
			{
				if (newData == nullptr)
				{
					spdlog::error("EVENT::LoadEvent: parsing failed - '{}' missing existing event "
								  "[zoneId={} eventId={} lineNumber={}]",
						first, zone, event_num, lineNumber);
					return false;
				}

				newData = nullptr;
			}
			else if (isalnum(first[0]))
			{
				spdlog::warn("EVENT::LoadEvent({}): unhandled opcode '{}' ({}:{})", zone, first,
					filename, lineNumber);
			}
			index = 0;
		}
	}
	return true;
}

void EVENT::Init()
{
	DeleteAll();
}

void EVENT::DeleteAll()
{
	m_arEvent.DeleteAllData();
}

} // namespace Ebenezer
