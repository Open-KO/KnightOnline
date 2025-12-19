#ifndef FILEIO_FILEREADER_H
#define FILEIO_FILEREADER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

class FileReader : public File
{
public:
	FileReader();
	bool OpenExisting(const std::filesystem::path& path) override;
	bool Create(const std::filesystem::path& path) override;
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr) override;
	bool Write(const void* buffer, size_t bytesToWrite, size_t* bytesWritten = nullptr) override;
	bool Seek(int64_t offset, int origin) override;
	void Flush() override;
	bool Close() override;
	~FileReader() override;

protected:
	LLFIO_V2_NAMESPACE::mapped_file_handle _mappedFileHandle;
	void* _address;
};

#endif // FILEIO_FILEREADER_H
