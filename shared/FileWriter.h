#ifndef SHARED_FILEWRITER_H
#define SHARED_FILEWRITER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

class FileWriter : public File
{
public:
	FileWriter();
	bool Open(const std::filesystem::path& path) override;
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead) override;
	bool Write(void* buffer, size_t byteToWrite, size_t* bytesWritten) override;
	bool Seek(size_t offset, int origin) override;
	void Close() override;
	~FileWriter() override;

protected:
	LLFIO_V2_NAMESPACE::file_handle _fileHandle;
};

#endif // SHARED_FILEWRITER_H
