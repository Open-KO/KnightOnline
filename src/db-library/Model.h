#ifndef DBLIBRARY_MODEL_H
#define DBLIBRARY_MODEL_H

#pragma once

#include <string>
#include <vector>

#include "ConnectionManager.h"
#include "SqlBuilder.h"

#include <nanodbc/nanodbc.h>
#include <spdlog/spdlog.h>

namespace db
{

/// \brief wraps generated model classes to extend support for SQL operations.
/// Should never have non-static methods/members to maintain a minimal footprint
class Model
{
	// Mostly hand-coded template with some possible generated functions/properties.
public:
	template <typename T>
	using BindingIndex = std::vector<typename T::BinderType::BindColumnFunction_t>;

	/// \brief uses a SqlBuilder to select a batch of data in a single trip to the database
	///
	/// \param sql SqlBuilder used to modify the select query
	/// \note this currently acts as an iterator and is not efficient
	/// \see ModelRecordSet for efficient iterator implementation until this is done correctly
	template <typename T>
	static std::vector<T> BatchSelect(SqlBuilder<T>& sql) noexcept(false)
	{
		auto conn = ConnectionManager::CreatePoolConnection(T::DbType());
		if (conn == nullptr)
			return {};

		std::string query       = sql.SelectCountString();
		nanodbc::statement stmt = conn->CreateStatement(query);
		nanodbc::result result  = stmt.execute();
		int64_t rowCount        = 0;

		if (result.next())
		{
			rowCount = result.get<int64_t>(0);
			if (sql.Limit > 0)
				rowCount = std::min(rowCount, sql.Limit);
		}

		query             = sql.SelectString();
		stmt              = nanodbc::statement(*conn, query);
		result            = stmt.execute();

		short columnCount = result.columns();

		BindingIndex<T> bindingsIndex;
		IndexColumnNameBindings<T>(result, columnCount, bindingsIndex);

		std::vector<T> resultModels;
		if (rowCount > 0)
			resultModels.reserve(static_cast<size_t>(rowCount));

		// result always starts before the first row, so calling next will not skip the first result.
		while (result.next())
		{
			T res {};
			Model::BindResult(result, res, bindingsIndex);
			resultModels.push_back(std::move(res));
		}

		return resultModels;
	}

	/// \brief uses a SqlBuilder to select a batch of data in a single trip to the database
	///
	/// \note this currently acts as an iterator and is not efficient
	/// \see ModelRecordSet for efficient iterator implementation until this is done correctly
	template <typename T>
	static std::vector<T> BatchSelect() noexcept(false)
	{
		SqlBuilder<T> filter {};
		return std::move(BatchSelect(filter));
	}

	/// \brief attempts to bind a result row to a given model class's members
	template <typename T>
	static void BindResult(
		const nanodbc::result& result, T& model, const BindingIndex<T>& bindingIndex)
	{
		short i = 0;
		for (const auto bind : bindingIndex)
		{
			if (bind != nullptr)
				bind(model, result, i);

			++i;
		}
	}

	template <typename T>
	static void IndexColumnNameBindings(
		const nanodbc::result& result, const short columnCount, BindingIndex<T>& bindingsIndex)
	{
		using BinderType              = typename T::BinderType;

		const auto& columnBindingsMap = BinderType::GetColumnBindings();
		std::string columnName;

		bindingsIndex.reserve(columnCount);

		for (short i = 0; i < columnCount; i++)
		{
			columnName = result.column_name(i);

			auto itr   = columnBindingsMap.find(columnName);
			if (itr == columnBindingsMap.end())
			{
				spdlog::warn("No binding found for : {}.{}", T::TableName(), columnName);
				bindingsIndex.push_back(nullptr);
				continue;
			}

			bindingsIndex.push_back(itr->second);
		}
	}
};

} // namespace db

#endif // DBLIBRARY_MODEL_H
