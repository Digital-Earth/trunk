# if !defined(HEADERS__CURSOR)
	# define HEADERS__CURSOR

	# include "Index.h"

	struct Cursor {
		struct Index sIndex;
		Direction_te eDirection;
	};

	static INLINE void Cursor_fStepAcross(
		REGISTER struct Cursor rsCursor[mReferenceConst(1)]
	) {
		mStrongAssert(rsCursor);

		rsCursor->eDirection = Index_feAddDirection(&(rsCursor->sIndex), rsCursor->eDirection);
	}

	/* Rotate by the given rotation factor.
	New direction is to current direction as 'eDirection' is to Direction_i001.
	*/
	static INLINE void Cursor_fRotate(
		REGISTER struct Cursor rsCursor[mReferenceConst(1)],
		REGISTER Direction_te const eDirection
	) {
		mStrongAssert(rsCursor);

		rsCursor->eDirection = Direction_feProduct(rsCursor->eDirection, eDirection);
	}

	static INLINE Boolean_tb Cursor_fbStepIn(
		REGISTER struct Cursor rsCursor[mReferenceConst(1)]
	) {
		mStrongAssert(rsCursor);

		return Index_fbIncrement(&(rsCursor->sIndex));
	}

	static INLINE Boolean_tb Cursor_fbStepOut(
		REGISTER struct Cursor rsCursor[mReferenceConst(1)]
	) {
		mStrongAssert(rsCursor);

		return Index_fbDecrement(&(rsCursor->sIndex));
	}

	Boolean_tb Cursor_fbTest(void);

# endif
