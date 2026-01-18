#ifndef DBLIBRARY_RECORDSETLOADER_H
#define DBLIBRARY_RECORDSETLOADER_H

#pragma once

#include <string>
#include <functional>

#include "Model.h"
#include "ModelRecordSet.h"
#include "Exceptions.h"      // db::DatasourceConfigNotFoundException

#include <nanodbc/nanodbc.h> // nanodbc::database_error

namespace recordset_loader
{
struct Error
{
	enum class EnumErrorType : uint8_t
	{
		None,
		DatasourceConfig,
		Database,
		EmptyTable
	};

	EnumErrorType Type = EnumErrorType::None;
	std::string Message;
};

template <typename ModelType_, typename BoundModelType_ = ModelType_>
class Base
{
public:
	using ModelType                = ModelType_;
	using BoundModelType           = BoundModelType_;
	using RecordSetType            = db::ModelRecordSet<ModelType, BoundModelType>;

	using ProcessFetchCallbackType = std::function<void(RecordSetType&)>;

	Base()                         = default;

	inline const Error& GetError() const
	{
		return _error;
	}

	inline short GetColumnCount() const
	{
		return _columnCount;
	}

	inline void SetProcessFetchCallback(ProcessFetchCallbackType processFetchCallback)
	{
		_processFetchCallback = std::move(processFetchCallback);
	}

	inline bool Load_AllowEmpty(bool fetchRowCount = false)
	{
		return Load<true>(fetchRowCount);
	}

	inline bool Load_ForbidEmpty(bool fetchRowCount = false)
	{
		return Load<false>(fetchRowCount);
	}

protected:
	template <bool AllowEmptyTable>
	bool Load(bool fetchRowCount)
	{
		using EnumErrorType = Error::EnumErrorType;

		_columnCount        = 0;

		try
		{
			RecordSetType recordset(fetchRowCount);
			recordset.open();

			_columnCount = recordset.columnCount();

			if constexpr (!AllowEmptyTable)
			{
				if (!recordset.next())
				{
					_error.Type    = EnumErrorType::EmptyTable;
					_error.Message = "Table empty!";
					return false;
				}

				if (_processFetchCallback != nullptr)
					_processFetchCallback(recordset);
			}
			else /* constexpr branch */
			{
				// We have no reason to trigger the callback if it was empty.
				if (recordset.next())
				{
					if (_processFetchCallback != nullptr)
						_processFetchCallback(recordset);
				}
			}

			_error.Type = EnumErrorType::None;
			_error.Message.clear();

			return true;
		}
		catch (const db::DatasourceConfigNotFoundException& ex)
		{
			_error.Type    = EnumErrorType::DatasourceConfig;
			_error.Message = ex.what();
		}
		catch (const nanodbc::database_error& ex)
		{
			_error.Type    = EnumErrorType::Database;
			_error.Message = ex.what();
		}

		return false;
	}

protected:
	Error _error {};
	short _columnCount {};
	std::function<void(RecordSetType&)> _processFetchCallback {};
};

} // namespace recordset_loader

#endif // DBLIBRARY_RECORDSETLOADER_H
