#ifndef PYXIS__DERM__SUB_INDEX_MATH_H
#define PYXIS__DERM__SUB_INDEX_MATH_H
/******************************************************************************
sub_index_math.h

begin		: 2003-12-03
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/derm/sub_index.h"

// standard includes
#include <vector>

// forward declarations
class PYXCoordPolar;

/*!
The PYXMath class defines various mathematical operations on PYXIS indices.
Any function that directly alters or manipulates a PYXIndex will be done
here.  The current areas of influence of the PYXMath class include:
/verbatim
	Tesseral Arithmetic
		Addition, Subtraction etc.
	Tesseral Geometry
		Index transformation to/from vectors etc.
	Resolution Manipulation
		Move up/down resolutions etc
/endverbatim
*/
//! Defines math operations on PYXIS indices.
class PYXLIB_DECL PYXMath
{
public:

	//! Test method
	static void test();

	//! The maximum absolute resolution of an index.
	static const int knMaxAbsResolution;

	/*! 
	The maximum relative resolution.  This defines the maximum tile 
	resolution. 
	*/
 	static const int knMaxRelResolution;

	//! The distance between hexagons at resolution zero
	static const double kfResZeroIntercell;

	/*!
	In the PYXIS grid, a class I hexagon is oriented so that three of its
	vertices face vertices of the icosahedron. A class II hexagon is oriented
	so that three of its sides face vertices of the icosahedron.
	*/
	//! Enumeration for hexagon class
	enum eHexClass {knClassI = 0, knClassII};

	//! Get the class of hexagon for a given resolution.
	static eHexClass getHexClass(int nResolution);

	//! Get the alternate hexagon class
	static eHexClass getAltHexClass(eHexClass nClass);

	/*!
	The PYXIS index can be rotated about the origin through 30 degrees
	in either the clockwise or counter clockwise direction
	*/
	//! Enumeration for hexagon rotation direction
	enum eRotateDir {knInvalid = -1, knCW, knCCW};

	/*!
	The directions point outward from the hexagon's centre and are
	perpendicular to the hexagon's sides.

	For a class I and II hexagon the directions are defined as follows:
	\verbatim

        Class I        Class II

         2  1             1
	      /\              _ 
 	   3 |  | 6       2 /   \ 6
	      \/          3 \ _ / 5
	     4  5             4
	
	\endverbatim 
	*/
	//! Enumeration for directions.
	enum eHexDirection
	{
		knDirectionZero = 0,
		knDirectionOne,
		knDirectionTwo,
		knDirectionThree,
		knDirectionFour,
		knDirectionFive,
		knDirectionSix,
	};

	//! Negate a direction
	static inline eHexDirection negateDir(eHexDirection nHexDir)
	{
		int nDir = nHexDir + 3;

		// hexagon has six sides
		if (6 < nDir)
		{
			nDir -= 6;
		}

		return static_cast<eHexDirection>(nDir);
	}

	//! Calculate the new cell index when we move one hexagon in a specific direction.
	static void move(	const PYXIndex& start,
						eHexDirection nDir,
						PYXIndex* pResult	);

	//! Add two indices.
	static void add(	const PYXIndex& first,
							const PYXIndex& second,
							int nResolution,
							PYXIndex* pResult,
							bool bGrow = false	);

	//! Subtract an index from another one.
	static void subtract(	const PYXIndex& first,
							const PYXIndex& second,
							int nResolution,
							PYXIndex* pResult	);

	//! Calculate a new cell index relative to the origin.
	static void multiply(	int nFactor,
							eHexDirection nHexDirection,
							int nResolution,
							PYXIndex* pResult	);

	//! Convert polar coordinates to a PYXIndex.
	static void polarToIndex(	const PYXCoordPolar& pt,
								int nResolution,
								PYXIndex& index,
								bool bGrow	);

	//! Convert a PYXIndex to polar coordinates.
	static void indexToPolar(const PYXIndex& index, PYXCoordPolar* pPolar);

	//! Get the parent index of the passed index
	static PYXIndex getParent(const PYXIndex& pyxIndex);

