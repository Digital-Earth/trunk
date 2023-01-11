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

# include "../Headers/file.h"
# include <limits.h>

static char const * frzcMode(
	REGISTER FileMode_te const eMode
) {
	mStrongAssert(FileMode_fbValid(eMode));

	{
		/* This array maps the File_teMode values to mode strings. */
		static char const * arzcMode[] = {
			"", "r", "w", "a",
			"", "rb", "wb", "ab",
			"", "r+", "w+", "a+",
			"", "rb+", "wb+", "ab+"
		};
		return arzcMode[eMode];
	}
}

static FILE * fpTemporary(void) {
	return tmpfile();
}

/*
File
*/

Boolean_tb File_fbOpen(
	REGISTER File_ts rsFile[mReferenceConst(1)],
	REGISTER const char rzcName[mReferenceConst(1)],
	REGISTER FileMode_te const eMode
) {
	mStrongAssert(rsFile);

	rsFile->pzcName = rzcName;
	rsFile->cMode = (char const)eMode;
	{
		REGISTER char const * const rzcMode = frzcMode(eMode);
		if (rsFile->pFile) {
			if (0 == (rsFile->pFile = freopen(rzcName, rzcMode, rsFile->pFile))) {
				rsFile->cMode = FileMode_iClosed;
				return 0;
			}
			return 1;
		}
		if (0 == (rsFile->pFile = fopen(rzcName, rzcMode))) {
			rsFile->cMode = FileMode_iClosed;
			return 0;
		}
	}
	return 1;
}

Boolean_tb File_fbOpenTemporary(
	REGISTER File_ts rsFile[mReferenceConst(1)]
) {
	mStrongAssert(rsFile);

	if (rsFile->pFile) return 0;
	rsFile->pzcName = 0;
	if (0 == (rsFile->pFile = fpTemporary())) {
		rsFile->cMode = FileMode_iClosed;
		return 0;
	}
	rsFile->cMode = FileMode_iWriteBinaryPlus;
	return 1;
}

char const * File_rzcTemporaryName(void) {
	return tmpnam(0);
}

Boolean_tb File_fbRead(
	REGISTER File_ts rsInput[mReferenceConst(1)],
	REGISTER char unsigned rcuChar[mReferenceConst(1)]
) {
	mStrongAssert(rsInput);
	assert(!File_fbClosed(rsInput));

	{
		REGISTER int const iChar = getc(rsInput->pFile);
		if (EOF == iChar) return 0;
		assert(iChar >= 0 && iChar <= UCHAR_MAX);
		*rcuChar = (char unsigned const)iChar;
	}
	return 1;
}
