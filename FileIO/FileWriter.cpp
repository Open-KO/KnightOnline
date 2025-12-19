#include "FileWriter.h"

#include <cassert>
#include <stdio.h> // SEEK_SET, SEEK_CUR, SEEK_END

namespace llfio = LLFIO_V2_NAMESPACE;

FileWriter::FileWriter()
{
	_sizeOnDisk = 0;
}

bool FileWriter::OpenExisting(const std::filesystem::path& path)
{
	// Close any existing file handle and reset write states.
	Close();

	auto handleResult = llfio::file(
		{},
		path.native(),
		llfio::handle::mode::write,
		llfio::handle::creation::open_existing,
		llfio::handle::caching::all,
		llfio::handle::flag::none);
	if (!handleResult)
		return false;

	_fileHandle = std::move(std::move(handleResult).value());
	_path = path;
	_open = true;
	_size = 0;

	llfio::stat_t stat;

	auto statResult = stat.fill(_fileHandle, llfio::stat_t::want::size);
	if (!statResult)
	{
		std::error_code ec;
		_size = std::filesystem::file_size(path, ec);
	}
	else
	{
		_size = static_cast<uint64_t>(stat.st_size);
	}

	_sizeOnDisk = _size;

	// We've opened an existing file, so we intend to append to the end.
	_offset = _size;

	return true;
}

bool FileWriter::Create(const std::filesystem::path& path)
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
	_open = true;
	_size = 0;
	_sizeOnDisk = 0;
	return true;
}

bool FileWriter::Read(void* buffer, size_t bytesToRead, size_t* bytesRead /*= nullptr*/)
{
	return false;
}

bool FileWriter::Write(const void* buffer, size_t bytesToWrite, size_t* bytesWritten /*= nullptr*/)
{
	if (bytesWritten != nullptr)
		*bytesWritten = 0;

	if (buffer == nullptr)
		return false;

	if (bytesToWrite == 0)
		return true;

	assert(_fileHandle.is_valid());

	auto writeResult = _fileHandle.write(_offset,
		{ { static_cast<const std::byte*>(buffer), bytesToWrite } });
	if (!writeResult)
		return false;

	size_t effectiveBytesWritten = writeResult.value();
	_offset += effectiveBytesWritten;

	if (bytesWritten != nullptr)
		*bytesWritten = effectiveBytesWritten;

	if (_offset > _size)
	{
		_size = _offset;
		_sizeOnDisk = _size;
	}

	// Succeed if we wrote all of the expected bytes.
	return effectiveBytesWritten == bytesToWrite;
}

bool FileWriter::Seek(int64_t offset, int origin)
{
	int64_t newOffset = offset;

	switch (origin)
	{
		// explicitly set to the given offset
		case SEEK_SET:
			break;

		// set relative to the current offset
		case SEEK_CUR:
			newOffset += static_cast<int64_t>(_offset);
			break;

		// set relative to the end offset (i.e. the size)
		case SEEK_END:
			newOffset += static_cast<int64_t>(_size);
			break;

		default:
			return false;
	}

	if (newOffset < 0)
		return false;

	_offset = static_cast<uint64_t>(newOffset);
	if (_offset > _size)
		_size = _offset;

	return true;
}

void FileWriter::Flush()
{
	if (!_fileHandle.is_valid())
		return;

	if (_sizeOnDisk >= _size)
		return;

	uint64_t bytesToAppend = _size - _sizeOnDisk;
	std::byte dummy = {};

	auto writeResult = _fileHandle.write(_size - 1,
		{ { &dummy, 1 } });
	if (!writeResult)
		return;

	_sizeOnDisk = _size;
}

void FileWriter::Close()
{
	Flush();

	if (_fileHandle.is_valid())
		(void) _fileHandle.close();

	_offset = 0;
	_open = false;
}

FileWriter::~FileWriter()
{
}
