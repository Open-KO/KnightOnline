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

inline constexpr int MAX_USER           = 3000;

inline constexpr int _LISTEN_PORT       = 15100;
inline constexpr int DB_PROCESS_TIMEOUT = 100;

////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
inline constexpr int SOCKET_BUFF_SIZE   = 1024 * 16;
inline constexpr int MAX_PACKET_SIZE    = 1024 * 8;

inline constexpr uint8_t PACKET_START1  = 0xAA;
inline constexpr uint8_t PACKET_START2  = 0x55;
inline constexpr uint8_t PACKET_END1    = 0x55;
inline constexpr uint8_t PACKET_END2    = 0xAA;

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
inline constexpr std::string_view ODBC        = "ODBC";
inline constexpr std::string_view DSN         = "DSN";
inline constexpr std::string_view UID         = "UID";
inline constexpr std::string_view PWD         = "PWD";

// SERVER_LIST section
inline constexpr std::string_view SERVER_LIST = "SERVER_LIST";
inline constexpr std::string_view COUNT       = "COUNT";

// Download section
inline constexpr std::string_view DOWNLOAD    = "DOWNLOAD";
inline constexpr std::string_view URL         = "URL";
inline constexpr std::string_view PATH        = "PATH";
} // namespace ini

} // namespace VersionManager

#endif // SERVER_VERSIONMANAGER_DEFINE_H
