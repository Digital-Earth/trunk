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

# if !defined(HEADERS__FLEXIBLE_STRUCT)
	# define HEADERS__FLEXIBLE_STRUCT

	# include "core.h"
	# include "memory.h"
	# include "size.h"

	/* This provides access to the flexible struct member. */
	# define FlexibleStruct_mrMember(sFlexible) (sFlexible).FlexibleStruct_Member

	/* This returns the size of the non-flexible part of the flexible struct. */
	# define FlexibleStruct_miuHeaderCharCount(tsFlexible) Size_miuCharOffset(tsFlexible, FlexibleStruct_Member)

	/* Note that some platforms may define a value for FlexibleStruct___MEMBER_SUBSCRIPT 
	that is known to work via the "struct hack".
	This is redefined here if the compiler supports true flexible array members.
	*/
	# if mbC99()
		# undef FlexibleStruct___MEMBER_SUBSCRIPT
		# define FlexibleStruct___MEMBER_SUBSCRIPT
	# endif

	# if defined(FlexibleStruct___MEMBER_SUBSCRIPT)
		# define FlexibleStruct_mDeclaration() FlexibleStruct_Member[FlexibleStruct___MEMBER_SUBSCRIPT]

		# if mbC99()
			/* Optimization. */
			# undef FlexibleStruct_miuHeaderCharCount
			# define FlexibleStruct_miuHeaderCharCount(tsFlexible) Size_miuCharCount(tsFlexible)
		# endif

		# define FlexibleStruct_mbAllocate(psFlexible, tsFlexible, iuFlexibleMemberCharCount) ( \
			( \
				mbVerify(0 < iuFlexibleMemberCharCount && mbIs((psFlexible), tsFlexible *)) && \
				Size_fbCanAdd(FlexibleStruct_miuHeaderCharCount(tsFlexible), (iuFlexibleMemberCharCount)) \
			)? Memory_mbAllocate( \
				(psFlexible), \
				FlexibleStruct_miuHeaderCharCount(tsFlexible) + (iuFlexibleMemberCharCount) \
			): 0 \
		)

		# define FlexibleStruct_mbAllocateAndZero(psFlexible, tsFlexible, iuFlexibleMemberCharCount) ( \
			( \
				mbVerify(0 < iuFlexibleMemberCharCount && mbIs((psFlexible), tsFlexible *)) && \
				Size_fbCanAdd(FlexibleStruct_miuHeaderCharCount(tsFlexible), (iuFlexibleMemberCharCount)) \
			)? Memory_mbAllocateAndZero( \
				(psFlexible), \
				FlexibleStruct_miuHeaderCharCount(tsFlexible) + (iuFlexibleMemberCharCount) \
			): 0 \
		)

		# define FlexibleStruct_mbReallocate(psFlexible, tsFlexible, iuFlexibleMemberCharCount) ( \
			( \
				mbVerify(0 < iuFlexibleMemberCharCount && mbIs((psFlexible), tsFlexible *)) && \
				Size_fbCanAdd(FlexibleStruct_miuHeaderCharCount(tsFlexible), (iuFlexibleMemberCharCount)) \
			)? Memory_mbReallocate( \
				(psFlexible), \
				FlexibleStruct_miuHeaderCharCount(tsFlexible) + (iuFlexibleMemberCharCount) \
			): 0 \
		)
	# else
		/* Do a single allocation which includes the struct and the flexible part, 
		and make the flexible member a pointer to the flexible part.
		*/
		# define FlexibleStruct_mDeclaration() * FlexibleStruct_Member

		/* Allocate extra space beyond the struct, and set the flexible member pointer to it. */
		# define FlexibleStruct_mbAllocate(psFlexible, tsFlexible, iuFlexibleMemberCharCount) ( \
			( \
				mbVerify(0 < iuFlexibleMemberCharCount && mbIs((psFlexible), tsFlexible *)) && \
				Size_fbCanAdd(Size_miuCharCount(tsFlexible), (iuFlexibleMemberCharCount)) \
			)? ( \
				Memory_mbAllocate( \
					(psFlexible), \
					Size_miuCharCount(tsFlexible) + (iuFlexibleMemberCharCount) \
				) && ( \
					((psFlexible)->FlexibleStruct_Member = (void * const)((psFlexible) + 1)), \
					1 \
				) \
			): 0 \
		)

		/* Allocate extra space beyond the struct, and set the flexible member pointer to it. */
		# define FlexibleStruct_mbAllocateAndZero(psFlexible, tsFlexible, iuFlexibleMemberCharCount) ( \
			( \
				mbVerify(0 < iuFlexibleMemberCharCount && mbIs((psFlexible), tsFlexible *)) && \
				Size_fbCanAdd(Size_miuCharCount(tsFlexible), (iuFlexibleMemberCharCount)) \
			)? ( \
				Memory_mbAllocateAndZero( \
					(psFlexible), \
					Size_miuCharCount(tsFlexible) + (iuFlexibleMemberCharCount) \
				) && ( \
					((psFlexible)->FlexibleStruct_Member = (void * const)((psFlexible) + 1)), \
					1 \
				) \
			): 0 \
		)

		/* Reallocate the struct, with its extra space beyond, and set the flexible member pointer to it
		(since reallocate might result in a different pointer). */
		# define FlexibleStruct_mbReallocate(psFlexible, tsFlexible, iuFlexibleMemberCharCount) ( \
			( \
				mbVerify(0 < iuFlexibleMemberCharCount && mbIs((psFlexible), tsFlexible *)) && \
				Size_fbCanAdd(Size_miuCharCount(tsFlexible), (iuFlexibleMemberCharCount)) \
			)? ( \
				Memory_mbReallocate( \
					(psFlexible), \
					Size_miuCharCount(tsFlexible) + (iuFlexibleMemberCharCount) \
				) && ( \
					((psFlexible)->FlexibleStruct_Member = (void * const)((psFlexible) + 1)), \
					1 \
				) \
			): 0 \
		)
	# endif

	# define FlexibleStruct_mbDeallocate(psFlexible) Memory_mbDeallocate(psFlexible)
	# define FlexibleStruct_mbDeallocateAndZero(psFlexible) Memory_mbDeallocateAndZero(psFlexible)

# endif
