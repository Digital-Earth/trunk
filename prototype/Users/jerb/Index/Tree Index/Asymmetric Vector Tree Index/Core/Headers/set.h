/* Copyright (c) 2007 Jason Erb

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

# if !defined(HEADERS__SET)
	# define HEADERS__SET

	# include "Boolean.h"
	# include "Core.h"

	typedef int unsigned tiuSet;

	tiuSet Set_fiuEmpty(void);

	tbBoolean Set_fbAdd(
		REGISTER tiuSet riuSet[mReferenceConst(1)],
		REGISTER int unsigned const iuElement
	);
	
	tbBoolean Set_fbRemove(
		REGISTER tiuSet riuSet[mReferenceConst(1)],
		REGISTER int unsigned const iuElement
	);

	tbBoolean Set_fbPop(
		REGISTER tiuSet riuSet[mReferenceConst(1)],
		REGISTER int unsigned * const piuElement
	);

	tbBoolean Set_fbContains(
		REGISTER tiuSet const iuSet,
		REGISTER int unsigned const iuElement
	);
	
	int unsigned Set_fiuCount(
		REGISTER tiuSet iuSet
	);

	tbBoolean Set_fbTest(void);

# endif
