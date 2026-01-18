#include "pch.h"
#include "utils.h"
#include <spdlog/spdlog.h>

void db::utils::LogDatabaseError(const nanodbc::database_error& dbErr, const char* source)
{
	spdlog::error("{}: {}", source, dbErr.what());
}
