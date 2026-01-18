#ifndef DBLIBRARY_UTILS_H
#define DBLIBRARY_UTILS_H

#pragma once

#include <string>

namespace nanodbc
{
class database_error;
}

namespace db
{

namespace utils
{
void LogDatabaseError(const nanodbc::database_error& dbErr, const char* source);
}

} // namespace db

#endif // DBLIBRARY_UTILS_H
