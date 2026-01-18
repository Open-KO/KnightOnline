#include "pch.h"
#include "ItemManagerApp.h"
#include "ItemManagerLogger.h"
#include "ItemManagerReadQueueThread.h"

#include <shared/Ini.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <filesystem>

using namespace std::chrono_literals;

namespace ItemManager
{

ItemManagerApp::ItemManagerApp(ItemManagerLogger& logger) : AppThread(logger)
{
	_telnetPort      = 2327;
	_readQueueThread = std::make_unique<ItemManagerReadQueueThread>();
	_smqOpenThread   = std::make_unique<TimerThread>(
        5s, std::bind(&ItemManagerApp::AttemptOpenSharedMemoryThreadTick, this));
}

ItemManagerApp::~ItemManagerApp()
{
	spdlog::info("ItemManagerApp::~ItemManagerApp: Shutting down, releasing resources.");

	spdlog::info("ItemManagerApp::~ItemManagerApp: Waiting for worker threads to fully shut down.");

	if (_readQueueThread != nullptr)
	{
		spdlog::info("ItemManagerApp::~ItemManagerApp: Shutting down ReadQueueThread...");

		_readQueueThread->shutdown();

		spdlog::info("ItemManagerApp::~ItemManagerApp: ReadQueueThread stopped.");
	}

	spdlog::info("ItemManagerApp::~ItemManagerApp: All resources safely released.");
}

std::filesystem::path ItemManagerApp::ConfigPath() const
{
	return "ItemManager.ini";
}

bool ItemManagerApp::OnStart()
{
	_itemLogger = spdlog::get(std::string(logger::ItemManagerItem));
	_expLogger  = spdlog::get(std::string(logger::ItemManagerExp));

	// Attempt to open shared memory queue first.
	// If it fails (memory not yet available), we'll run the _smqOpenThread to periodically check
	// until it can finally be opened.
	if (!AttemptOpenSharedMemory())
	{
		spdlog::info("ItemManagerApp::OnStart: shared memory unavailable, waiting for memory to "
					 "become available");
		_smqOpenThread->start();
	}
	else
	{
		OnSharedMemoryOpened();
	}

	return true;
}

bool ItemManagerApp::AttemptOpenSharedMemory()
{
	return LoggerRecvQueue.Open(SMQ_ITEMLOGGER);
}

void ItemManagerApp::AttemptOpenSharedMemoryThreadTick()
{
	if (!AttemptOpenSharedMemory())
		return;

	// Shared memory is open, this thread doesn't need to exist anymore.
	_smqOpenThread->shutdown(false);

	// Run the server
	OnSharedMemoryOpened();
}

void ItemManagerApp::OnSharedMemoryOpened()
{
	_readQueueThread->start();

	spdlog::info("ItemManagerApp::OnSharedMemoryOpened: server started, processing requests");
}

void ItemManagerApp::ItemLogWrite(const char* pBuf)
{
	int index = 0, srclen = 0, tarlen = 0, type = 0, putitem = 0, putcount = 0, putdure = 0;
	int64_t putserial = 0;
	char srcid[MAX_ID_SIZE + 1] {}, tarid[MAX_ID_SIZE + 1] {};

	srclen = GetShort(pBuf, index);
	if (srclen <= 0 || srclen > MAX_ID_SIZE)
	{
		spdlog::trace("### ItemLogWrite Fail : srclen = %d ###\n", srclen);
		return;
	}

	GetString(srcid, pBuf, srclen, index);

	tarlen = GetShort(pBuf, index);
	if (tarlen <= 0 || tarlen > MAX_ID_SIZE)
	{
		spdlog::trace("### ItemLogWrite Fail : tarlen = %d ###\n", tarlen);
		return;
	}

	GetString(tarid, pBuf, tarlen, index);

	type      = GetByte(pBuf, index);
	putserial = GetInt64(pBuf, index);
	putitem   = GetDWORD(pBuf, index);
	putcount  = GetShort(pBuf, index);
	putdure   = GetShort(pBuf, index);

	_itemLogger->info(
		"{}, {}, {}, {}, {}, {}, {}", srcid, tarid, type, putserial, putitem, putcount, putdure);
}

void ItemManagerApp::ExpLogWrite(const char* pBuf)
{
	int index = 0, aclen = 0, charlen = 0, type = 0, level = 0, exp = 0, loyalty = 0, money = 0;
	char acname[MAX_ID_SIZE + 1] {}, charid[MAX_ID_SIZE + 1] {};

	aclen = GetShort(pBuf, index);
	if (aclen <= 0 || aclen > MAX_ID_SIZE)
	{
		spdlog::trace("### ExpLogWrite Fail : tarlen = %d ###\n", aclen);
		return;
	}

	GetString(acname, pBuf, aclen, index);
	charlen = GetShort(pBuf, index);
	if (charlen <= 0 || charlen > MAX_ID_SIZE)
	{
		spdlog::trace("### ExpLogWrite Fail : tarlen = %d ###\n", charlen);
		return;
	}

	GetString(charid, pBuf, charlen, index);
	type    = GetByte(pBuf, index);
	level   = GetByte(pBuf, index);
	exp     = GetDWORD(pBuf, index);
	loyalty = GetDWORD(pBuf, index);
	money   = GetDWORD(pBuf, index);
	_expLogger->info(
		"{}, {}, {}, {}, {}, {}, {}", acname, charid, type, level, exp, loyalty, money);
}

} // namespace ItemManager
