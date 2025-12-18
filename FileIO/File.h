#ifndef FILEIO_FILE_H
#define FILEIO_FILE_H

#pragma once

#include <filesystem> // std::filesystem::path, std::filesystem::file_size()
#include <inttypes.h>

class File
{
public:
	const std::filesystem::path& Path() const
	{
		return _path;
	}

	uint64_t Offset() const
	{
		return _offset;
	}

	uint64_t Size() const
	{
		return _size;
	}

	bool IsOpen() const
	{
		return _open;
	}

protected:
	File() = default;
	virtual ~File() {}

public:
	virtual bool OpenExisting(const std::filesystem::path& path) = 0;
	virtual bool Create(const std::filesystem::path& path) = 0;
	virtual bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr) = 0;
	virtual bool Write(const void* buffer, size_t byteToWrite, size_t* bytesWritten = nullptr) = 0;
	virtual bool Seek(int64_t offset, int origin) = 0;
	virtual void Close() = 0;

protected:
	uint64_t _offset = 0;
	uint64_t _size = 0;
	std::filesystem::path _path;
	bool _open = false;
};

#endif // FILEIO_FILE_H
