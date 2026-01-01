#ifndef DBLIBRARY_SQLBUILDER_H
#define DBLIBRARY_SQLBUILDER_H

#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include <spdlog/spdlog.h>

namespace db
{

	/// \brief Used to create safe T-SQL statements for database ModelType objects
	template <typename ModelType>
	class SqlBuilder
	{
	public:
		/// \brief Limits the size of the result set.  Defaults to 0 for no limit
		int64_t Limit = 0;

		/// \brief adds additional SQL clause after any WHERE statements, 
		/// such as an "ORDER BY"
		std::string PostWhereClause;

		/// \brief adds a WHERE statement manually specified by the caller. Takes
		/// priority over IsWherePK.  Caller should include "WHERE" keyword when
		/// setting this string
		/// \see IsWherePK
		std::string Where;
		
		/// \brief will cause SelectString() to use a WHERE statement using PK columns
		bool IsWherePK = false;

		/// \brief returns a select query string based on any configured modifiers
		std::string SelectString()
		{
			std::string query = "SELECT ";

			if (Limit > 0)
				query += "TOP " + std::to_string(Limit) + " ";

			if (selectCols.empty())
			{
				// fill with all supported ModelType bindings as a default
				fillSelectColumns();
			}

			const std::unordered_set<std::string>& modelBlobColumns
				= ModelType::BlobColumns();

			std::vector<std::string> deferredBlobColumns;
			deferredBlobColumns.reserve(modelBlobColumns.size());

			short i = 0;
			for (const std::string& col : selectCols)
			{
				// long-data blob columns aren't always stored with the result set
				// they need to be fetched after all regular columns, so we should
				// defer them until the end of the list
				if (modelBlobColumns.contains(col))
				{
					deferredBlobColumns.push_back(col);
					continue;
				}

				if (i > 0)
					query += ", ";

				query += '[' + col + ']';
				i++;
			}

			// all deferred long-data blob columns can now be added
			// to the end of our list to ensure they will be fetched
			for (const std::string& col : deferredBlobColumns)
			{
				if (i > 0)
					query += ", ";

				query += '[' + col + ']';
				i++;
			}

			query += " FROM [" + ModelType::TableName() + ']';

			if (!Where.empty())
			{
				query += " " + Where;
			}
			else if (IsWherePK && !ModelType::PrimaryKey().empty())
			{
				query += " WHERE ";
				i = 0;
				for (const std::string& col : ModelType::PrimaryKey())
				{
					if (i > 0)
						query += " AND ";
					query += '[' + col + "] = ?";
					i++;
				}
			}

			if (!PostWhereClause.empty())
			{
				query += " " + PostWhereClause;
			}

			spdlog::trace("using query: {}", query);
			return query;
		}

		/// \brief returns a select (row) count query string based on any configured modifiers
		std::string SelectCountString()
		{
			std::string query = "SELECT COUNT(*) FROM [" + ModelType::TableName() + ']';
			if (!Where.empty())
			{
				query += " " + Where;
			}
			else if (IsWherePK && !ModelType::PrimaryKey().empty())
			{
				query += " WHERE ";
				short i = 0;
				for (const std::string& col : ModelType::PrimaryKey())
				{
					if (i > 0)
						query += " AND ";
					query += '[' + col + "] = ?";
					i++;
				}
			}
			
			if (!PostWhereClause.empty())
			{
				query += " " + PostWhereClause;
			}
			
			spdlog::trace("using query: {}", query);
			return query;
		}

		std::string InsertString()
		{
			std::string insertQuery = "INSERT INTO [" + ModelType::TableName() + "] (";
			std::string paramList;
			for (const std::string& colName : ModelType::OrderedColumnNames())
			{
				if (paramList.length() > 0)
				{
					insertQuery += ", ";
					paramList += ", ";
				}
				insertQuery += '[' + colName + ']';
				paramList += '?';
			}
			insertQuery += ") VALUES (" + paramList + ")";

			spdlog::trace("using query: {}", insertQuery);
			return insertQuery;
		}

		// untested but should be almost right if needed
		// std::string UpdateString()
		// {
		// 	std::string updateQuery = "UPDATE [" + ModelType::TableName() + "] SET ";
		// 	std::string paramList;
		// 	for (const std::string& colName : ModelType::OrderedColumnNames())
		// 	{
		// 		if (paramList.length() > 0)
		// 		{
		// 			updateQuery += ", ";
		// 		}
		// 		updateQuery += '[' + colName + "] = ?";
		// 	}
		// if (!Where.empty())
		// {
		// 	query += " " + Where;
		// }
		// if (!PostWhereClause.empty())
		// {
		// 	query += " " + PostWhereClause;
		// }
		// }

		/// \brief returns a select query built using the table's PK.  Delete statements
		/// not using the PK should be carefully hand coded.
		/// \note does not support manual use of Where
		std::string DeleteByIdString()
		{
			std::string query;
			// do not create queries for tables with defined PKs
			if (ModelType::PrimaryKey().empty())
			{
				return query;
			}

			query = "DELETE FROM [" + ModelType::TableName() + "] WHERE ";
			short i = 0;
			for (const std::string& col : ModelType::PrimaryKey())
			{
				if (i > 0)
					query += " AND ";
				query += '[' + col + "] = ?";
				i++;
			}

			spdlog::trace("using query: {}", query);
			return query;
		}
		
		/// \brief sets the columns for a select statement to a subset of bindable values
		///
		/// \param columns list of columns to select
		/// \warning if an input column is not valid for binding it will be ignored and
		/// a warn message will be logged
		void SetSelectColumns(const std::vector<std::string>& columns)
		{
			selectCols.clear();
			for (const std::string& col : columns)
			{
				if (!isValidColumnName(col))
				{
					spdlog::warn("Invalid column name: {}.{}", ModelType::TableName(), col);
					continue;
				}

				selectCols.insert(col);
			}
		}

		/// \brief excludes a list of columns from the current binding set
		///
		/// \param columns list of columns to exclude from the query
		void ExcludeColumns(const std::vector<std::string>& columns)
		{
			// if selectColumns hasn't been populated, assume the default of SELECT * and filter from that
			if (selectCols.empty())
				fillSelectColumns();

			for (const std::string& columnName : columns)
				selectCols.erase(columnName);
		}

	private:
		// selectCols is a set of columns that will be used as part of a SELECT query.
		// we use an std::set to prevent duplicate column entries
		std::unordered_set<std::string> selectCols;

		inline bool isValidColumnName(const std::string& colName) const
		{
			return ModelType::ColumnNames().contains(colName);
		}

		/// \brief sets selectedCols to the ModelType's supported column set
		void fillSelectColumns()
		{
			selectCols.clear();
			selectCols.insert(ModelType::ColumnNames().begin(), ModelType::ColumnNames().end());
		}
	};

}

#endif // DBLIBRARY_SQLBUILDER_H
