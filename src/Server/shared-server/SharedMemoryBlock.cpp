#include "pch.h"
#include "SharedMemoryBlock.h"

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <spdlog/spdlog.h>

using namespace boost::interprocess;

struct mapped_region_impl : public mapped_region
{
	using mapped_region::mapped_region;
};

struct shared_memory_object_impl : public shared_memory_object
{
	using shared_memory_object::shared_memory_object;
};

SharedMemoryBlock::SharedMemoryBlock()
{
}

char* SharedMemoryBlock::OpenOrCreate(const std::string& name, uint32_t totalSize)
{
	// Attempt to open first.
	char* segment = Open(name);
	if (segment != nullptr)
		return segment;

	try
	{
		auto sharedMemoryObject = std::make_unique<shared_memory_object_impl>(
			create_only, name.c_str(), read_write);
		sharedMemoryObject->truncate(totalSize);

		auto mappedRegion   = std::make_unique<mapped_region_impl>(*sharedMemoryObject, read_write);

		_sharedMemoryObject = std::move(sharedMemoryObject);
		_mappedRegion       = std::move(mappedRegion);

		_name               = name;
		_created            = true;

		return static_cast<char*>(_mappedRegion->get_address());
	}
	catch (const interprocess_exception& ex)
	{
		spdlog::error("SharedMemoryBlock::OpenOrCreate: failed to create shared memory segment. "
					  "name='{}' ex={}",
			name, ex.what());
	}

	return nullptr;
}

char* SharedMemoryBlock::Open(const std::string& name)
{
	Release();

	try
	{
		auto sharedMemoryObject = std::make_unique<shared_memory_object_impl>(
			open_only, name.c_str(), read_write);
		auto mappedRegion   = std::make_unique<mapped_region_impl>(*sharedMemoryObject, read_write);

		_sharedMemoryObject = std::move(sharedMemoryObject);
		_mappedRegion       = std::move(mappedRegion);

		_name               = name;

		return static_cast<char*>(_mappedRegion->get_address());
	}
	catch (const interprocess_exception& ex)
	{
		if (ex.get_error_code() != not_found_error)
		{
			spdlog::error("SharedMemoryBlock::Open: failed to open existing shared memory block. "
						  "name='{}' ex={}",
				name, ex.what());
		}
	}

	return nullptr;
}

void SharedMemoryBlock::Release()
{
	if (_created)
	{
		shared_memory_object::remove(_name.c_str());
		_created = false;
	}

	_name.clear();
	_mappedRegion.reset();
	_sharedMemoryObject.reset();
}

SharedMemoryBlock::~SharedMemoryBlock()
{
	Release();
}
