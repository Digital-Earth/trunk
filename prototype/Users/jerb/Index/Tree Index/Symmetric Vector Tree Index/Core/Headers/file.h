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

# if !defined(HEADERS__FILE)
	# define HEADERS__FILE

	# include "boolean.h"
	# include "char_string.h"
	# include "core.h"
	# include "file_mode.h"
	# include <stdio.h>

	/* A file wrapper to be used when opening or closing files.  Not copyable. */
	typedef struct {
		FILE * pFile;
		char const * pzcName; /* Unowned. */
		char /* FileMode_te */ cMode;
	} File_ts;

	# define File_mInitializer() {0, 0, 0}

	static INLINE Boolean_tb File_fbValid(
		REGISTER File_ts const rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);

		return (0 == rsFile->pFile && 0 == rsFile->cMode) || FileMode_fbValid(rsFile->cMode);
	}

	static INLINE FileMode_te File_feMode(
		REGISTER File_ts const rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);

		return rsFile->cMode;
	}

	static INLINE Boolean_tb File_fbInput(
		REGISTER File_ts rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);
		
		if (rsFile->pFile) return 0;
		rsFile->pFile = stdin;
		rsFile->pzcName = 0;
		rsFile->cMode = FileMode_iReadText;
		return 1;
	}

	static INLINE Boolean_tb File_fbOutput(
		REGISTER File_ts rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);
		
		if (rsFile->pFile) return 0;
		rsFile->pFile = stdout;
		rsFile->pzcName = 0;
		rsFile->cMode = FileMode_iWriteText;
		return 1;
	}

	static INLINE Boolean_tb File_fbError(
		REGISTER File_ts rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);
		
		if (rsFile->pFile) return 0;
		rsFile->pFile = stderr;
		rsFile->pzcName = 0;
		rsFile->cMode = FileMode_iWriteText;
		return 1;
	}

	/* Returns true if it is open after the call. */
	Boolean_tb File_fbOpen(
		REGISTER File_ts rsFile[mReferenceConst(1)],
		REGISTER const char rzcName[mReferenceConst(1)],
		REGISTER FileMode_te const eMode
	);

	/* Open a temporary file in binary update mode. */
	Boolean_tb File_fbOpenTemporary(
		REGISTER File_ts rsFile[mReferenceConst(1)]
	);

	char const * File_rzcTemporaryName(void);

	/* Returns true if it was open, and is now closed. */
	static INLINE Boolean_tb File_fbClose(
		REGISTER File_ts rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);

		rsFile->cMode = FileMode_iClosed;
		{
			REGISTER FILE * const pFile = rsFile->pFile;
			rsFile->pFile = 0;
			return (pFile && 0 == fclose(pFile));
		}
	}

	static INLINE Boolean_tb File_fbClosed(
		REGISTER File_ts rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);
		
		return (FileMode_iClosed == rsFile->cMode && mbVerify(0 == rsFile->pFile));
	}

	static INLINE Boolean_tb File_fbExists(
		REGISTER char const * const rzcName
	) {
		mAssert(rzcName);

		{
			REGISTER FILE * const pFile = fopen(rzcName, "r");
			if (pFile) {
				fclose(pFile);
				return 1;
			}
		}
		return 0;
	}

	/* Delete the file. */
	static INLINE Boolean_tb File_fbRemove(
		REGISTER char const * const rzcName
	) {
		mAssert(rzcName);

		return (0 == remove(rzcName));
	}

	static INLINE Boolean_tb File_fbWrite(
		REGISTER File_ts rsOutput[mReferenceConst(1)],
		REGISTER char unsigned const cuChar
	) {
		mAssert(rsOutput);
		mAssert(!File_fbClosed(rsOutput));

		return (cuChar == putc(cuChar, rsOutput->pFile));
	}

	static INLINE Boolean_tb File_fbWriteString(
		REGISTER File_ts rsOutput[mReferenceConst(1)],
		REGISTER CharString_tzc const rzcString[mReferenceConst(1)]
	) {
		mAssert(rsOutput);
		mAssert(!File_fbClosed(rsOutput));

		return (EOF != fputs(rzcString, rsOutput->pFile));
	}

	Boolean_tb File_fbRead(
		REGISTER File_ts rsInput[mReferenceConst(1)],
		REGISTER char unsigned rcuChar[mReferenceConst(1)]
	);

	static INLINE Boolean_tb File_fbRewind(
		REGISTER File_ts rsFile[mReferenceConst(1)]
	) {
		mAssert(rsFile);
		
		if (!rsFile->pFile) return 0;
		rewind(rsFile->pFile);
		return 1;
	}

# endif
