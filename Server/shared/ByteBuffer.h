#pragma once

#include <vector>
#include <string>

class ByteBuffer
{
public:
	constexpr static size_t DEFAULT_SIZE = 32;
	bool m_doubleByte;

	ByteBuffer(): _rpos(0), _wpos(0), m_doubleByte(true) { _storage.reserve(DEFAULT_SIZE); }
	ByteBuffer(size_t res): _rpos(0), _wpos(0), m_doubleByte(true) { _storage.reserve(res <= 0 ? DEFAULT_SIZE : res); }
	ByteBuffer(const ByteBuffer &buf): _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage) { }
	virtual ~ByteBuffer() {}

	void clear() 
	{
		_storage.clear();
		_rpos = _wpos = 0;
	}

	template <typename T> void append(T value) { append((uint8_t *)&value, sizeof(value)); }
	template <typename T> void put(size_t pos,T value) { put(pos,(uint8_t *)&value, sizeof(value)); }

	// stream like operators for storing data
	ByteBuffer &operator<<(bool value) { append<char>((char)value); return *this; }

	// unsigned
	ByteBuffer &operator<<(uint8_t value)		{ append<uint8_t> (value); return *this; }
	ByteBuffer &operator<<(uint16_t value)	{ append<uint16_t>(value); return *this; }
	ByteBuffer &operator<<(uint32_t value)	{ append<uint32_t>(value); return *this; }
	ByteBuffer &operator<<(uint64_t value)	{ append<uint64_t>(value); return *this; }
	// signed as in 2e complement
	ByteBuffer &operator<<(int8_t value)		{ append<int8_t> (value); return *this; }
	ByteBuffer &operator<<(int16_t value)		{ append<int16_t>(value); return *this; }
	ByteBuffer &operator<<(int32_t value)		{ append<int32_t>(value); return *this; }
	ByteBuffer &operator<<(int64_t value)		{ append<int64_t>(value); return *this; }
	ByteBuffer &operator<<(float value)		{ append<float>(value); return *this; }

	ByteBuffer &operator<<(ByteBuffer &value)
	{
		if (value.wpos())
			append(value.contents(), value.wpos());
		return *this;
	}

	// stream like operators for reading data
	ByteBuffer &operator>>(bool &value)		{ value = read<char>() > 0 ? true : false; return *this; }
	// unsigned
	ByteBuffer &operator>>(uint8_t &value)	{ value = read<uint8_t>(); return *this; }
	ByteBuffer &operator>>(uint16_t &value)	{ value = read<uint16_t>(); return *this; }
	ByteBuffer &operator>>(uint32_t &value)	{ value = read<uint32_t>(); return *this; }
	ByteBuffer &operator>>(uint64_t &value)	{ value = read<uint64_t>(); return *this; }
	// signed as in 2e complement
	ByteBuffer &operator>>(int8_t &value)		{ value = read<int8_t>(); return *this; }
	ByteBuffer &operator>>(int16_t &value)	{ value = read<int16_t>(); return *this; }
	ByteBuffer &operator>>(int32_t &value)	{ value = read<int32_t>(); return *this; }
	ByteBuffer &operator>>(int64_t &value)	{ value = read<int64_t>(); return *this; }
	ByteBuffer &operator>>(float &value)	{ value = read<float>(); return *this; }

	// Hacky KO string flag - either it's a single byte length, or a double byte.
	void SByte() { m_doubleByte = false; }
	void DByte() { m_doubleByte = true; }

	ByteBuffer &operator<<(const std::string &value)  { *this << value.c_str(); return *this; }
	ByteBuffer &operator<<(std::string &value) { *this << value.c_str(); return *this; }
	ByteBuffer &operator<<(const char *str) 
	{
		uint16_t len = (uint16_t)strlen(str);
		if (m_doubleByte)
			append((uint8_t*)&len, 2);
		else
			append((uint8_t*)&len, 1);
		append((uint8_t *)str, len);
		return *this;
	}	
	ByteBuffer &operator<<(char *str)  { *this << (const char*)str; return *this; }

	ByteBuffer &operator>>(std::string& value) 
	{
		uint16_t len;
		value.clear();
		if (m_doubleByte)
			len = read<uint16_t>();
		else
			len = read<uint8_t>();

		if (_rpos + len <= size())
		{
			for (uint16_t i = 0; i < len; i++)
				value.push_back(read<char>());
		}
		return *this;
	}

	uint8_t operator[](size_t pos) { return read<uint8_t>(pos); }

	size_t rpos() const { return _rpos; };
	size_t rpos(size_t rpos) { return _rpos = rpos; };
	size_t wpos() const { return _wpos; };
	size_t wpos(size_t wpos) { return _wpos = wpos; };

	template <typename T>
	T read() 
	{
		T r = read<T>(_rpos);
		_rpos += sizeof(T);
		return r;
	}

	template <>
	std::string read() 
	{
		std::string r;
		readString(r);
		return r;
	}

	template <typename T>
	T read(size_t pos) const 
	{
		//ASSERT(pos + sizeof(T) <= size());
		if (pos + sizeof(T) > size())
			return (T)0;
		return *((T*)&_storage[pos]);
	}

	void read(void *dest, size_t len) 
	{
		if (_rpos + len <= size()) 
			memcpy(dest, &_storage[_rpos], len);
		else // throw error();
			memset(dest, 0, len);
		_rpos += len;
	}

	void readString(std::string& dest)
	{
		size_t len = 0;
		if (m_doubleByte)
			len = read<uint16_t>();
		else
			len = read<uint8_t>();

		readString(dest, len);
	}

	void readString(std::string& dest, size_t len)
	{
		dest.clear();
		dest.assign(len, '\0');

		if (_rpos + len <= size())
			read(&dest[0], len);
	}

	const std::vector<uint8_t>& storage() const
	{
		return _storage;
	}

	std::vector<uint8_t>& storage()
	{
		return _storage;
	}

	const uint8_t* contents() const
	{
		return &_storage[0];
	}

	size_t size() const
	{
		return _storage.size();
	}

	// one should never use resize
	void resize(size_t newsize) 
	{
		_storage.resize(newsize);
		_rpos = 0;
		_wpos = size();
	}

	void sync_for_read()
	{
		_rpos = 0;
		_wpos = size();
	}

	void reserve(size_t ressize)  { if (ressize > size()) _storage.reserve(ressize); };

	// append to the end of buffer
	void append(const std::string& str) { append((uint8_t *)str.c_str(),str.size() + 1); }
	void append(const char *src, size_t cnt) { return append((const uint8_t *)src, cnt); }
	void append(const void *src, size_t cnt)
	{
		if (!cnt)
			return;

		// 10MB is far more than you'll ever need.
		ASSERT(size() < 10000000);

		if (_storage.size() < _wpos + cnt)
			_storage.resize(_wpos + cnt);

		memcpy(&_storage[_wpos], src, cnt);
		_wpos += cnt;
	}

	void append(const ByteBuffer& buffer) { if (buffer.size() > 0) append(buffer.contents(), buffer.size()); }
	void append(const ByteBuffer& buffer, size_t len)
	{
		ASSERT(buffer.rpos() + len <= buffer.size());
		append(buffer.contents() + buffer.rpos(), len);
	}

	void readFrom(ByteBuffer& buffer, size_t len)
	{
		ASSERT(buffer.rpos() + len <= buffer.size());
		append(buffer.contents() + buffer.rpos(), len);
		buffer.rpos(buffer.rpos() + len);
	}

	void put(size_t pos, const void *src, size_t cnt) 
	{
		ASSERT(pos + cnt <= size());
		memcpy(&_storage[pos], src, cnt);
	}

protected:
	// read and write positions
	size_t _rpos, _wpos;
	std::vector<uint8_t> _storage;
};
