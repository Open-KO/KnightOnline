#include "FileReader.h"

#include <cassert>
#include <cstring> // std::memcpy()
#include <stdio.h> // SEEK_SET, SEEK_CUR, SEEK_END

namespace llfio = LLFIO_V2_NAMESPACE;

FileReader::FileReader()
{
	_address = nullptr;
}

bool FileReader::OpenExisting(const std::filesystem::path& path)
{
	// Close any existing file handle and reset read states.
	Close();

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
	_path = path;
	_open = true;

	// Internally the size is always fetched on load (as we don't supply a size) and is what's reserved.
	// If we can get away with it, this approach is very cheap, but we should just be careful.
	// The next best way to get this is _mappedFileHandle.underlying_file_maximum_extent(), but
	// we'd rather not use a second syscall unless we absolutely have to.
	_size = static_cast<uint64_t>(_mappedFileHandle.capacity());

	return true;
}

bool FileReader::Create(const std::filesystem::path& /*path*/)
{
	return false;
}

bool FileReader::Read(void* buffer, size_t bytesToRead, size_t* bytesRead /*= nullptr*/)
{
	if (bytesRead != nullptr)
		*bytesRead = 0;

	if (buffer == nullptr)
		return false;

	if (bytesToRead == 0)
		return true;

	assert(_mappedFileHandle.is_valid());
	assert(_offset <= _size);

	const size_t remainingBytes = static_cast<size_t>(_size - _offset);
	const size_t bytesToCopy = std::min(bytesToRead, remainingBytes);

	if (bytesToCopy == 0)
		return false;

	std::memcpy(buffer, static_cast<uint8_t*>(_address) + _offset, bytesToCopy);

	_offset += bytesToCopy;

	if (bytesRead != nullptr)
		*bytesRead = bytesToCopy;

	// We read at least 1 byte, even if it wasn't the full amount.
	return true;
}

bool FileReader::Write(const void* /*buffer*/, size_t /*bytesToWrite*/, size_t* /*bytesWritten = nullptr*/)
{
	return false;
}

bool FileReader::Seek(int64_t offset, int origin)
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

	if (newOffset < 0
		|| static_cast<uint64_t>(newOffset) > _size)
		return false;

	_offset = static_cast<uint64_t>(newOffset);
	return true;
}

void FileReader::Flush()
{
	// nothing to flush
}

bool FileReader::Close()
{
	if (!_mappedFileHandle.is_valid())
		return false;

	(void) _mappedFileHandle.close();

	_size = 0;
	_offset = 0;
	_address = nullptr;
	_open = false;
	return true;
}

FileReader::~FileReader()
{
}
