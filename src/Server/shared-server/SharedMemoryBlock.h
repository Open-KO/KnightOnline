#ifndef SERVER_SHAREDSERVER_SHAREDMEMORYBLOCK_H
#define SERVER_SHAREDSERVER_SHAREDMEMORYBLOCK_H

#pragma once

#include <memory>
#include <string>

struct mapped_region_impl;
struct shared_memory_object_impl;
class SharedMemoryBlock
{
public:
	SharedMemoryBlock();
	char* OpenOrCreate(const std::string& name, uint32_t totalSize);
	char* Open(const std::string& name);
	void Release();
	~SharedMemoryBlock();

private:
	std::string _name;
	std::unique_ptr<shared_memory_object_impl> _sharedMemoryObject;
	std::unique_ptr<mapped_region_impl> _mappedRegion;
	bool _created = false;
};

#endif // SERVER_SHAREDSERVER_SHAREDMEMORYBLOCK_H