	//! Vector of factors
	typedef std::vector<int> FactorVector;

	//! Factor an index into its component directions.
	static void factor(PYXIndex pyxIndex, FactorVector& vecFactors);

	//! Factor an index into signed components in directions 2 and 6
	static void factor(	const PYXIndex& index,
						int* pnMove2,
						int* pnMove6	);

	//! Zoom in
	static bool zoomIn(	PYXIndex* pIndex, 
						PYXMath::eHexDirection nHexDirection = knDirectionZero	);

	//! Zoom out
	static bool zoomOut(	PYXIndex* pIndex,
							eHexDirection* pnDirection = 0	);

	//! Determmine the direction from the origin
	static eHexDirection originDir(const PYXIndex &pyxIndex);

	//! Calculate the common ancestor index of two indices
	static PYXIndex calcAncestorIndex(	const PYXIndex& index1,
										const PYXIndex& index2	);

	//! Calculate the relative descendant index from a parent to child
	static PYXIndex calcDescendantIndex(	const PYXIndex& parent,
											const PYXIndex& child	);

	//! Rotate an index about the origin 
	static void rotateIndex(	PYXIndex* pIndex,
								unsigned int nRotCount,
								eRotateDir nDirection	);

	//! Rotate a direction.
	static eHexDirection rotateDirection(	const eHexDirection nDirection,
											int nRotate	);

	/*!
	This adjusts a difference between two directions.
	\param pnRotate	The rotation. (optional) (out)
	\param nDelta	The delta.
	*/
	//! Apply a rotation delta.
	static void rotateDelta(int* pnRotate, int nDelta)
	{
		if (pnRotate)
		{
			// hexagon has six sides
			(*pnRotate += nDelta + 6) %= 6;
		}
	}

	/*!
	This adjusts a difference between two directions.
	\param nRotate	The rotation.
	\param nDelta	The delta.
	\return The rotation after applying the delta.
	*/
	//! Apply a rotation delta.
	static int rotateDelta(int nRotate, int nDelta)
	{
		rotateDelta(&nRotate, nDelta);
		return nRotate;
	}

	//! Return the direction of the index from the origin
	static eHexDirection hexSector(const PYXIndex& pyxIndex);

	//! Calculate the reference direction for a particular angle
	static eHexDirection hexSector(double angle);

	//! Calculate the inter-cell distance for a given resolution (0 based)
	static double calcInterCellDistance(int nResolution);

	//! Calculate the circumradius for a given resolution (0 based)
	static double calcCircumRadius(int nResolution);

	//! Calculate the inradius for a given resolution (0 based)
	static double calcInRadius(int nResolution);

	//! Determine if the index has only a single direction component
	static bool isInLine(const PYXIndex& index);

	//! Factor an index into its component directions.
	static void factor(	const PYXIndex& index,
						eHexDirection* pnDirA,
						int* pnMoveA,
						eHexDirection* pnDirB,
						int* pnMoveB	);

private:

	//! Hide constructor
	PYXMath();

	//! Reduce a set of three factors to two.
	static void reduceFactors(int* pnD1, int* pnD2, int* pnD3);

	//! Add two indices.
	static void add(	PYXIndex first,
						PYXIndex second,
						eHexClass,
						PYXIndex* pSum	);

	//! Subroutine used by the add method.
	static void pairToIntegers(unsigned int nPair, int* pnA, int* pnB);

	//! Subroutine used by the add method.
	static unsigned int integersToPair(int nA, int nB);

	//! Subroutine used by the add method.
	static void divideByThree(	int* pnA,
								int* pnB,
								int* pnR,
								int* pnS	);

	//! Extract the cell count in a particular direction for a polar coordinate.
	static int extractDirectionComponent(
		eHexDirection nDirection, 
		PYXCoordPolar* pPolar	);	

	//! Initialize static data
	static void initStaticData();

	//! Free any static data
	static void freeStaticData() {}

	//! Vector of inter-cell distances for each resolution
	static std::vector<double> m_vecInterCellDistances;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

#endif
