#ifndef DBLIBRARY_EXCEPTIONS_H
#define DBLIBRARY_EXCEPTIONS_H

#pragma once

#include <stdexcept>
#include <string>

#include <nanodbc/nanodbc.h>
#include <spdlog/fmt/fmt.h>

namespace db
{

class DatasourceConfigNotFoundException : public std::runtime_error
{
public:
	explicit DatasourceConfigNotFoundException(const std::string& message) :
		std::runtime_error(message)
	{
	}
};

class ApplicationError : public nanodbc::database_error
{
public:
	explicit ApplicationError(const std::string& message) :
		nanodbc::database_error(nullptr, 0, fmt::format("[application error] {}", message))
	{
	}
};

} // namespace db

#endif // DBLIBRARY_EXCEPTIONS_H
