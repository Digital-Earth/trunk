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

# if !defined(HEADERS__FILE_MODE)
	# define HEADERS__FILE_MODE

	# include "bit.h"
	# include "boolean.h"
	# include "core.h"
	# include "file_operation.h"

	/* These values are chosen such that bit-wise queries can be made.  Bits, from right:
	- 0-1: operations
	- 2: t = 0, b = 1
	- 3: default = 0, + = 1
	*/
	typedef enum {
		FileMode_iClosed = 0,
		FileMode_iReadText = 1 /* "r"; read text mode */,
		FileMode_iWriteText = 2 /* "w"; write text mode (truncates file to zero length or creates new file) */,
		FileMode_iAppendText = 3 /* "a"; append text mode for writing (opens or creates file and sets file pointer to the end-of-file) */,
		FileMode_iReadBinary = 5 /* "rb"; read binary mode */,
		FileMode_iWriteBinary = 6 /* "wb"; write binary mode (truncates file to zero length or creates new file) */,
		FileMode_iAppendBinary = 7 /* "ab"; append binary mode for writing (opens or creates file and sets file pointer to the end-of-file) */,
		FileMode_iReadTextPlus = 9 /* "r+"; read and write text mode */,
		FileMode_iWriteTextPlus = 10 /* "w+"; read and write text mode (truncates file to zero length or creates new file) */,
		FileMode_iAppendTextPlus = 11 /* "a+"; read and write text mode (opens or creates file and sets file pointer to the end-of-file) */,
		FileMode_iReadBinaryPlus = 13 /* "r+b" or "rb+"; read and write binary mode */,
		FileMode_iWriteBinaryPlus = 14 /* "w+b" or "wb+"; read and write binary mode (truncates file to zero length or creates new file) */,
		FileMode_iAppendBinaryPlus = 15 /* "a+b" or "ab+"; read and write binary mode (opens or creates file and sets file pointer to the end-of-file) */
	} FileMode_te;

	# define FileMode_miOperation(eFileMode) ((eFileMode) & 3)

	static INLINE Boolean_tb FileMode_fbValid(
		REGISTER FileMode_te const eFileMode
	) {
		return (0 == eFileMode || (0 < eFileMode && eFileMode < 16 && 0 < FileMode_miOperation(eFileMode)));
	}

	static INLINE FileOperation_te FileMode_feOperation(
		REGISTER FileMode_te const eFileMode
	) {
		mAssert(FileMode_fbValid(eFileMode));
	
		return FileMode_miOperation(eFileMode);
	}

	static INLINE Boolean_tb FileMode_fbBinary(
		REGISTER FileMode_te const eFileMode
	) {
		mAssert(FileMode_fbValid(eFileMode));

		return Bit_mbOn(eFileMode, 2);
	}

	/* Indicates whether there is a '+' in the mode string. */
	static INLINE Boolean_tb FileMode_fbPlus(
		REGISTER FileMode_te const eFileMode
	) {
		mAssert(FileMode_fbValid(eFileMode));

		return Bit_mbOn(eFileMode, 3);
	}
	
	static INLINE Boolean_tb FileMode_fbClosed(
		REGISTER FileMode_te const eFileMode
	) {
		return 0 == eFileMode;
	}
	
# endif
