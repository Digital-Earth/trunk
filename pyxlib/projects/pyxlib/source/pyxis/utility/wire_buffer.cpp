/******************************************************************************
wire_buffer.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "wire_buffer.h"
#include "bit_utils.h"
#include "tester.h"
#include "exceptions.h"

// standard includes
#include <cassert>

Tester<PYXWireBuffer> tester();
void PYXWireBuffer::test()
{
	PYXStringWireBuffer buffer;

	buffer<<(unsigned char)0;
	buffer<<(unsigned char)3;
	buffer<<(unsigned char)0;

	buffer.setPos(4,expandIfNeeded);
	buffer<<(unsigned char)3;
	buffer.setPos(10,expandIfNeeded);
	TEST_ASSERT(buffer.size()==11);
	buffer<<10;
	buffer.setPos(4);
	unsigned char read;
	int intRead;
	buffer>> read;
	TEST_ASSERT(read==3&&"WireBuffer fails");
	buffer.setPos(10);
	buffer >> intRead;
	TEST_ASSERT(intRead==10&&"WireBuffer fails");

	PYXStringWireBuffer copy;
	copy.copyFromString(buffer.toString());
	copy.setPos(10);
	copy >> intRead;
	TEST_ASSERT(intRead==10&&"WireBuffer copy fails");

	std::vector<int> numbers;
	numbers.push_back(0);
	numbers.push_back(1);
	numbers.push_back(10);
	numbers.push_back(100);
	numbers.push_back(1000);
	numbers.push_back(10000);
	numbers.push_back(100000);
	numbers.push_back(-1);
	numbers.push_back(-10);
	numbers.push_back(-100);
	numbers.push_back(-1000);
	numbers.push_back(-10000);
	numbers.push_back(-100000);
	numbers.push_back(127);
	numbers.push_back(128);
	numbers.push_back(63);
	numbers.push_back(64);
	numbers.push_back(65);
	numbers.push_back(66);
	numbers.push_back(-127);
	numbers.push_back(-128);
	numbers.push_back(-63);
	numbers.push_back(-64);
	numbers.push_back(-65);
	numbers.push_back(-66);

	buffer.clear();
	for(std::vector<int>::iterator it = numbers.begin(); it != numbers.end(); ++it)
	{
		buffer << PYXCompactInteger(*it);
	}
	buffer.setPos(0);
	for(std::vector<int>::iterator it = numbers.begin(); it != numbers.end(); ++it)
	{
		int v;
		buffer >> PYXCompactInteger(v);
		TEST_ASSERT(*it == v);
	}
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,unsigned char value)
{
	buffer.write(value);
	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,int value)
{
	const size_t size = sizeof(int);

	//TODO: this is not network portable
	buffer.write((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,unsigned int value)
{
	const size_t size = sizeof(unsigned int);

	//TODO: this is not network portable
	buffer.write((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,float value)
{
	const size_t size = sizeof(float);

	//TODO: this is not network portable
	buffer.write((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,double value)
{
	const size_t size = sizeof(double);

	//TODO: this is not network portable
	buffer.write((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const std::string & value)
{	
	size_t stringSize = value.size();
	unsigned char prefix[5];
	size_t prefixSize=0;

	if (value.size() < 128)
	{
		prefixSize=1;
		prefix[0]=(unsigned char)stringSize;
	}
	else if (value.size() < 256*64)
	{
		prefixSize=2;
		prefix[0]=(unsigned char)(stringSize >> 8)+128;
		prefix[1]=(unsigned char)(stringSize & 0xFF);
	}
	else
	{
		prefixSize=5;
		prefix[0]=255;
		memcpy(&prefix[1],&stringSize,sizeof(int));
	}

	size_t size = stringSize+prefixSize;

	buffer.write((unsigned char *)prefix,prefixSize);
	buffer.write((unsigned char *)value.c_str(),stringSize);

	return buffer;
}


PYXWireBuffer & operator >>(PYXWireBuffer & buffer,unsigned char & value)
{
	buffer.read(value);
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,int & value)
{
	const size_t size = sizeof(int);

	//TODO: this is not network portable
	buffer.read((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,unsigned int & value)
{
	const size_t size = sizeof(unsigned int);

	//TODO: this is not network portable
	buffer.read((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,float & value)
{
	const size_t size = sizeof(float);

	//TODO: this is not network portable
	buffer.read((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,double & value)
{
	const size_t size = sizeof(double);

	//TODO: this is not network portable
	buffer.read((unsigned char *)&value,size);
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,std::string & value)
{
	size_t stringSize = 0;

	unsigned char prefix0,prefix1;

	buffer.read(prefix0);

	if (prefix0 == 0xFF)
	{
		int size;
		buffer.read((unsigned char*)&size,sizeof(int));
		stringSize = (size_t)size;
	}
	else if (prefix0 >= 128)
	{
		buffer.read(prefix1);
		stringSize = prefix1 + ((prefix0 & 0x7F)<<8);
	}
	else
	{
		stringSize = prefix0;
	}

	if (stringSize<256)
	{
		//small string - void memory allocation - use the stack for reading the string out of the buffer
		char buff[256];
		buffer.read((unsigned char *)buff,stringSize);
		value.assign(buff,stringSize);
	}
	else
	{
		//this is a large string - create a buffer on memory
		boost::scoped_array<char> buff(new char[stringSize]);
		buffer.read((unsigned char *)buff.get(),stringSize);
		value.assign(buff.get(),stringSize);
	}

	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXConstBufferSlice & value)
{
	size_t stringSize = value.size();
	unsigned char prefix[5];
	size_t prefixSize=0;

	if (value.size() < 128)
	{
		prefixSize=1;
		prefix[0]=(unsigned char)stringSize;
	}
	else if (value.size() < 256*64)
	{
		prefixSize=2;
		prefix[0]=(unsigned char)(stringSize >> 8)+128;
		prefix[1]=(unsigned char)(stringSize & 0xFF);
	}
	else
	{
		prefixSize=5;
		prefix[0]=255;
		memcpy(&prefix[1],&stringSize,sizeof(int));
	}

	size_t size = stringSize+prefixSize;

	buffer.write((unsigned char *)prefix,prefixSize);
	buffer.write((unsigned char *)value.begin(),stringSize);

	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXConstBufferSlice & value)
{
	size_t sliceSize = 0;

	unsigned char prefix0,prefix1;

	buffer.read(prefix0);

	if (prefix0 == 0xFF)
	{
		int size;
		buffer.read((unsigned char*)&size,sizeof(int));
		sliceSize = (size_t)size;
	}
	else if (prefix0 >= 128)
	{
		buffer.read(prefix1);
		sliceSize = prefix1 + ((prefix0 & 0x7F)<<8);
	}
	else
	{
		sliceSize = prefix0;
	}

	value = buffer.read(sliceSize);
	return buffer;
}


PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCompactInteger & value)
{
	unsigned char localBuffer[10];
	int location=0;

	long v = value.m_value;
	if (v<0)
	{
		v=(-v)*2-1;	
	}
	else
	{
		v<<=1;
	}
	while (v>127)
	{
		localBuffer[location] = BitUtils::knBit8 | ( (unsigned char) v % 128);
		location++;
		v >>= 7;
	}
	localBuffer[location] = (unsigned char) v;
	location++;

	buffer.write(localBuffer,location);

	return buffer;
}


PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXCompactInteger & value)
{
	long v = 0;
	unsigned char c;
	int offset = 0;
	buffer.read(c);

	while (c>127)
	{
		v |= (c&0x7F)<<offset;
		buffer.read(c);
		offset+=7;
	}
	v |= (c&0x7F)<<offset;

	if (v%2==0)
	{
		value.m_value = v>>1;
	}
	else
	{
		value.m_value = -(v>>1)-1;
	}

	return buffer;
}
