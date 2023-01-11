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

# if !defined(HEADERS__FILE_OPERATION)
	# define HEADERS__FILE_OPERATION

	# include "boolean.h"
	# include "core.h"

	/* A file operation. */
	typedef enum {
		FileOperation_iNone = 0,
		FileOperation_iRead = 1,
		FileOperation_iWrite = 2,
		FileOperation_iAppend = 3
	} FileOperation_te;

	static INLINE Boolean_tb FileOperation_fbValid(
		REGISTER FileOperation_te const eFileOperation
	) {
		return eFileOperation >= 0 && eFileOperation < 4;
	}

# endif
