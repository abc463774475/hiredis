#pragma once
#pragma execution_character_set ("utf_8")

#include <assert.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <string.h>

using namespace std;

namespace	sim{
	template<typename Exception,int DEFAULT_SIZE = 16>
	class ByteBuffer
	{
	public:
		typedef Exception			ByteBufferException;
		// constructor
		ByteBuffer(): _rpos(0), _wpos(0),_packType(0)
		{
			_storage.reserve(DEFAULT_SIZE);
		}
		
		// copy constructor
		ByteBuffer(const ByteBuffer &buf): _rpos(buf._rpos), _wpos(buf._wpos),
				_packType(buf._packType), _storage(buf._storage) 
		{

		}

		void clear()
		{
			_storage.clear();
			_rpos = _wpos = _packType = 0;
		}
private:
		template <typename T> void put(unsigned int pos,T value)
		{
			put(pos,(uint8_t*)&value,sizeof(value));
		}
public:
		ByteBuffer &operator<<(bool value)
		{
			char temp;
			if(value)
			{
				temp = 1;
			}
			else
			{
				temp = 0;
			}
			append<char>(temp);
			return *this;
		}

		ByteBuffer &operator<<(uint8_t value)
		{
			append<uint8_t>(value);
			return *this;
		}

		ByteBuffer &operator<<(uint16_t value)
		{
			append<uint16_t>(value);
			return *this;
		}

		ByteBuffer &operator<<(uint32_t value)
		{
			append<uint32_t>(value);
			return *this;
		}

		ByteBuffer &operator<<(uint64_t value)
		{
// 			char sz[32];
// 			sprintf (sz,"%I64d",value);
			//(*this) << string(sz);
			append<uint64_t>(value);
			return *this;
		}

		// signed as in 2e complement
		ByteBuffer &operator<<(int8_t value)
		{
			append<int8_t>(value);
			return *this;
		}

		ByteBuffer &operator<<(int16_t value)
		{
			append<int16_t>(value);
			return *this;
		}

		ByteBuffer &operator<<(int32_t value)
		{
			append<int32_t>(value);
			return *this;
		}

		ByteBuffer &operator<<(int64_t value)
		{
			append<int64_t>(value);
// 			char sz[32];
// 			sprintf (sz,"%I64d",value);
// 			(*this) << string(sz);
			return *this;
		}

		// floating points
		ByteBuffer &operator<<(float value)
		{
			append<float>(value);
			return *this;
		}

		ByteBuffer &operator<<(double value)
		{
			append<double>(value);
			return *this;
		}

		ByteBuffer &operator<<(const std::string &value)
		{
			string str = value ;
			uint16_t len = str.length();
			append<uint16_t>(len);
			append((uint8_t const *)str.c_str(), len);
			return *this;

// 			uint16_t_t len = value.length();
// 			append<uint16_t_t>(len);
// 			append((uint8_t_t const *)value.c_str(), len);
// 			return *this;
		}

		ByteBuffer &operator << (const ByteBuffer &buffer)
		{
			append(buffer);
			return *this;
		}

		ByteBuffer &operator>>(bool &value)
		{
			value = read<char>() > 0 ? true : false;
			return *this;
		}

		ByteBuffer &operator>>(uint8_t &value)
		{
			value = read<uint8_t>();
			return *this;
		}

		ByteBuffer &operator>>(uint16_t &value)
		{
			value = read<uint16_t>();
			return *this;
		}

		ByteBuffer &operator>>(uint32_t &value)
		{
			value = read<uint32_t>();
			return *this;
		}

		ByteBuffer &operator>>(uint64_t &value)
		{
			value = read<uint64_t>();
// 			string	str;
// 			(*this) >> str;
// 			sscanf(str.c_str(),"%I64d",&value);
			return *this;
		}

		//signed as in 2e complement
		ByteBuffer &operator>>(int8_t &value)
		{
			value = read<int8_t>();
			return *this;
		}

		ByteBuffer &operator>>(int16_t &value)
		{
			value = read<int16_t>();
			return *this;
		}

		ByteBuffer &operator>>(int32_t &value)
		{
			value = read<int32_t>();
			return *this;
		}

		ByteBuffer &operator>>(int64_t &value)
		{
			value = read<int64_t>();
// 			string	str;
// 			(*this) >> str;
// 			sscanf(str.c_str(),"%I64d",&value);
			return *this;
		}

		ByteBuffer &operator>>(float &value)
		{
			value = read<float>();
			return *this;
		}

		ByteBuffer &operator>>(double &value)
		{
			value = read<double>();
			return *this;
		}

