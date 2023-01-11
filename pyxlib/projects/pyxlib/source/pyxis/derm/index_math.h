#ifndef PYXIS__DERM__INDEX_MATH_H
#define PYXIS__DERM__INDEX_MATH_H
/******************************************************************************
index_math.h

begin		: 2004-01-29
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/sub_index_math.h"

// standard includes
#include <vector>
#include <cassert>

// forward declarations
class PYXIcosIndex;

/*!
The PYXMath class defines various mathematical operations on PYXISIcosIndex 
objects. Any function that directly alters or manipulates a PYXIcosIndex will
be done here. The current areas covered by the PYXIcosMath class include:

/verbatim
	Tesseral Arithmetic
		Addition, Subtraction, move etc.
	Tesseral geometry
		Resolution 1 connectivity
		Resolution 2 connectivity
		Missing directions for pentagons

/endverbatim

Resolution 0 is represented by pentagons that are each centred on the 12 
vertices.  Resolution 1 is made up of hexagons and pentagons and
forms the 3D shape of a truncated icosahedron (soccer ball).  The remaining
resolutions are made up entirely of hexagons that are tesselated down from 
the first generation pentagons at the vertices.

When the icosahedron is assembled into its 3D shape it has 12 unique
vertices that are represented as follows when the icosahedron is unfolded:

\verbatim
    1   1   1   1   1
   /\  /\  /\  /\  /\
  /  \/  \/  \/  \/  \
 2----3---4---5---6---2
  \  /\  /\  /\  /\  /\
   \/  \/  \/  \/  \/  \
    7---8---9---10--11--7
    \  /\  /\  /\  /\  /
     \/  \/  \/  \/  \/
     12  12  12  12  12
\endverbatim

The names of the faces can be represented as follows when the icosahedron
is unfolded:
\verbatim

 /\  /\  /\  /\  /\
/A \/B \/C \/D \/E \
--------------------
\F /\G /\H /\I /\J /\
 \/K \/L \/M \/N \/O \
  --------------------
  \P /\Q /\R /\S /\T /
   \/  \/  \/  \/  \/
\endverbatim

*/
//! Defines math operations on PYXIS icosahedron indices.
class PYXLIB_DECL PYXIcosMath 
{
public:

	//! Test method
	static void test();

	//! Zoom into children.
	static bool zoomIntoChildren(	PYXIcosIndex* pIndex, 
						PYXMath::eHexDirection nHexDirection = PYXMath::knDirectionZero	);

	//! Zoom into neighbourhood.
	static bool zoomIntoNeighbourhood(	PYXIcosIndex* pIndex, 
						PYXMath::eHexDirection nHexDirection = PYXMath::knDirectionZero	);

	//! Zoom out.
	static bool zoomOut(	PYXIcosIndex* pIndex,
							PYXMath::eHexDirection* pnDirection = 0	);


	//! Calculate the common ancestor index from two indices
	static PYXIcosIndex calcAncestorIndex(	const PYXIcosIndex& index1,
											const PYXIcosIndex& index2	);

	//! Calculate the relative descendant index from a parent to child.
	static PYXIndex calcDescendantIndex(	const PYXIcosIndex& parent,
											const PYXIcosIndex& child	);

	//! Rotate a direction by the specified number of 30 degree rotations.
	static PYXMath::eHexDirection rotateDirection(	const PYXIcosIndex& index,
													PYXMath::eHexDirection nDirection,
													int nRotate	);

	//! Calculate the new cell index when we move one hexagon in a given dir.
	static PYXIcosIndex move(	const PYXIcosIndex& start,
								PYXMath::eHexDirection nHexDirection	);

	//! Perform a single step movement at any resolution.
	static bool move(	PYXIcosIndex* pIndex,
						PYXMath::eHexDirection nHexDirection,
						int* pnRotate = 0	);

	//! Add two indices.
	static PYXIcosIndex add(	PYXIcosIndex first,
								PYXIcosIndex second,
								int nResolution	);

	//! Determine if a direction is valid for the hexagon/pentagon.
	static bool isValidDirection(	const PYXIcosIndex& pyxIndex,
									PYXMath::eHexDirection nHexDirection	);

