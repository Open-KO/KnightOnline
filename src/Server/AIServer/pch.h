#ifndef SERVER_AISERVER_PCH_H
#define SERVER_AISERVER_PCH_H

#pragma once

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>

#include <asio.hpp>

#include "Packet.h"
#include "Define.h"

#include <shared-server/server_config.h>
#include <shared-server/utilities.h>

#include <spdlog/spdlog.h>

#define myrand myrand_ai

#endif // SERVER_AISERVER_PCH_H