		ByteBuffer &operator>>(std::string& str)
		{
			string value;
			uint16_t len = read<uint16_t>();
			if(_rpos  + len  > size())
				throw ByteBufferException(false, _rpos, len, size(),_packType);
			if ( _rpos +len != size())
			{
				value.insert(value.begin(),&_storage[_rpos],&_storage[_rpos]+len);			// 越界
			}
			else
			{
				value.insert(value.begin(),_storage.begin()+_rpos,_storage.end());
			}
			_rpos += len ;
			
			str = value ;
			return *this;
		}

		ByteBuffer &operator >> ( ByteBuffer &buffer)
		{
			// 长度
			int32_t	len = read<int32_t>();
			if ( len != 0 )
			{
				if (_rpos + len > size())
				{
					throw ByteBufferException(false,_rpos,len,size(),_packType);
				}
				else if ( _rpos + len <size() )
				{
					vector<uint8_t>	vTemp(&_storage[0]+_rpos,&_storage[0]+_rpos+len);
					buffer._storage = vTemp;
				}
				else
				{
					vector<uint8_t>	vTemp(_storage.begin()+_rpos,_storage.end());
					buffer._storage = vTemp;
				}
				//buffer._storage.insert(buffer._storage.begin(),&_storage[0]+_rpos,&_storage[0]+_rpos+len);
				buffer._wpos = len;
			}
			_rpos += len;
			return *this;
		}
		uint8_t operator[](unsigned int pos) const
		{
			return read<uint8_t>(pos);
		}

		unsigned int rpos() const { return _rpos; }

		unsigned int rpos(unsigned int rpos_)
		{
			_rpos = rpos_;
			return _rpos;
		}

		unsigned int wpos() const { return _wpos; }

		unsigned int wpos(unsigned int wpos_)
		{
			_wpos = wpos_;
			return _wpos;
		}
private:
		template<typename T>
		void read_skip() { read_skip(sizeof(T)); }
public:
		void read_skip(unsigned int skip)
		{
			if(_rpos + skip > size())
				throw ByteBufferException(false, _rpos, skip, size(),_packType);
			_rpos += skip;
		}
private:
		template <typename T> T read()
		{
			T r = read<T>(_rpos);
			_rpos += sizeof(T);
			return r;
		}

		template <typename T> T read(unsigned int pos) const
		{
			if(pos + sizeof(T) > size())
				throw ByteBufferException(false, pos, sizeof(T), size(),_packType);
			T val = *((T const*)&_storage[pos]);
			return val;
		}
public:
		void read(uint8_t *dest, unsigned int len)
		{
			if(_rpos  + len > size())
				throw ByteBufferException(false, _rpos, len, size(),_packType);
			memmove(dest, &_storage[_rpos], len);
			_rpos += len;
		}

		const uint8_t *contents() const 
		{
			if ( _storage.size() == 0 )
			{
				return NULL;
			}
			return &_storage[0]; 
		}
		const uint8_t *contents(unsigned int pos) const
		{
			if (pos >=size())
			{
				throw ByteBufferException(false, _rpos, 0, size(),_packType);
			}

			return &_storage[pos];
		}

		unsigned int size() const { return _storage.size(); }
		bool empty() const { return _storage.empty(); }

		void resize(unsigned int newsize)
		{
			_storage.resize(newsize);
		}

		void reserve(unsigned int ressize)
		{
			if (ressize > size())
				_storage.reserve(ressize);
		}

		void append(const std::string& str)
		{
			append((uint8_t const*)str.c_str(), str.size() + 1);
		}

		void append(const char *src, unsigned int cnt)
		{
			return append((const uint8_t *)src, cnt);
		}
private:
		/*template<class T> void append(const T *src, unsigned int cnt)
		{
			return append((const uint8_t_t *)src, cnt * sizeof(T));
		}*/
public:
		void append(const uint8_t *src, unsigned int cnt)
		{
			if (!cnt)
				return;
			if (_storage.size() < _wpos + cnt)
				_storage.resize(_wpos + cnt);
			memmove(&_storage[_wpos], src, cnt);
			_wpos += cnt;
		}

		void append(const ByteBuffer& buffer)
		{
			append<int32_t>(buffer._storage.size());
			if(buffer.wpos())
			{
				append(buffer.contents(), buffer.wpos());
			}
		}
		void put(unsigned int pos, const uint8_t *src, unsigned int cnt)
		{
			if(pos + cnt > size())
				throw ByteBufferException(true, pos, cnt, size(),_packType);
			memmove(&_storage[pos], src, cnt);
		}

	protected:
		template <typename T> void append(T value)
		{
			append((uint8_t *)&value, sizeof(value));
		}

	public:
		unsigned int _rpos, _wpos,_packType;
		std::vector<uint8_t> _storage;
	};
};