	//! Determine if a specified direction is valid for the given vertex.
	static bool isValidDirection(	int nVertex,
	 								PYXMath::eHexDirection nHexDirection	);

	//! Is the resolution valid.
	static bool isValidResolution(int nResolution);

	//! Determine the vertex owner of a given face.
	static int faceOwner(char nFace);

	//! Get the parent index of the specified index.
	static PYXIcosIndex getParent(const PYXIcosIndex& pyxIndex);

	//! Get the indices of the cells covering the cell specified by the index.
	static int getCoveringCells(const PYXIcosIndex& index, PYXIcosIndex* pIndices);

	//! Get the indices of the cells covered by the cell specified by the index.
	static int getCoveredCells(const PYXIcosIndex& index, PYXIcosIndex* pIndices);

	//! Returns all covering cells starting at knMinSubRes.
	static void getAllCoveringCells(const PYXIcosIndex& index, std::vector<PYXIcosIndex>& vecCovering, std::vector<short>& vecSlice);

	//! Get the vertices for a given face.
	static void getFaceVertices(	char cFace,
									int* pnVertex1,
									int* pnVertex2,
									int* pnVertex3	);

	//! Get the angle of direction one from the base of the face.
	static double getDir1Angle(char cFace);

	//! Determine if the face points upwards.
	static bool getFaceOrientation(char cFace);

	//! Find the gap direction for a given index.
	static bool getCellGap(	const PYXIcosIndex& index,
							PYXMath::eHexDirection* pnDirection = 0	);

	//! Verify the index is not in a pentagon gap.
	static bool isValidIndex(const PYXIcosIndex& pyxIndex);

	//! Convert polar coordinates to a PYXIcosIndex.
	static void polarToIndex(	const PYXCoordPolar& pt,
								int nResolution,
								char cFace,
								PYXIcosIndex* pIndex	);

	//! Find the polar coordinate and the face origin for a given index.
	static void indexToPolar(	const PYXIcosIndex& index,
								PYXCoordPolar* pPolar,
								char* pcFace	);

	//! Get the direction that best represents a polar coordinate.
	static PYXMath::eHexDirection getDirectionFromPolar(double fAngle);

	//! Get the number of cells for a given resolution.
	static int getCellCount(int nResolution);

	//! Get the number of cells for a given resolution.
	static int getCellCount(const PYXIcosIndex& root, int nResolution);

	//! Get the offset for a given resolution.
	static unsigned int getOffset(const PYXIcosIndex& root, int nResolution);

	//! Calculate the position of a given index within its resolution.
	static unsigned int calcCellPosition(const PYXIcosIndex& index);

	//! Calculate the position of a given index within its resolution.
	static unsigned int calcCellPosition(	const PYXIcosIndex& root,
											const PYXIcosIndex& index	);

	//! Calculate the position of an edge index along the edge.
	static int calcEdgePosition(	const PYXIcosIndex& root,
									const PYXIcosIndex& index	);

	//! Calculate an index from an offset.
	static PYXIcosIndex calcIndexFromOffset(	const PYXIcosIndex& root,
												int nRes,
												unsigned int nOffset	);

	//! Determine if the specified index is a sibling of this one.
	static bool areSiblings(	const PYXIcosIndex& index1,	
								const PYXIcosIndex& index2,
								PYXMath::eHexDirection* pnDirection = 0	);

	//! Determine the PYXIS direction of this index from the parent.
	static bool directionFromParent(	const PYXIcosIndex& index,
										PYXMath::eHexDirection* pnDirection = 0	);

	//! Determine if all hexagons at the data res are decendants of the root.
	static bool isDataContained(	const PYXIcosIndex& rootIndex, 
									int nDataDepth	);

	//! Determine the PYXIcosIndex values for all the neighbours of an index.
	static void getNeighbours(	const PYXIcosIndex& pyxIndex, 
								std::vector<PYXIcosIndex>* pVecNeighbours	);

	//! Determine the direction of a neighbouring PYXIcosIndex.
	static PYXMath::eHexDirection getNeighbourDirection(	const PYXIcosIndex& indexFrom,
															const PYXIcosIndex& indexTo	);

