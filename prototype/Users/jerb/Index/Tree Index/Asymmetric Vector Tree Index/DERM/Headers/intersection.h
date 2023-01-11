# if !defined(HEADERS__INTERSECTION)
	# define HEADERS__INTERSECTION

	/* The 8 topological relations resulting from the 9-intersection model. */
	typedef enum {
		Intersection_iEqual = 1,
		Intersection_iDisjoint = 2,
		Intersection_iMeet = 3,
		Intersection_iContains = 4,
		Intersection_iInside = 5,
		Intersection_iCovers = 6,
		Intersection_iCoveredBy = 7,
		Intersection_iOverlap = 8
	} Intersection_te;

	static INLINE Boolean_tb Intersection_fbValid(
		REGISTER Intersection_te const eIntersection
	) {
		return (eIntersection > 0 && eIntersection <= 8);
	}

# endif
