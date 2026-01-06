#ifndef SERVER_VERSIONMANAGER_DEFINE_H
#define SERVER_VERSIONMANAGER_DEFINE_H

#pragma once

#include <filesystem>
#if defined(_DEBUG)
#include <iostream>
#endif

#include <string>
#include <string_view>

#include <shared/globals.h>
#include <shared-server/STLMap.h>

#include <VersionManager/model/VersionManagerModel.h>

namespace VersionManager
{

constexpr int MAX_USER           = 3000;

constexpr int _LISTEN_PORT       = 15100;
constexpr int DB_PROCESS_TIMEOUT = 100;

////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
constexpr int SOCKET_BUFF_SIZE   = 1024 * 16;
constexpr int MAX_PACKET_SIZE    = 1024 * 8;

constexpr uint8_t PACKET_START1  = 0xAA;
constexpr uint8_t PACKET_START2  = 0x55;
constexpr uint8_t PACKET_END1    = 0x55;
constexpr uint8_t PACKET_END2    = 0xAA;

typedef union
{
	int16_t i;
	uint8_t b[2];
} MYSHORT;

struct _NEWS
{
	char Content[4096] = {};
	int16_t Size       = 0;
};

struct _SERVER_INFO
{
	std::string strServerIP;
	std::string strServerName;
	int16_t sUserCount = 0;
	int16_t sUserLimit = 0;
	int16_t sServerID  = 1;
};

namespace model       = versionmanager_model;

using VersionInfoList = CSTLMap<model::Version>;

// ini config variable names
namespace ini
{
// ODBC Config Section
static constexpr std::string_view ODBC        = "ODBC";
static constexpr std::string_view DSN         = "DSN";
static constexpr std::string_view UID         = "UID";
static constexpr std::string_view PWD         = "PWD";

// SERVER_LIST section
static constexpr std::string_view SERVER_LIST = "SERVER_LIST";
static constexpr std::string_view COUNT       = "COUNT";

// Download section
static constexpr std::string_view DOWNLOAD    = "DOWNLOAD";
static constexpr std::string_view URL         = "URL";
static constexpr std::string_view PATH        = "PATH";
} // namespace ini

} // namespace VersionManager

#endif // SERVER_VERSIONMANAGER_DEFINE_H
