#pragma once
#ifndef VIEW_MODEL__ADDR_H
#define VIEW_MODEL__ADDR_H
/******************************************************************************
addr.h

begin		: 2007-10-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// standard includes
#include <cassert>
#include <string>

/*!
Stores an address in bytes like this (using nibbles of 0-6 plus 7 (actually 0xf) as terminator):
(0,1) (2,3) (4,5) ... (38,39)

TODO could pack in groups of 3 (not 4) bits
*/
//! Simple PYXIS-like address.
template <int SZ>
class BasicAddr
{
public:

	enum
	{
		maxsize = SZ,
		maxn = SZ - 1
	};

public:

	// Do not use this class directly.
	struct proxy
	{
		proxy(unsigned char* buf, int n) : m_p(buf + n/2), m_bHi(!(n%2)) {}
		proxy& operator =(int n)
		{
			assert((0 <= n && n <= 7) || (n == 0xf));
			*m_p = m_bHi ? ((n<<4) | (*m_p)&0x0f) : ((*m_p)&0xf0 | n&0x0f);
			return *this;
		}
		operator int() const
		{
			return m_bHi ? ((*m_p)>>4) : ((*m_p)&0x0f);
		}
		unsigned char* m_p;
		bool m_bHi;
	};

public:

	BasicAddr()
	{
		set(0, 0xf);
	}

	// Assumes string is digits like "0123"
	explicit BasicAddr(const char* str)
	{
		int n = 0;
		while (*str)
		{
			assert(n <= maxn);
			assert('0' <= *str && *str <= '6');
			set(n++, *str++ - '0');
		}
		if (n <= maxn)
		{
			set(n, 0xf);
		}
	}

	void clear()
	{
		set(0, 0xf);
	}

	int operator [](int n) const
	{
		return !(n%2) ? (buf[n/2]>>4) : (buf[n/2]&0xf);
	}

	proxy operator [](int n)
	{
		return proxy(buf, n);
	}

	// Debugging aid.
	std::string toString() const
	{
		std::string str;
		int n = 0;
		while (get(n) != 0xf)
		{
			str.push_back('0' + get(n++));
		}
		return str;
	}

private:

	int get(int n) const
	{
		assert(n <= maxn);
		return (*this)[n];
	}

	void set(int n, int d)
	{
		// TODO optimize this with a direct computation
		assert(n <= maxn);
		(*this)[n] = d;
	}

private:

	unsigned char buf[maxsize/2];
};

typedef BasicAddr<40> Addr;

inline
bool operator ==(const Addr& lhs, const Addr& rhs)
{
	// TODO could likely get better performance than this
	int n = 0;
	while (n <= lhs.maxn)
	{
		if (lhs[n] != rhs[n])
		{
			return false;
		}
		if (lhs[n] == 0xf)
		{
			break;
		}
		++n;
	}
	return true;
}

inline
bool operator !=(const Addr& lhs, const Addr& rhs)
{
	// TODO could likely get better performance than this
	return !(lhs == rhs);
}

#endif
