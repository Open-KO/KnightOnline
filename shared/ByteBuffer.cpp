#include "pch.h"
#include "ByteBuffer.h"

#include <cassert>

ByteBuffer::ByteBuffer()
	: _doubleByte(true), _rpos(0), _wpos(0)
{
	_storage.reserve(DEFAULT_SIZE);
}

ByteBuffer::ByteBuffer(size_t res)
	: _doubleByte(true), _rpos(0), _wpos(0)
{
	_storage.reserve(res <= 0 ? DEFAULT_SIZE : res);
}

ByteBuffer::ByteBuffer(const ByteBuffer& buf)
	: _doubleByte(true), _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage)
{
}

ByteBuffer::~ByteBuffer()
{
}

void ByteBuffer::clear()
{
	_storage.clear();
	_rpos = _wpos = 0;
}

// stream like operators for storing data
ByteBuffer& ByteBuffer::operator<<(bool value)
{
	append<char>((char) value);
	return *this;
}

// unsigned
ByteBuffer& ByteBuffer::operator<<(uint8_t value)
{
	append<uint8_t>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(uint16_t value)
{
	append<uint16_t>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(uint32_t value)
{
	append<uint32_t>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(uint64_t value)
{
	append<uint64_t>(value);
	return *this;
}

// signed as in 2e complement
ByteBuffer& ByteBuffer::operator<<(int8_t value)
{
	append<int8_t>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(int16_t value)
{
	append<int16_t>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(int32_t value)
{
	append<int32_t>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(int64_t value)
{
	append<int64_t>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(float value)
{
	append<float>(value);
	return *this;
}

ByteBuffer& ByteBuffer::operator<<(ByteBuffer& value)
{
	if (value.wpos() > 0)
		append(value.contents(), value.wpos());
	return *this;
}

// Hacky KO string flag - either it's a single byte length, or a double byte.
void ByteBuffer:: SByte()
{
	_doubleByte = false;
}

void ByteBuffer::DByte()
{
	_doubleByte = true;
}

uint8_t ByteBuffer::operator[](size_t pos)
{
	return read<uint8_t>(pos);
}

size_t ByteBuffer::rpos() const
{
	return _rpos;
}

size_t ByteBuffer::rpos(size_t rpos)
{
	_rpos = rpos;
	return _rpos;
}

size_t ByteBuffer::wpos() const
{
	return _wpos;
}

size_t ByteBuffer::wpos(size_t wpos)
{
	_wpos = wpos;
	return _wpos;
}

void ByteBuffer:: read(void* dest, size_t len)
{
	if (_rpos + len <= size())
		memcpy(dest, &_storage[_rpos], len);
	else // throw error();
		memset(dest, 0, len);
	_rpos += len;
}

void ByteBuffer::readString(std::string& dest)
{
	size_t len = 0;
	if (_doubleByte)
		len = read<uint16_t>();
	else
		len = read<uint8_t>();

	readString(dest, len);
}

void ByteBuffer::readString(std::string& dest, size_t len)
{
	dest.clear();
	dest.assign(len, '\0');

	if (_rpos + len <= size())
		read(&dest[0], len);
}

const std::vector<uint8_t>& ByteBuffer::storage() const
{
	return _storage;
}

std::vector<uint8_t>& ByteBuffer::storage()
{
	return _storage;
}

const uint8_t* ByteBuffer::contents() const
{
	return &_storage[0];
}

size_t ByteBuffer::size() const
{
	return _storage.size();
}

// one should never use resize
void ByteBuffer::resize(size_t newsize)
{
	_storage.resize(newsize);
	_rpos = 0;
	_wpos = size();
}

void ByteBuffer::sync_for_read()
{
	_rpos = 0;
	_wpos = size();
}

void ByteBuffer::reserve(size_t ressize)
{
	if (ressize > size())
		_storage.reserve(ressize);
}

// append to the end of buffer
void ByteBuffer::append(const void* src, size_t cnt)
{
	if (cnt == 0)
		return;

	// 10MB is far more than you'll ever need.
	assert(size() < 10000000);

	if (_storage.size() < _wpos + cnt)
		_storage.resize(_wpos + cnt);

	memcpy(&_storage[_wpos], src, cnt);
	_wpos += cnt;
}

void ByteBuffer::append(const ByteBuffer& buffer)
{
	if (buffer.size() > 0)
		append(buffer.contents(), buffer.size());
}

void ByteBuffer::append(const ByteBuffer& buffer, size_t len)
{
	assert(buffer.rpos() + len <= buffer.size());
	append(buffer.contents() + buffer.rpos(), len);
}

void ByteBuffer::readFrom(ByteBuffer& buffer, size_t len)
{
	assert(buffer.rpos() + len <= buffer.size());
	append(buffer.contents() + buffer.rpos(), len);
	buffer.rpos(buffer.rpos() + len);
}

void ByteBuffer::put(size_t pos, const void* src, size_t cnt)
{
	assert(pos + cnt <= size());
	memcpy(&_storage[pos], src, cnt);
}
