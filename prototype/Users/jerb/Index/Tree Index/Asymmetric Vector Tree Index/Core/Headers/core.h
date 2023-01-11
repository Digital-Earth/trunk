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

# if !defined(HEADERS__CORE)
	# define HEADERS__CORE

	# include <limits.h>
	# include <stdlib.h>
	# include <assert.h>

	/* Version. */

	# if defined(__STDC__) && __STDC__ && (__STDC_VERSION__ >= 199901L)
		# define mbC99() 1
	# else
		# define mbC99() 0
	# endif

	# if defined(__GNUC__)
		# define mbGnuC() 1
	# else
		# define mbGnuC() 0
	# endif

	/* Debugging. */

	# if defined(NDEBUG)
		# define mStaticAssert(bExpression)
		# define mStrongAssert(bExpression)
		# define mbVerify(bExpression) bExpression
	# else
		# define mStaticAssert(bExpression) extern char const acStaticAssertFailure[(bExpression)? 1: -1]
		# define mStrongAssert(bExpression) { if (!(bExpression)) { assert(0); exit(EXIT_FAILURE); } }
		# define mbVerify(bExpression) ((bExpression) || (assert(0), 0))
	# endif

	/* Instrumentation. */

	# if mbC99()
		# define msFunction() __func__
	# else
		# define msFunction() ""
	# endif
	# define msFile() __FILE__
	# define miuLine() __LINE__

	/* Tokenization. */

	# define mQuote(Expression) #Expression
	# define mConcatenation(Left, Right) Left##Right

	/* Keywords. */

	# if mbGnuC()
		/* There's a bug in GCC 3.3, used by Xcode, whereby registers are screwed.
		Just do a blanket disable of the 'register' keyword for now.
		*/
		# define REGISTER
	# elif !defined(NDEBUG)
		/* Use the register keyword to enforce not taking addresses of variables. */
		# define REGISTER register
	# else
		/* Let the compiler do its own register optimization in the non-debug builds. */
		# define REGISTER
	# endif

	# if mbC99()
		# define INLINE inline
		# define RESTRICT restrict
	# else
		# if !defined(INLINE)
			# if defined(__inline)
				# define INLINE __inline __fastcall
			# elif defined(_inline)
				# define INLINE _inline _fastcall
			# else
				# define INLINE
			# endif
		# endif
		# if !defined(RESTRICT)
			# define RESTRICT
		# endif
	# endif

	/* Types. */

	/* This macro checks that the given expression has a type 
	which is compatible with (assignable to) the specified type, 
	forcing a compile error if it does not. 
	It does not evaluate the expression. 
	Note that the specified type must be a complete type, 
	i.e. it must not be a pointer to a struct which has 
	not been defined. 

	The implementation of this macro looks like it dereferences 
	a null pointer, but because that code is inside sizeof(), it will 
	not get executed; the compiler will instead just check that it is 
	type-correct.
	*/
	# define mbIs(Expression, tComplete) ( \
		(void)(sizeof (*(tComplete *)0 = (Expression))), \
		1 \
	)

	/* The same as EXPRRESSION cast to type DESTINATION_TYPE, 
	except that it checks that the expression has a type which is 
	compatible with (assignable to) SOURCE_TYPE, forcing a compile error 
	if it does not.
	*/
	# define mAs(Expression, tSource, tDestination) ( \
		mbIs((Expression), tSource), \
		(tDestination)(Expression) \
	)

	/* Integers. */

	# if mbC99()
		# define LONG_LONG long long
	# elif !defined(LONG_LONG)
		# define LONG_LONG long
	# endif

	# define miuUnsignedTypeMaximum(tiuType) ((tiuType)-1)

	/* References. */

	/* A constant reference to an array. */
	# if mbC99()
		# define mReferenceConst(iuElementCount) static const (iuElementCount)
	# else
		# define mReferenceConst(iuElementCount) 1
	# endif

	/* A non-constant reference to an array. */
	# if mbC99()
		# define mReference(iuElementCount) static (iuElementCount)
	# else
		# define mReference(iuElementCount) 1
	# endif

# endif
