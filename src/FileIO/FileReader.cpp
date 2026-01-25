#include "FileReader.h"

#include <cassert>
#include <cstdio>  // SEEK_SET, SEEK_CUR, SEEK_END
#include <cstring> // std::memcpy()

namespace boost_ipc = boost::interprocess;

FileReader::FileReader()
{
}

bool FileReader::OpenExisting(const std::filesystem::path& path)
{
	// Close any existing file handle and reset read states.
	Close();

	// Open and map the given file into memory for reading.
	try
	{
		_mappedFileHandle = boost_ipc::file_mapping(path.native().c_str(), boost_ipc::read_only);
		_mappedFileRegion = boost_ipc::mapped_region(_mappedFileHandle, boost_ipc::read_only);
	}
	catch (const boost_ipc::interprocess_exception&)
	{
		return false;
	}

	_address = _mappedFileRegion.get_address();
	_size    = static_cast<uint64_t>(_mappedFileRegion.get_size());
	_path    = path;
	_open    = true;

	return true;
}

bool FileReader::Create(const std::filesystem::path& /*path*/)
{
	// A FileReader cannot create files.
	return false;
}

bool FileReader::Read(void* buffer, size_t bytesToRead, size_t* bytesRead /*= nullptr*/)
{
	// Ensure bytesRead is always initialised, regardless of whether this succeeds or fails.
	if (bytesRead != nullptr)
		*bytesRead = 0;

	// Fail on invalid buffers.
	if (buffer == nullptr)
		return false;

	// Nothing to read, so just skip ahead and consider this successful.
	if (bytesToRead == 0)
		return true;

	assert(_open);
	assert(_offset <= _size);

	// Determine how many bytes are remaining to be read.
	const size_t remainingBytes = static_cast<size_t>(_size - _offset);

	// If we're trying to read more than is available, we should still
	// attempt the read, copying only what's actually available.
	const size_t bytesToCopy    = std::min(bytesToRead, remainingBytes);

	// Read operations can only be considered successful if we actually
	// manage to read at least 1 byte.
	// If there's nothing available to read, we must fail.
	if (bytesToCopy == 0)
		return false;

	// Read directly from our memory mapped buffer.
	std::memcpy(buffer, static_cast<uint8_t*>(_address) + _offset, bytesToCopy);

	// Advance the current file offset by how much we actually read.
	_offset += bytesToCopy;

	// Update the caller with the number bytes we actually read.
	if (bytesRead != nullptr)
		*bytesRead = bytesToCopy;

	// We read at least 1 byte, even if it wasn't the full amount.
	return true;
}

bool FileReader::Write(
	const void* /*buffer*/, size_t /*bytesToWrite*/, size_t* /*bytesWritten = nullptr*/)
{
	// Write operations are not supported in a FileReader.
	return false;
}

bool FileReader::Seek(int64_t offset, int origin)
{
	// File not opened, we cannot seek.
	if (!IsOpen())
		return false;

	int64_t newOffset = offset;

	switch (origin)
	{
		// Explicitly set to the given offset
		case SEEK_SET:
			break;

		// Set is relative to the current offset
		case SEEK_CUR:
			newOffset += static_cast<int64_t>(_offset);
			break;

		// Set is relative to the end offset (i.e. the size)
		case SEEK_END:
			newOffset += static_cast<int64_t>(_size);
			break;

		default:
			// Unsupported seek type.
			return false;
	}

	// New offset would be before the start of the file, so it is invalid and should fail.
	if (newOffset < 0)
		return false;

	// New offset is after the end of the file, so it is invalid and shoul fail.
	if (static_cast<uint64_t>(newOffset) > _size)
		return false;

	// Update the current file offset.
	_offset = static_cast<uint64_t>(newOffset);

	// The file offset is valid, so we can consider this request successful.
	return true;
}

void FileReader::Flush()
{
	// Nothing to flush in a FileReader.
}

bool FileReader::Close()
{
	if (!_open)
		return false;

	_mappedFileRegion = {};
	_mappedFileHandle = {};

	_size             = 0;
	_offset           = 0;
	_address          = nullptr;
	_open             = false;
	return true;
}

FileReader::~FileReader()
{
}
