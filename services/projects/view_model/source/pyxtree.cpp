/******************************************************************************
pyxtree.cpp

begin		: 2007-10-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "pyxtree.h"

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

		assert(pt[5][0][4] == 'C');
		assert(pt[5] == 'A');
		assert(pt[5][0] == 'B');
		assert(pt[3] == 'H');
		assert(pt[3][4] == 'I');
		assert(pt == 'Z');
		assert(pt[5][2] == 'D');
		assert(pt[5][2][6] == 'F');
		assert(pt[5][2][1] == 'E');

		pt[5][0].erase(4);
		pt[5].erase(2);

		assert(!pt[5][0].getChild(4));
		assert(!pt[5].getChild(2));

		// TESTING SOME SIZES
		BasicPYXTree<short> pt2;
		BasicPYXTree<int> pt3;
		BasicPYXTree<void*> pt4;
		int n1 = sizeof(pt); // 8 (char fits in padding)
		int n2 = sizeof(pt2); // 8 (short fits in padding)
		int n3 = sizeof(pt3); // 12
		int n4 = sizeof(pt4); // 12
	}
} autoTest;

}
