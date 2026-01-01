#ifndef DBLIBRARY_MODELRECORDSET_H
#define DBLIBRARY_MODELRECORDSET_H

#pragma once

#include <memory>
#include <optional>
#include <string>

#include "ConnectionManager.h"
#include "Model.h"
#include "PoolConnection.h"
#include "SqlBuilder.h"

#include <nanodbc/nanodbc.h>

namespace db
{

	/// \brief queries the templated model table using an iterator to the result set
	template <
		typename ModelType,
		typename BoundModelType = ModelType>
	class ModelRecordSet
	{
	public:
		ModelRecordSet(bool fetchRowCount = false)
			: _fetchRowCount(fetchRowCount)
		{
		}

		~ModelRecordSet()
		{
			close();
		}

		/// \brief closes and releases all associated database resources
		void close()
		{
			_stmt.reset();
			_poolConn.reset();
		}

		/// \brief fetches the current result set's row count, if requested and available
		const std::optional<int64_t>& rowCount() const
		{
			return _rowCount;
		}

		/// \brief fetches the current result set's column count
		short columnCount() const
		{
			return _columnCount;
		}

		/// \brief binds the current result record to an associated model object and returns it
		BoundModelType get()
		{
			BoundModelType model {};
			Model::BindResult<BoundModelType>(_result, model, _bindingIndex);
			return std::move(model);
		}

		/// \brief binds the current result record to an associated model object and returns it
		void get_ref(BoundModelType& model)
		{
			Model::BindResult<BoundModelType>(_result, model, _bindingIndex);
		}

		/// \brief attempts to move the result iterator one forward
		///
		/// \return Boolean: true if there is a next record, false otherwise
		bool next()
		{
			return _result.next();
		}

		/// \brief opens a connection to the model's database and queries its table using 
		/// a SqlBuilder.
		/// Results are accessed by iterating with next() and requesting a bound model object
		/// with get()
		///
		/// \see next(), get()
		/// \throws db::DatasourceConfigNotFoundException
		/// \throws nanodbc::database_error
		void open() noexcept(false)
		{
			SqlBuilder<ModelType> filterObj {};
			prepare(filterObj);
			execute();
		}

		/// \brief opens a connection to the model's database and queries its table using 
		/// a SqlBuilder.
		/// Results are accessed by iterating with next() and requesting a bound model object
		/// with get()
		///
		/// \see next(), get()
		/// \throws db::DatasourceConfigNotFoundException
		/// \throws nanodbc::database_error
		void open(SqlBuilder<ModelType>& filterObj) noexcept(false)
		{
			prepare(filterObj);
			execute();
		}

		/// \brief opens a connection to the model's database, resets the recordset's state,
		/// and prepares a new statement for the pending lookup.
		///
		/// \throws db::DatasourceConfigNotFoundException
		/// \throws nanodbc::database_error
		std::shared_ptr<nanodbc::statement> prepare() noexcept(false)
		{
			SqlBuilder<ModelType> filterObj {};
			return prepare(filterObj);
		}

		/// \brief opens a connection to the model's database, resets the recordset's state,
		/// and prepares a new statement for the pending lookup.
		///
		/// \throws db::DatasourceConfigNotFoundException
		/// \throws nanodbc::database_error
		std::shared_ptr<nanodbc::statement> prepare(SqlBuilder<ModelType>& filterObj) noexcept(false)
		{
			_columnCount = 0;
			_rowCount.reset();

			if (_poolConn == nullptr)
				_poolConn = ConnectionManager::CreatePoolConnection(ModelType::DbType());

			if (_fetchRowCount)
				_selectCountQuery = filterObj.SelectCountString();
			else
				_selectCountQuery.clear();

			std::string query = filterObj.SelectString();

			_selectLimit = filterObj.Limit;
			_stmt = std::make_shared<nanodbc::statement>(_poolConn->CreateStatement(query));

			return _stmt;
		}

		/// \brief executes the existing prepared statement using the existing connection.
		///
		/// \see next(), get()
		/// \throws std::logic_error
		/// \throws nanodbc::database_error
		void execute() noexcept(false)
		{
			if (_stmt == nullptr)
			{
				throw std::logic_error("Statement not initialized");
			}

			if (_fetchRowCount)
			{
				nanodbc::statement stmt = _poolConn->CreateStatement(_selectCountQuery);
				nanodbc::result result = stmt.execute();

				int64_t rowCount = 0;
				if (result.next())
				{
					rowCount = result.get<int64_t>(0);
					if (_selectLimit > 0)
						rowCount = (std::min)(rowCount, _selectLimit); // NOTE: allow for Windows.h defining min

					_rowCount = rowCount;
				}
			}

			_result = _stmt->execute();
			_columnCount = _result.columns();

			Model::IndexColumnNameBindings<BoundModelType>(
				_result,
				_columnCount,
				_bindingIndex);
		}

	private:
		std::shared_ptr<PoolConnection> _poolConn {};
		std::shared_ptr<nanodbc::statement> _stmt {};
		nanodbc::result _result {};
		Model::BindingIndex<BoundModelType> _bindingIndex {};
		bool _fetchRowCount {};
		std::optional<int64_t> _rowCount {};
		short _columnCount {};
		std::string _selectCountQuery {};
		int64_t _selectLimit {};
	};

}

#endif // DBLIBRARY_MODELRECORDSET_H
