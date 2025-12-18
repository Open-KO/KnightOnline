#ifndef SHARED_FILEREADER_H
#define SHARED_FILEREADER_H

#pragma once

#include "File.h"

#include <llfio/llfio.hpp>

class FileReader : public File
{
public:
	FileReader();
	bool Create(const std::filesystem::path& path) override;
	bool OpenExisting(const std::filesystem::path& path) override;
	bool Read(void* buffer, size_t bytesToRead, size_t* bytesRead = nullptr) override;
	bool Write(const void* buffer, size_t byteToWrite, size_t* bytesWritten = nullptr) override;
	bool Seek(int64_t offset, int origin) override;
	void Close() override;
	~FileReader() override;

protected:
	LLFIO_V2_NAMESPACE::mapped_file_handle _mappedFileHandle;
	void* _address;
};

#endif // SHARED_FILEREADER_H
