#ifndef SERVER_VERSIONMANAGER_DEFINE_H
#define SERVER_VERSIONMANAGER_DEFINE_H

#pragma once

#include <filesystem>
#if defined(_DEBUG)
#include <iostream>
#endif
#include <string>

#include <shared/globals.h>
#include <shared-server/STLMap.h>

constexpr int MAX_USER				= 3000;

constexpr int _LISTEN_PORT			= 15100;
constexpr int DB_PROCESS_TIMEOUT	= 100;

////////////////////////////////////////////////////////////
// Socket Define
////////////////////////////////////////////////////////////
#define SOCKET_BUFF_SIZE		(1024*16)
#define MAX_PACKET_SIZE			(1024*8)

#define PACKET_START1			0XAA
#define PACKET_START2			0X55
#define PACKET_END1				0X55
#define PACKET_END2				0XAA

// status
#define STATE_CONNECTED			0X01
#define STATE_DISCONNECTED		0X02
////////////////////////////////////////////////////////////

typedef union
{
	int16_t		i;
	uint8_t		b[2];
} MYSHORT;

#include <VersionManager/model/VersionManagerModel.h>
namespace model = versionmanager_model; 

struct _NEWS
{
	char Content[4096]	= {};
	int16_t Size		= 0;
};

struct _SERVER_INFO
{
	char	strServerIP[20]		= {};
	char	strServerName[20]	= {};
	int16_t	sUserCount			= 0;
	int16_t	sUserLimit			= 0;
	int16_t	sServerID			= 1;
};

typedef CSTLMap <model::Version>	VersionInfoList;

// ini config variable names
namespace ini
{
	// ODBC Config Section
	static constexpr char ODBC[] = "ODBC";
	static constexpr char DSN[] = "DSN";
	static constexpr char UID[] = "UID";
	static constexpr char PWD[] = "PWD";

	// SERVER_LIST section
	static constexpr char SERVER_LIST[] = "SERVER_LIST";
	static constexpr char COUNT[] = "COUNT";

	// Download section
	static constexpr char DOWNLOAD[] = "DOWNLOAD";
	static constexpr char URL[] = "URL";
	static constexpr char PATH[] = "PATH";
}

#endif // SERVER_VERSIONMANAGER_DEFINE_H
