#ifndef FILEIO_FILEWRITER_H
#define FILEIO_FILEWRITER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

class FileWriter : public File
{
public:
	FileWriter();
	bool Create(const std::filesystem::path& path) override;
	bool OpenExisting(const std::filesystem::path& path) override;
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr) override;
	bool Write(const void* buffer, size_t byteToWrite, size_t* bytesWritten = nullptr) override;
	bool Seek(int64_t offset, int origin) override;
	void Close() override;
	~FileWriter() override;

protected:
	LLFIO_V2_NAMESPACE::file_handle _fileHandle;
};

#endif // FILEIO_FILEWRITER_H
