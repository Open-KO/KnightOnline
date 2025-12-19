#ifndef FILEIO_FILEWRITER_H
#define FILEIO_FILEWRITER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

class FileWriter : public File
{
public:
	uint64_t SizeOnDisk() const
	{
		return _sizeOnDisk;
	}

	FileWriter();
	bool OpenExisting(const std::filesystem::path& path) override;
	bool Create(const std::filesystem::path& path) override;
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr) override;
	bool Write(const void* buffer, size_t bytesToWrite, size_t* bytesWritten = nullptr) override;
	bool Seek(int64_t offset, int origin) override;
	void Flush() override;
	bool Close() override;
	~FileWriter() override;

protected:
	LLFIO_V2_NAMESPACE::file_handle _fileHandle;
	uint64_t _sizeOnDisk;
};

#endif // FILEIO_FILEWRITER_H
