#ifndef DBLIBRARY_EXCEPTIONS_H
#define DBLIBRARY_EXCEPTIONS_H

#pragma once

#include <stdexcept>
#include <string>
#include <format>

#include <nanodbc/nanodbc.h>

namespace db
{

	class DatasourceConfigNotFoundException : public std::runtime_error
	{
	public:
		explicit DatasourceConfigNotFoundException(const std::string& message)
			: std::runtime_error(message)
		{
		}
	};

	class ApplicationError : public nanodbc::database_error
	{
	public:
		explicit ApplicationError(const std::string& message)
			: nanodbc::database_error(nullptr, 0, std::format("[application error] {}", message))
		{
		}
	};

}

#endif // DBLIBRARY_EXCEPTIONS_H
