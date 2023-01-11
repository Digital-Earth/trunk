/******************************************************************************
pyxtree_utils.cpp

begin		: 2007-10-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "pyxtree_utils.h"

namespace
{

struct AutoTest
{
	AutoTest()
	{
		BasicPYXTree<char> pt;

		pt[5][0][4] = 'C';
		pt[5] = 'A';
		pt[5][0] = 'B';
		pt[3] = 'H';
		pt[3][4] = 'I';
		pt = 'Z';
		pt[5][2] = 'D';
		pt[5][2][6] = 'F';
		pt[5][2][1] = 'E';

		BasicPYXTree<char>* q;
		q = queryPTree(&pt, Addr("3"));
		assert(q && *q == 'H');
		q = queryPTree(&pt, Addr("34"));
		assert(q && *q == 'I');
		q = queryPTree(&pt, Addr("5"));
		assert(q && *q == 'A');
		q = queryPTree(&pt, Addr("50"));
		assert(q && *q == 'B');
		q = queryPTree(&pt, Addr("504"));
		assert(q && *q == 'C');
		q = queryPTree(&pt, Addr("52"));
		assert(q && *q == 'D');
		q = queryPTree(&pt, Addr("521"));
		assert(q && *q == 'E');
		q = queryPTree(&pt, Addr("526"));
		assert(q && *q == 'F');
		assert(!queryPTree((BasicPYXTree<char>*)0, Addr("504")));
		assert(!queryPTree(&pt, Addr("2")));
		assert(!queryPTree(&pt, Addr("32")));
		assert(!queryPTree(&pt, Addr("342")));
/*
0
+-3
| \-4
\-5
  +-0
  | \-4
  \-2
    +-1
    \-6
	*/
		std::string str = pyxtreeToString(pt);
	}
} autoTest;

}
