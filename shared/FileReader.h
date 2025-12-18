#ifndef SHARED_FILEREADER_H
#define SHARED_FILEREADER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

class FileReader : public File
{
public:
	const size_t Size() const
	{
		return _size;
	}

	FileReader();
	bool Open(const std::filesystem::path& path) override;
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead) override;
	bool Write(void* buffer, size_t byteToWrite, size_t* bytesWritten) override;
	bool Seek(size_t offset, int origin) override;
	void Close() override;
	~FileReader() override;

protected:
	LLFIO_V2_NAMESPACE::mapped_file_handle _mappedFileHandle;
	void* _address;
	size_t _size;
};

#endif // SHARED_FILEREADER_H
