#pragma once
#ifndef VIEW_MODEL__PYXTREE_UTILS_H
#define VIEW_MODEL__PYXTREE_UTILS_H
/******************************************************************************
pyxtree_utils.h

begin		: 2007-10-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "addr.h"
#include "pyxtree.h"

// standard includes
#include <iosfwd>
#include <sstream>
#include <string>

//! Return node at address in tree, creating any nodes along path that aren't present.
template <typename T>
inline
BasicPYXTree<T>* insertPTree(BasicPYXTree<T>* ptree, const Addr& addr)
{
	BasicPYXTree<T>* pt = ptree;

	assert(ptree);

	int n = 0;
	while (n <= addr.maxn)
	{
		int nDigit = addr[n];

		if (nDigit == 0xf)
		{
			break;
		}

		if (!pt)
		{
			break;
		}

		pt = &(*pt)[nDigit];
		++n;
	}

	return pt;
}

//! Return node at address in tree, creating any nodes along path that aren't present.
template <typename T>
inline
BasicPYXTree<T>* pyxtreeInsert(BasicPYXTree<T>& ptree, const Addr& addr)
{
	BasicPYXTree<T>* pt = &ptree;

	int n = 0;
	while (n <= addr.maxn)
	{
		int nDigit = addr[n];

		if (nDigit == 0xf)
		{
			break;
		}

		if (!pt)
		{
			break;
		}

		pt = &(*pt)[nDigit];
		++n;
	}

	return pt;
}

//! Return node at address in tree, or null if not present.
template <typename T>
inline
BasicPYXTree<T>* queryPTree(BasicPYXTree<T>* ptree, const Addr& addr)
{
	BasicPYXTree<T>* pt = ptree;

	int n = 0;
	while (n <= addr.maxn)
	{
		int nDigit = addr[n];

		if (nDigit == 0xf)
		{
			break;
		}

		if (!pt)
		{
			break;
		}

		pt = pt->getChild(nDigit);
		++n;
	}

	return pt;
}

//! Return ancestor node for address in tree.
template <typename T>
inline
BasicPYXTree<T>* descendPTree(BasicPYXTree<T>* ptree, const Addr& addr, Addr* pAddrOut = 0)
{
	BasicPYXTree<T>* pt = ptree;

	int n = 0;
	while (n <= addr.maxn)
	{
		int nDigit = addr[n];

		if (nDigit == 0xf)
		{
			break;
		}

		BasicPYXTree<T>* pChild = pt->getChild(nDigit);

		if (!pChild)
		{
			break;
		}

		pt = pChild;

		if (pAddrOut)
		{
			(*pAddrOut)[n] = nDigit;
		}

		++n;
	}

	if (pAddrOut)
	{
		(*pAddrOut)[n] = 0xf;
	}

	return pt;
}

template <typename T>
inline
std::ostream& pyxtreeNodeToStream(std::ostream& out, const BasicPYXTree<T>& ptree)
{
	return out;
}

/*!
This function prints the general form of a pyxtree as text. To specify how to
print a particular node within this form, provide an overloaded implementation
of the pyxtreeNodeToStream function for your type of pyxtree.
*/
//! Shows a pyxtree as a text tree.
template <typename T>
inline
std::ostream& pyxtreeToStream(std::ostream& out, const BasicPYXTree<T>& ptree, std::string strPrefix = "", int nDigit = 0)
{
	out << nDigit << ' ';
	pyxtreeNodeToStream(out, ptree);
	out << '\n';

	const int nChildCount = ptree.getFlagCount();
	for (int nChild = 0; nChild != nChildCount; ++nChild)
	{
		bool bLast = nChild == nChildCount - 1;
		int nChildDigit = ptree.digit(nChild);
		const BasicPYXTree<T>* pChild = ptree.getChildAtPos(nChild);

		out << strPrefix << (bLast ? "\\-" : "+-");

		strPrefix.append(bLast ? "  " : "| ");
		pyxtreeToStream(out, *pChild, strPrefix, nChildDigit);
		strPrefix.resize(strPrefix.size() - 2);
	}

	return out;
}

//! Shows a pyxtree as a text tree string.
template <typename T>
inline
std::string pyxtreeToString(const BasicPYXTree<T>& ptree)
{
	std::ostringstream out;
	pyxtreeToStream(out, ptree);
	return out.str();
}

#endif
