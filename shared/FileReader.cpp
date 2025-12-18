#include "pch.h"
#include "FileReader.h"

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdio.h> // SEEK_SET, SEEK_CUR, SEEK_END

namespace llfio = LLFIO_V2_NAMESPACE;

FileReader::FileReader()
{
	_address = nullptr;
	_size = 0;
}

bool FileReader::Open(const std::filesystem::path& path)
{
	// Close any existing file handle and reset read states.
	Close();

	std::error_code ec;
	auto fileSize = std::filesystem::file_size(path, ec);
	if (ec)
		return false;

	auto handleResult = llfio::mapped_file(
		{},
		path.native(),
		llfio::handle::mode::read,
		llfio::handle::creation::open_existing,
		llfio::handle::caching::all,
		llfio::handle::flag::none);
	if (!handleResult)
		return false;

	_mappedFileHandle = std::move(std::move(handleResult).value());
	_address = _mappedFileHandle.address();
	_size = static_cast<size_t>(fileSize);
	_path = path;
	return true;
}

bool FileReader::Read(void* buffer, size_t bytesToRead, size_t* bytesRead /*= nullptr*/)
{
	assert(buffer != nullptr);

	if (bytesRead != nullptr)
		*bytesRead = 0;

	if (bytesToRead == 0)
		return true;

	assert(_mappedFileHandle.is_valid());
	assert(_offset <= _size);

	const size_t remainingBytes = _size - _offset;
	const size_t bytesToCopy = std::min(bytesToRead, remainingBytes);

	if (bytesToCopy == 0)
		return false;

	std::memcpy(buffer, static_cast<uint8_t*>(_address) + _offset, bytesToCopy);

	_offset += bytesToCopy;

	if (bytesRead != nullptr)
		*bytesRead = bytesToCopy;

	// Succeed if we read all of the expected bytes.
	return bytesToCopy == bytesToRead;
}

bool FileReader::Write(void* buffer, size_t byteToWrite, size_t* bytesWritten /*= nullptr*/)
{
	spdlog::error("FileReader::Write: Write not supported in a reader");
	assert(!"FileReader: Write not supported");
	return false;
}

bool FileReader::Seek(size_t offset, int origin)
{
	switch (origin)
	{
		case SEEK_SET:
			if (offset > _size)
				return false;

			_offset = offset;
			return true;

		case SEEK_CUR:
			if ((_offset + offset) > _size)
				return false;

			_offset += offset;
			return true;

		case SEEK_END:
			if (offset > _size)
				return false;

			_offset = _size - offset;
			return true;
	}

	spdlog::error("FileReader::Seek: Unsupported seek type {}", origin);
	assert(!"FileReader::Seek: Unsupported seek type");

	return false;
}

void FileReader::Close()
{
	if (_mappedFileHandle.is_valid())
	{
		(void) _mappedFileHandle.close();
		_address = nullptr;
	}

	_size = 0;
	_offset = 0;
}

FileReader::~FileReader()
{
}
