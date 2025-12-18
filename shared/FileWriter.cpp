#include "pch.h"
#include "FileWriter.h"

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdio.h> // SEEK_SET, SEEK_CUR, SEEK_END

namespace llfio = LLFIO_V2_NAMESPACE;

FileWriter::FileWriter()
{
}

bool FileWriter::Open(const std::filesystem::path& path)
{
	// Close any existing file handle and reset write states.
	Close();

	auto handleResult = llfio::file(
		{},
		path.native(),
		llfio::handle::mode::write,
		llfio::handle::creation::always_new,
		llfio::handle::caching::all,
		llfio::handle::flag::none);
	if (!handleResult)
		return false;

	_fileHandle = std::move(std::move(handleResult).value());
	_path = path;
	return true;
}

bool FileWriter::Read(void* buffer, size_t bytesToRead, size_t* bytesRead)
{
	spdlog::error("FileWriter::Read: Read not supported in a writer");
	assert(!"FileWriter: Read not supported");
	return false;
}

bool FileWriter::Write(void* buffer, size_t bytesToWrite, size_t* bytesWritten)
{
	assert(_fileHandle.is_valid());
	assert(buffer != nullptr);

	if (bytesWritten != nullptr)
		*bytesWritten = 0;

	if (bytesToWrite == 0)
		return true;

	auto writeResult = _fileHandle.write(_offset,
		{ { static_cast<std::byte*>(buffer), bytesToWrite } });
	if (!writeResult)
		return false;

	size_t effectiveBytesWritten = writeResult.value();
	_offset += effectiveBytesWritten;

	if (bytesWritten != nullptr)
		*bytesWritten = effectiveBytesWritten;

	// Succeed if we wrote all of the expected bytes.
	return effectiveBytesWritten == bytesToWrite;
}

bool FileWriter::Seek(size_t offset, int origin)
{
	switch (origin)
	{
		case SEEK_SET:
			_offset = offset;
			return true;

		case SEEK_CUR:
			_offset += offset;
			return true;

		// unsupported - we don't know how big a file is going to be,
		// it can grow as we seek & write to it.
		case SEEK_END:
			spdlog::error("FileWriter::Seek: SEEK_END not supported");
			assert(!"FileWriter::Seek: SEEK_END not supported");
			return false;
	}

	spdlog::error("FileReader::Seek: Unsupported seek type {}", origin);
	assert(!"FileReader::Seek: Unsupported seek type");

	return false;
}

void FileWriter::Close()
{
	if (_fileHandle.is_valid())
		(void) _fileHandle.close();

	_offset = 0;
}

FileWriter::~FileWriter()
{
}
