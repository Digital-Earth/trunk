#ifndef PYXIS__UTILITY__WIRE_BUFFER_H
#define PYXIS__UTILITY__WIRE_BUFFER_H
/******************************************************************************
wire_buffer.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis\utility\memory_manager.h"

// standard includes
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/scoped_array.hpp>

class PYXLIB_DECL PYXConstBuffer : public PYXObject, protected ObjectMemoryUsageCounter<PYXConstBuffer>
{
private: //Members
	size_t m_size;
	boost::scoped_array<unsigned char> m_buffer;

public: // Create & Constructors
	static PYXPointer<PYXConstBuffer> create(size_t length)
	{
		return PYXNEW(PYXConstBuffer,length);
	}

	static PYXPointer<PYXConstBuffer> create(const char * data,size_t length)
	{
		return PYXNEW(PYXConstBuffer,data,length);
	}

	static PYXPointer<PYXConstBuffer> create(const std::string & str)
	{
		return PYXNEW(PYXConstBuffer,str);
	}

	//create a const buffer but the data is not initialized
	PYXConstBuffer(size_t length) : m_size(length)
	{
		if (m_size>0)
		{
			m_buffer.reset(new unsigned char[m_size]);
			consumeMemory(m_size);
		}
	}

	PYXConstBuffer(const  char * data,size_t length) : m_size(length)
	{
		if (m_size>0)
		{
			m_buffer.reset(new unsigned char[m_size]);
			consumeMemory(m_size);
			memcpy(m_buffer.get(),data,m_size);
		}
	}

	PYXConstBuffer(const std::string & str) : m_size(str.size())
	{
		if (m_size>0)
		{
			m_buffer.reset(new unsigned char[m_size]);
			consumeMemory(m_size);
			memcpy(m_buffer.get(),str.c_str(),m_size);
		}
	}

	virtual ~PYXConstBuffer()
	{
		releaseMemory(m_size);
	}

private:
	PYXConstBuffer(const PYXConstBuffer & other) {};
	const PYXConstBuffer & operator =(const PYXConstBuffer & other) { return *this; };

public : //Methods
	const unsigned char * begin() const { return m_buffer.get(); }
	const unsigned char * end() const { return &m_buffer[m_size]; }
	size_t size() const { return m_size; }

	const unsigned char & operator[](size_t pos) const { 
		assert(pos<size());
		return m_buffer[pos]; 
	}
};


class PYXLIB_DECL PYXConstBufferSlice : public PYXObject
{
private: // Members
	PYXPointer<PYXConstBuffer> m_buffer;
	const unsigned char * m_begin;
	const unsigned char * m_end;

public: // Creators & Constructors
	static PYXPointer<PYXConstBufferSlice> create(const PYXPointer<PYXConstBuffer> & buffer)
	{
		return PYXNEW(PYXConstBufferSlice,buffer);
	}

	static PYXPointer<PYXConstBufferSlice> create(const PYXPointer<PYXConstBuffer> & buffer,size_t begin,size_t end)
	{
		return PYXNEW(PYXConstBufferSlice,buffer,begin,end);
	}

	static PYXPointer<PYXConstBufferSlice> create(const PYXConstBufferSlice & slice)
	{
		return PYXNEW(PYXConstBufferSlice,slice);
	}

	static PYXPointer<PYXConstBufferSlice> create(const PYXConstBufferSlice & slice,size_t begin,size_t end)
	{
		return PYXNEW(PYXConstBufferSlice,slice,begin,end);
	}

	PYXConstBufferSlice(const PYXPointer<PYXConstBuffer> & buffer) : m_buffer(buffer), m_begin(m_buffer->begin()),m_end(m_buffer->end())
	{
	}

	PYXConstBufferSlice(const PYXPointer<PYXConstBuffer> & buffer,size_t begin,size_t end) : m_buffer(buffer), m_begin(m_buffer->begin()+begin), m_end(m_buffer->begin()+end)
	{
		assert(m_buffer->end() >= m_begin);
		assert(m_buffer->end() >= m_end);
		assert(m_begin <= m_end);
	}

	PYXConstBufferSlice() : m_begin(0), m_end(0)
	{
	}

	PYXConstBufferSlice(const PYXConstBufferSlice & slice) : m_buffer(slice.m_buffer), m_begin(slice.begin()), m_end(slice.end())
	{
	}

	PYXConstBufferSlice(const PYXConstBufferSlice & slice,size_t begin,size_t end) : m_buffer(slice.m_buffer), m_begin(slice.begin()+begin), m_end(slice.begin()+end)
	{
		assert(slice.end() >= m_begin);
		assert(slice.end() >= m_end);
		assert(m_begin <= m_end);
	}

	PYXConstBufferSlice(const std::string & str) : m_buffer(PYXConstBuffer::create(str)), m_begin(m_buffer->begin()), m_end(m_buffer->end())
	{
	}

	PYXConstBufferSlice & operator=(const PYXConstBufferSlice & slice) 
	{
		m_buffer = slice.m_buffer;
		m_begin = slice.m_begin;
		m_end = slice.m_end;
		return *this;
	}

public: //Methods
	const unsigned char * begin() const { return m_begin; }
	const unsigned char * end() const { return m_end; }
	size_t size() const { return m_end-m_begin; }

	const unsigned char & operator[](size_t pos) const { 
		assert(pos<size());
		return *(m_begin+pos); 
	}

	operator std::string() const {
		if (size()==0)
		{
			return std::string();
		}
		return std::string(m_begin,m_end);
	}

	PYXConstBufferSlice slice(size_t begin,size_t end)
	{
		return PYXConstBufferSlice(*this,begin,end);
	}

	PYXConstBufferSlice slice(const unsigned char * begin,size_t length)
	{
		return PYXConstBufferSlice(*this,begin-m_begin,begin-m_begin+length);
	}
};

///////////////////////////////////////////////////////////////////////////////
// PYXWireBuffer
///////////////////////////////////////////////////////////////////////////////


class PYXLIB_DECL PYXWireBuffer
{
public:
	enum setPosOption {
		expandIfNeeded,
	};

	virtual ~PYXWireBuffer() {}

	virtual void write(unsigned char value) = 0;
	virtual void write(const PYXConstBufferSlice & slice) = 0;
	virtual void write(unsigned char * buffer,size_t length) = 0;

	virtual void read(unsigned char & value) = 0;
	virtual PYXConstBufferSlice read(size_t length) = 0;
	virtual void read(unsigned char * buffer,size_t length) = 0;

	virtual PYXPointer<PYXConstBufferSlice> getBuffer() const = 0;

	virtual size_t pos() const = 0;
	virtual size_t size() const = 0;
	virtual void setPos(size_t pos) = 0;
	virtual void setPos(size_t pos,setPosOption option) = 0;

public:
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,unsigned char value);
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,int value);
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,unsigned int value);
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,float value);
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,double value);
	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const std::string & value);

	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,unsigned char & value);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,int & value);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,unsigned int & value);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,float & value);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,double & value);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,std::string & value);

public:
	static void test();
};


class PYXLIB_DECL PYXStringWireBuffer : public PYXWireBuffer
{
private:
	mutable std::stringstream m_stream;	

public:
	PYXStringWireBuffer(){}
	PYXStringWireBuffer(size_t length): m_stream(std::string(length,0))
	{
		setPos(0);
	}
	PYXStringWireBuffer(const std::string & data): m_stream(data)
	{
		setPos(0);
	}
	PYXStringWireBuffer(const unsigned char * buffer,size_t length) : m_stream(std::string((const char*)buffer,length))
	{
		setPos(0);
	}

	std::string toString() const { return m_stream.str(); }
	void copyToString(std::string & string) { string = m_stream.str(); }
	void copyFromString(const std::string & string) { m_stream.str(string); setPos(0); }

public:
	virtual void write(unsigned char value)
	{
		m_stream.write((char*)&value,1);
		m_stream.seekg(m_stream.tellp(),std::ios_base::beg);
	}

	virtual void write(const PYXConstBufferSlice & slice)
	{
		if (slice.size()==0)
		{
			return;
		}
		m_stream.write((const char *)slice.begin(),slice.size());
		m_stream.seekg(m_stream.tellp(),std::ios_base::beg);
	}

	virtual void write(unsigned char * buffer,size_t length)
	{
		m_stream.write((char *)buffer,length);
		m_stream.seekg(m_stream.tellp(),std::ios_base::beg);
	}

	virtual void read(unsigned char & value)
	{
		m_stream.read((char *)&value,1);	
		m_stream.seekp(m_stream.tellg(),std::ios_base::beg);
	}

	virtual PYXConstBufferSlice read(size_t length)
	{
		PYXPointer<PYXConstBuffer> constBuffer = PYXConstBuffer::create(length);
		m_stream.read((char*)(const_cast<unsigned char *>(constBuffer->begin())),length);	
		m_stream.seekp(m_stream.tellg(),std::ios_base::beg);
		return PYXConstBufferSlice(constBuffer,0,length);
	}

	virtual void read(unsigned char * buffer,size_t length)
	{
		m_stream.read((char*)buffer,length);	
		m_stream.seekp(m_stream.tellg(),std::ios_base::beg);
	}

	virtual PYXPointer<PYXConstBufferSlice> getBuffer() const
	{
		return PYXConstBufferSlice::create(PYXConstBuffer::create(toString()));
	}

	virtual size_t size() const 
	{
		if ((int)m_stream.tellg() == -1)
		{
			return 0;
		}

		m_stream.seekg(0,std::ios_base::end); 
		size_t length = (size_t)m_stream.tellg();
		m_stream.seekg(m_stream.tellp(),std::ios_base::beg);

		return length;
	}

	virtual size_t pos() const { return (size_t)m_stream.tellg(); }

	virtual void setPos(size_t pos)
	{
		m_stream.seekg(pos,std::ios_base::beg);
		m_stream.seekp(pos,std::ios_base::beg);
	}

	virtual void setPos(size_t pos,setPosOption option)
	{
		if(option == expandIfNeeded)
		{
			int lengthDifference=size()-pos;
			if(lengthDifference <= 0)
			{
				//m_stream.seekg(0,std::ios_base::end);
				m_stream.seekp(0,std::ios_base::end);
				for(;lengthDifference<=0;lengthDifference++)
				{
					unsigned char value=0;

					m_stream.write((char *)&value,1);	
				}
			}
			setPos(pos);
		}
	}

	void clear()
	{
		m_stream.str(std::string());
	}
};

class PYXLIB_DECL PYXConstWireBuffer : public PYXWireBuffer
{
private:
	PYXPointer<PYXConstBufferSlice> m_slice;
	const unsigned char * m_current;

public:
	PYXConstWireBuffer(const PYXWireBuffer & buffer)
	{
		const PYXConstWireBuffer * other = dynamic_cast<const PYXConstWireBuffer *>(&buffer);

		if (other != 0)
		{
			m_slice = other->m_slice;
			m_current = other->m_current;
		}
		else
		{
			m_slice = buffer.getBuffer();
			m_current = m_slice->begin()+buffer.pos();
		}
	}


	PYXConstWireBuffer(const PYXConstBufferSlice & slice) : m_slice(PYXConstBufferSlice::create(slice)),m_current(slice.begin())
	{
	}

	PYXConstWireBuffer(const PYXPointer<PYXConstBufferSlice> & slice) : m_slice(slice),m_current(slice->begin())
	{
	}

	PYXConstWireBuffer(const std::string & str) : m_slice(PYXConstBufferSlice::create(PYXConstBuffer::create(str)))
	{
		m_current = m_slice->begin();
	}

	PYXConstWireBuffer(const char * data, size_t length) : m_slice(PYXConstBufferSlice::create(PYXConstBuffer::create(data,length)))
	{
		m_current = m_slice->begin();
	}

	PYXConstWireBuffer(const PYXConstWireBuffer & other) : m_slice(other.m_slice),m_current(other.m_current)
	{
	}

	PYXConstWireBuffer & operator = (const PYXConstWireBuffer & other)
	{
		m_slice = other.m_slice;
		m_current = other.m_current;

		return *this;
	}

	PYXConstWireBuffer & operator = (const PYXWireBuffer & buffer)
	{
		const PYXConstWireBuffer * other = dynamic_cast<const PYXConstWireBuffer *>(&buffer);

		if (other != 0)
		{
			m_slice = other->m_slice;
			m_current = other->m_current;
		}
		else
		{
			m_slice = buffer.getBuffer();
			m_current = m_slice->begin()+buffer.pos();
		}

		return *this;
	}

public:
	virtual void write(unsigned char value) 
	{
		assert(0 && "write to const buffer is not allowed");
	}
	virtual void write(const PYXConstBufferSlice & slice)
	{
		assert(0 && "write to const buffer is not allowed");
	}
	virtual void write(unsigned char * buffer,size_t length)
	{
		assert(0 && "write to const buffer is not allowed");
	}

	virtual void read(unsigned char & value)
	{
		value = *m_current;

		if (m_current < m_slice->end())
		{
			m_current++;
		}			
	}

	virtual PYXConstBufferSlice read(size_t length)
	{
		assert(m_current+length <= m_slice->end());
		PYXConstBufferSlice slice = m_slice->slice(m_current,length);
		m_current += length;
		return slice;
	}

	virtual void read(unsigned char * buffer,size_t length)
	{
		assert(m_current+length <= m_slice->end());
		memcpy(buffer,m_current,length);
		m_current += length;
	}

	virtual PYXPointer<PYXConstBufferSlice> getBuffer() const
	{
		return m_slice;
	}

	virtual size_t pos() const 
	{
		return m_current - m_slice->begin();
	}
	virtual size_t size() const
	{
		return m_slice->size();
	}
	virtual void setPos(size_t pos)
	{
		assert(pos<=size());
		m_current = m_slice->begin() + pos;
	}

	virtual void setPos(size_t pos,setPosOption option)
	{
		assert(pos<=size());
		m_current = m_slice->begin() + pos;
	}
};


PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,unsigned char value);
PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,int value);
PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,unsigned int value);
PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,float value);
PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,double value);
PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const std::string & value);

PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,unsigned char & value);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,int & value);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,unsigned int & value);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,float & value);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,double & value);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,std::string & value);

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXConstBufferSlice & value);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXConstBufferSlice & value);

template<typename TValue>
PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const std::vector<TValue> & vector)
{
	buffer << (int)vector.size();
	for(std::vector<TValue>::const_iterator it = vector.begin();it!= vector.end();++it)
	{
		buffer << *it;
	}
	return buffer;
}

template<typename TValue>
PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const std::deque<TValue> & deque)
{
	buffer << (int)deque.size();
	for(std::deque<TValue>::const_iterator it = deque.begin();it!= deque.end();++it)
	{
		buffer << *it;
	}
	return buffer;
}


template<typename TValue>
PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const std::list<TValue> & list)
{
	buffer << (int)list.size();
	for(std::list<TValue>::const_iterator it = list.begin();it!= list.end();++it)
	{
		buffer << *it;
	}
	return buffer;
}

template<typename TKey,typename TValue>
PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const std::map<TKey,TValue> & map)
{
	buffer << (int)map.size();
	for(std::map<TKey,TValue>::const_iterator it = map.begin();it!= map.end();++it)
	{
		buffer << it->first << it->second;
	}
	return buffer;
}

template<typename TValue>
PYXWireBuffer & operator >>(PYXWireBuffer & buffer,std::vector<TValue> & vector)
{
	int count;
	buffer >> count;
	vector.resize(count);
	for(int i=0;i<count;++i)
	{
		buffer >> vector[i];
	}
	return buffer;
}

template<typename TValue>
PYXWireBuffer & operator >>(PYXWireBuffer & buffer,std::deque<TValue> & deque)
{
	int count;
	buffer >> count;
	deque.resize(count);
	for(int i=0;i<count;++i)
	{
		buffer >> deque[i];
	}
	return buffer;
}


template<typename TValue>
PYXWireBuffer & operator >>(PYXWireBuffer & buffer,std::list<TValue> & list)
{
	int count;
	buffer >> count;
	list.clear();

	TValue value;

	for(int i=0;i<count;++i)
	{
		buffer >> value;
		list.push_back(value);
	}
	return buffer;
}

template<typename TKey,typename TValue>
PYXWireBuffer & operator >>(PYXWireBuffer & buffer,std::map<TKey,TValue> & map)
{
	int count;
	buffer >> count;
	map.clear();

	std::pair<TKey,TValue> keyValue;

	for(int i=0;i<count;++i)
	{
		buffer >> keyValue.first >> keyValue.second;
		map.insert(keyValue);
	}
	return buffer;
}

///////////////////////////////////////////////////////////////////////////////
// PYXCompactInteger
///////////////////////////////////////////////////////////////////////////////

class PYXCompactInteger
{
private:
	int & m_value;

public:
	PYXCompactInteger(int & value) : m_value(value)
	{
	}

	friend PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCompactInteger & value);
	friend PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXCompactInteger & value);
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCompactInteger & value);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXCompactInteger & value);

#endif // guard
