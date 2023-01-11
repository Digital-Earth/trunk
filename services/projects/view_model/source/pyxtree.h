#pragma once
#ifndef VIEW_MODEL__PYXTREE_H
#define VIEW_MODEL__PYXTREE_H
/******************************************************************************
pyxtree.h

begin		: 2007-10-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// standard includes
#include <cassert>
#include <utility>

/*!
Note that due to structure padding, we could have 32 (not just 8) bit flags to
mark children. When serializing, we need only store the single char though.

The flags tell us the structure of the tree. That is, whether there are any
children, how many they are, and exactly what their relative indexes are.

The children pointer tells us whether we actually have the (logical) children
(physically) present. That is, the tree might have three children, which aren't
loaded. We know this because the flags say the children should be there but the
pointer is null.

When the children are loaded, the pointer will not be null. Then we can access
the children (and their flags). We should never have children without having our
own flags, of course.

Now assume we have a way to denote another state. This is where we have not a
children pointer (to children loaded into memory), but rather a file offset on
disk (where we can find the children). We can store this extra info in the space
in the structure padding, because we don't need to store it on disk itself.

Flags are ordered such that bit 0 (lowest order) is child 0, bits 1-6 are
children 1-6, and bit 7 (highest order) is a special flag. Note that bit 7
would be the sign bit if it were a signed char.

Children are allocated together to save memory overhead. There is still padding
wasted. This could perhaps be recovered by reorganizing the data a little bit
and doing what may be construed as evil hacks to the pointers. We might save,
e.g., on the order of 15% at a guess (depends on a few factors including the
structure of the tree and what is stored in the tree).
*/
//! Simple PYXIS-like tree.
template <typename T>
class BasicPYXTree
{
public:

	static int countBits(unsigned char byte)
	{
		int nibblebits[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
		return nibblebits[byte>>4] + nibblebits[byte&0xf];
	}

public:

	BasicPYXTree() : flags(0), children(0) {}
	~BasicPYXTree() { delete[] children; }

	bool empty() const { return !getFlagCount(); }
	void clear() { flags = 0; delete[] children; }

	int getFlagCount() const { return countBits(flags); }
	bool getFlag(int n) const { return (flags & (1<<n)) != 0; }
	void setFlag(int n) { flags |= (1<<n); }
	void clearFlag(int n) { flags &= ~(1<<n); }
	unsigned char getFlags() const { return flags; }
	void setFlags(unsigned char flagByte) { flags = flagByte; }

	BasicPYXTree* getChild(int n) const { return (getFlag(n) && children) ? children + pos(n) : 0; }

	BasicPYXTree* getChildAtPos(int nPos) const { return getChild(digit(nPos)); }

	BasicPYXTree* allocChildren(int nCount)
	{
		assert(!children);
		children = new BasicPYXTree[nCount];
		return children;
	}

	BasicPYXTree* insert(int n)
	{
		assert(!getFlag(n));
		const int nPos = pos(n);
		if (children)
		{
			assert(getFlagCount());
			const int nCopyCount = getFlagCount();
			BasicPYXTree* newchildren = new BasicPYXTree[nCopyCount + 1];
			BasicPYXTree* src = children;
			BasicPYXTree* dst = newchildren;
			// TODO could optimize with memcpy or placement new or similar
			for (int nCopy = 0; nCopy != nCopyCount; ++nCopy)
			{
				if (nCopy == nPos)
				{
					++dst;
				}
				std::swap(*dst++, *src++);
				// TODO could possibly do just copy not swap, depending on type of T
			}
			std::swap(children, newchildren);
			delete[] newchildren;
		}
		else
		{
			assert(!getFlagCount());
			children = new BasicPYXTree[1];
		}
		setFlag(n);
		return children + nPos;
	}

	void erase(int n)
	{
		assert(getFlag(n));
		assert(children);
		const int nPos = pos(n);
		const int nCopyCount = getFlagCount() - 1;
		BasicPYXTree* newchildren = new BasicPYXTree[nCopyCount];
		BasicPYXTree* src = children;
		BasicPYXTree* dst = newchildren;
		// TODO could optimize with memcpy or placement new or similar
		for (int nCopy = 0; nCopy != nCopyCount; ++nCopy)
		{
			if (nCopy == nPos)
			{
				++src;
			}
			std::swap(*dst++, *src++);
		}
		std::swap(children, newchildren);
		delete[] newchildren;
		clearFlag(n);
	}

public:

	BasicPYXTree& operator[](int n)
	{
		if (getFlag(n))
		{
			return *getChild(n);
		}
		else
		{
			return *insert(n);
		}
	}

	T& operator *()
	{
		return val;
	}

	const T& operator *() const
	{
		return val;
	}

	T& data()
	{
		return val;
	}

	const T& data() const
	{
		return val;
	}

	BasicPYXTree& operator =(const T& v)
	{
		val = v;
		return *this;
	}

	bool operator ==(const T& v)
	{
		return val == v;
	}

public:

	int pos(int n) const
	{
		// TODO faster to use mask LUT or just compute ((1<<n)-1)?
		int mask[] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f };
		return countBits(flags&mask[n]);
	}

	int digit(int nPos) const
	{
		int f = flags;
		int n = 0;
		int p = -1;
		while (f)
		{
			if (f&0x1)
			{
				if (++p == nPos)
				{
					break;
				}
			}
			++n;
			f>>=1;
		}
		return n;
	}

	void swap(BasicPYXTree& rhs)
	{
		std::swap(children, rhs.children);
		std::swap(flags, rhs.flags);
		std::swap(val, rhs.val);
	}

private:

	BasicPYXTree* children;
	unsigned char flags;
	T val;
};

namespace std
{

template <typename T>
inline void swap(BasicPYXTree<T>& lhs, BasicPYXTree<T>& rhs)
{
	lhs.swap(rhs);
}

}

#endif
