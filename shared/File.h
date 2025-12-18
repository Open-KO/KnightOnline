#ifndef SHARED_FILE_H
#define SHARED_FILE_H

#pragma once

#include <filesystem> // std::filesystem::path, std::filesystem::file_size()

class File
{
public:
	const std::filesystem::path& Path() const
	{
		return _path;
	}

	const uint64_t Offset() const
	{
		return _offset;
	}

protected:
	File() = default;
	virtual bool Open(const std::filesystem::path& path) = 0;
	virtual bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead) = 0;
	virtual bool Write(void* buffer, size_t byteToWrite, size_t* bytesWritten) = 0;
	virtual bool Seek(size_t offset, int origin) = 0;
	virtual void Close() = 0;
	virtual ~File() {};

protected:
	size_t _offset = 0;
	std::filesystem::path _path;
};

#endif // SHARED_FILE_H
