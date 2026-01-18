#include "pch.h"
#include "ByteBuffer.h"

#include <cassert>

// Ideally we shouldn't really be using macros for this kind've thing,
// but the compile-time benefits from having them implemented are largely worth it here.
// Doesn't really make sense to implement each of them manually.
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define IMPL_BYTEBUFFER_POD_TEMPLATE(type)                     \
	template <>                                                \
	void ByteBuffer::append<type>(type value)                  \
	{                                                          \
		append(&value, sizeof(value));                         \
	}                                                          \
	template <>                                                \
	void ByteBuffer::put<type>(size_t pos, type value)         \
	{                                                          \
		put(pos, &value, sizeof(value));                       \
	}                                                          \
	template <>                                                \
	ByteBuffer& ByteBuffer::operator<< <type>(type value)      \
	{                                                          \
		append<type>(value);                                   \
		return *this;                                          \
	}                                                          \
	template <>                                                \
	type ByteBuffer::read<type>(size_t pos) const              \
	{                                                          \
		/*assert(pos + sizeof(type) <= size());*/              \
		if (pos + sizeof(type) > size())                       \
			return {};                                         \
		return *reinterpret_cast<const type*>(&_storage[pos]); \
	}                                                          \
	template <>                                                \
	type ByteBuffer::read<type>()                              \
	{                                                          \
		type r  = read<type>(_rpos);                           \
		_rpos  += sizeof(type);                                \
		return r;                                              \
	}

IMPL_BYTEBUFFER_POD_TEMPLATE(float)
IMPL_BYTEBUFFER_POD_TEMPLATE(bool)
IMPL_BYTEBUFFER_POD_TEMPLATE(char)
IMPL_BYTEBUFFER_POD_TEMPLATE(uint8_t)
IMPL_BYTEBUFFER_POD_TEMPLATE(uint16_t)
IMPL_BYTEBUFFER_POD_TEMPLATE(uint32_t)
IMPL_BYTEBUFFER_POD_TEMPLATE(uint64_t)
IMPL_BYTEBUFFER_POD_TEMPLATE(int8_t)
IMPL_BYTEBUFFER_POD_TEMPLATE(int16_t)
IMPL_BYTEBUFFER_POD_TEMPLATE(int32_t)
IMPL_BYTEBUFFER_POD_TEMPLATE(int64_t)

#undef IMPL_BYTEBUFFER_POD_TEMPLATE
// NOLINTEND(cppcoreguidelines-macro-usage)

template <>
std::string ByteBuffer::read<std::string>(size_t pos) const
{
	std::string r;
	readString(pos, r);
	return r;
}

template <>
std::string ByteBuffer::read<std::string>()
{
	std::string r;
	readString(r);
	return r;
}

ByteBuffer::ByteBuffer() : _doubleByte(true), _rpos(0), _wpos(0)
{
	_storage.reserve(DEFAULT_SIZE);
}

ByteBuffer::ByteBuffer(size_t res) : _doubleByte(true), _rpos(0), _wpos(0)
{
	_storage.reserve(res <= 0 ? DEFAULT_SIZE : res);
}

ByteBuffer::ByteBuffer(const ByteBuffer& buf) :
	_doubleByte(true), _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage)
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

ByteBuffer& ByteBuffer::operator<<(ByteBuffer& value)
{
	if (value.wpos() > 0)
		append(value.contents(), value.wpos());
	return *this;
}

// Hacky KO string flag - either it's a single byte length, or a double byte.
void ByteBuffer::SByte()
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

bool ByteBuffer::read(size_t pos, void* dest, size_t len) const
{
	if (pos + len > size())
		return false;

	memcpy(dest, &_storage[_rpos], len);
	return true;
}

bool ByteBuffer::read(void* dest, size_t len)
{
	if (!read(_rpos, dest, len))
		return false;

	_rpos += len;
	return true;
}

bool ByteBuffer::readString(size_t pos, std::string& dest) const
{
	dest.clear();

	if (_doubleByte)
	{
		uint16_t len = 0;
		if (!read(pos, &len, sizeof(uint16_t)))
			return false;

		pos += sizeof(uint16_t);
		dest.assign(len, '\0');
		return read(pos, &dest[0], len);
	}
	else
	{
		uint8_t len = 0;
		if (!read(pos, &len, sizeof(uint8_t)))
			return false;

		pos += sizeof(uint8_t);
		dest.assign(len, '\0');
		return read(pos, &dest[0], len);
	}
}

bool ByteBuffer::readString(size_t pos, std::string& dest, size_t len) const
{
	dest.clear();
	dest.assign(len, '\0');

	if (pos + len > size())
		return false;

	return read(pos, &dest[0], len);
}

bool ByteBuffer::readString(std::string& dest)
{
	dest.clear();

	if (_doubleByte)
	{
		uint16_t len = 0;
		if (!read(&len, sizeof(uint16_t)))
			return false;

		dest.assign(len, '\0');
		return read(&dest[0], len);
	}
	else
	{
		uint8_t len = 0;
		if (!read(&len, sizeof(uint8_t)))
			return false;

		dest.assign(len, '\0');
		return read(&dest[0], len);
	}
}

bool ByteBuffer::readString(std::string& dest, size_t len)
{
	dest.clear();
	dest.assign(len, '\0');
	return read(&dest[0], len);
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

	const auto& bufferStorage = buffer.storage();
	append(&bufferStorage[buffer.rpos()], len);
}

void ByteBuffer::readFrom(ByteBuffer& buffer, size_t len)
{
	assert(buffer.rpos() + len <= buffer.size());

	const auto& bufferStorage = buffer.storage();
	append(&bufferStorage[buffer.rpos()], len);
	buffer.rpos(buffer.rpos() + len);
}

void ByteBuffer::put(size_t pos, const void* src, size_t cnt)
{
	assert(pos + cnt <= size());
	memcpy(&_storage[pos], src, cnt);
}