	//! Rotate the index in the counter-clockwise direction.
	static void rotateIndex(	PYXIcosIndex* pIndex,
								int nRotation	);

	//! Rotate the specified digits leaving the 
	static void rotateTail(	PYXIcosIndex* pIndex,
							int nDigits,
							int nRotation	);

	//! A definition for a vector of indices
	typedef std::vector<PYXIcosIndex> IndexVector;


	/*! 
	Helper class to calculaing a bounding sphere around cells and tiles on a unit sphere
	
	Note: The radius caluclated are in radians on the unit sphere.
	*/
	class PYXLIB_DECL UnitSphere
	{
	protected:
		friend class PYXIcosMath;
	
		static void initialize();

		static void uninitialize();

	public:
		//! Calculate the circumradius for a given resolution (0 based) on the unit sphere
		static double calcCellCircumRadius(int nResolution);

		//! Calculate the circumradius for a given cell on the unit sphere
		static double calcCellCircumRadius(const PYXIcosIndex & index);

		//! Calculate the circumradius for a inifinte tile with a root index on the unit sphere
		static double calcTileCircumRadius(const PYXIcosIndex & index);

	private:
		UnitSphere() {};
		UnitSphere(const UnitSphere & other) {};

		static std::vector<double> s_radius;
	};


private:

	//! Hide constructor.
	PYXIcosMath();

	//! Disable copy constructor.
	PYXIcosMath(const PYXIcosMath&);

	//! Disable copy assignment.
	void operator=(const PYXIcosMath&);

	//! Initialize static data.
	static void initStaticData();

	//! Free any static data.
	static void freeStaticData() {}

	//! Initialize cell count and resolution offset vectors for vertices.
	static void initVertexVectors();

	//! Initialize cell count and resolution offset vectors for faces.
	static void initFaceVectors();

	//! Perform a single step movement on resolution 0.
	static bool resolution0Move(	PYXIcosIndex* pIndex, 
									PYXMath::eHexDirection nHexDirection,
									int* pnRotate = 0	);

	//! Perform a single step movement on the resolution 1.
	static bool resolution1Move(	PYXIcosIndex* pIndex, 
									PYXMath::eHexDirection nHexDirection,
									int* pnRotate = 0	);

	//! Perform a single step movement on resolution 2.
	static void resolution2Move(	PYXIcosIndex* pIndex,
									PYXMath::eHexDirection nHexDirection,
									int* pnRotate = 0	);

	//! Get the face in a given direction from a specific vertex.
	static char faceFromVertex(int nVertex, int nHexDirection);

	//! Get the face in a given direction from a specific vertex with no checking of input.
	static char faceFromVertexUnsafe(int nVertex, int nHexDirection);

	//! Get the direction of a vertex to an adjacent face.
	static  bool faceDirectionFromVertex(	int nVertex,
											char nFace,
											PYXMath::eHexDirection* pnDirection	);

	//! Provide a corrected index when tesselation is exceeded.
	static void overflowCorrect(	PYXIcosIndex* pIndex,
									int nResolution,
									int* pnRotate = 0	);

	//! Correct a vertex tesselation when index is in gap.
	static bool gapCorrect(		const PYXIcosIndex& startIndex,
								PYXIcosIndex* pResultIndex,
								int* pnRotate = 0	);

	//! Determine the rotation of the pole in a given direction.
	static int getPolarRotation(	unsigned int nPole,
									PYXMath::eHexDirection nHexDirection	);

	//! Typedef for resolution cell count vectors.
	typedef std::vector<int> ResolutionCellsVector;

	//! Number of vertex cells in each resolution.
	static ResolutionCellsVector m_vecVertexResolutionCells;

	//! Number of face cells in each resolution.
	static ResolutionCellsVector m_vecFaceResolutionCells;

	//! Typedef for resolution offset vectors.
	typedef std::vector<unsigned int> ResolutionOffsetsVector;

	//! Offset for each resolution in a vertex.
	static ResolutionOffsetsVector m_vecVertexResolutionOffsets;

	//! Offset for each resolution in a face.
	static ResolutionOffsetsVector m_vecFaceResolutionOffsets;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

#endif // guard
