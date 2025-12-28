#ifndef DBLIBRARY_RECORDSETLOADER_VECTOR_H
#define DBLIBRARY_RECORDSETLOADER_VECTOR_H

#pragma once

#include "RecordSetLoader.h"

#include <type_traits>

namespace recordset_loader
{

	template <
		typename ModelType,
		typename BoundModelType = ModelType>
	class Vector : public Base<ModelType, BoundModelType>
	{
	public:
		using ContainerType = std::vector<BoundModelType*>;
		using RecordSetType = Base<ModelType, BoundModelType>::RecordSetType;

		Vector(ContainerType& targetContainer)
			: _targetContainer(targetContainer)
		{
			this->SetProcessFetchCallback(
				std::bind(&Vector::ProcessFetch, this, std::placeholders::_1));
		}

	protected:
		void ProcessFetch(RecordSetType& recordset)
		{
			ContainerType localContainer;

			const auto& rowCount = recordset.rowCount();
			if (rowCount.has_value())
				localContainer.reserve(static_cast<size_t>(*rowCount));

			do
			{
				BoundModelType* model = new BoundModelType();
				recordset.get_ref(*model);
				localContainer.push_back(model);
			}
			while (recordset.next());

			_targetContainer.swap(localContainer);
		}

	protected:
		ContainerType& _targetContainer;
	};

}

#endif // DBLIBRARY_RECORDSETLOADER_VECTOR_H
