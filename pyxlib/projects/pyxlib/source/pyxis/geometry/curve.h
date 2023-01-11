#ifndef PYXIS__GEOMETRY__CURVE_H
#define PYXIS__GEOMETRY__CURVE_H
/******************************************************************************
curve.h

begin		: 2005-01-12
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/great_circle_arc.h"

// forward declarations
class SnyderProjection;
class CoordLatLon;

//! An ordered collection of connected cells at a given resolution.
/*!
PYXCurve is an ordered collection of connected cells. It is used to represent
linear features. It is implemented as a collection of nodes (cells) in a line.

NOTE: The PYXCurve iterator cannot currently handle line segments with
endpoints in different resolution 1 tesselations. Also, lines that cross a
tesselation gap may not be correctly handled either.
*/
class PYXLIB_DECL PYXCurve : public PYXGeometry
{
public:

	//! Test method
	static void test();
	static bool testIsNeighbour(const PYXIcosIndex& index1, const PYXIcosIndex& index2);
	static void testLine(const CoordLatLon& coordStart, const CoordLatLon& coordDest, bool bDetailed = false);
	static PYXCurve testPoint(const CoordLatLon& coord);

	//! Create method
	static PYXPointer<PYXCurve> create()
	{
		return PYXNEW(PYXCurve);
	}

	//! Copy creator
	static PYXPointer<PYXCurve> create(const PYXCurve& rhs)
	{
		return PYXNEW(PYXCurve, rhs);
	}

	//! Deserialization creator.
	static PYXPointer<PYXCurve> create(std::basic_istream< char>& in)
	{
		return PYXNEW(PYXCurve, in);
	}

	//! Constructor
	PYXCurve() : m_nNodeDrop(0),m_length(0) {}

	//! Deserialization constructor.
	PYXCurve(std::basic_istream< char>& in);

	//! Destructor
	virtual ~PYXCurve() {}

	//! Equality operator
	bool operator ==(const PYXCurve& rhs) const;

	//! Inequality operator.
	bool operator !=(const PYXCurve& rhs) const {return !(*this == rhs);}

	//! Less than operator
	bool operator <(const PYXCurve& rhs) const;

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	/*!
	Is the geometry empty?

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	//! Is the geometry empty?
	virtual bool isEmpty() const {return m_vecNode.empty();}

	//! Get the cell resolution.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Does the specified geometry intersect with this one?
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const; 

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	//! Add a node to the curve.
	void addNode(const PYXIcosIndex& index);

	//! Serialize the curve to a stream.
	virtual void serialize(std::basic_ostream< char>& out) const;

	//! Derserialize a curve from a stream.
	virtual void deserialize(std::basic_istream< char>& in);

	/*!
	\param	pVecIndex	The container to hold the returned indices.
	*/
	//! Calculate a series of PYXIS indices around a geometry.
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
	{
		assert(pVecIndex != 0 && "Null pointer.");
		pVecIndex->clear();
		assert(false && "Not yet implemented.");
	}

	virtual PYXBoundingCircle getBoundingCircle() const
	{		
		assert(false && "Not yet implemented.");
		return PYXBoundingCircle();
	}

	//! Copies a representation of this geometry into a tile collection.
	virtual inline void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual inline void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	//! Clear the geometry of the curve and remove all nodes.
	void clear() {m_vecNode.clear();}

	//! Return the list of nodes that make up the curve.
	const std::vector<PYXIcosIndex>& getNodes() const { return m_vecNode; }

	//! Return the number of added nodes that were dropped because they were duplicates.
	int getNodeDropCount() const { return m_nNodeDrop; }

	//! Return the lengh of the line in radains
	double getLength() const { return m_length; }

public:

	//! Make a somewhat random curve for testing.
	void randomize(const PYXIcosIndex& A, const PYXIcosIndex& B, int nRes, int nDivisions);

	//! Make a somewhat random curve for testing.
	void randomize(int nRes, int nDivisions)
	{
		PYXIcosIndex A, B;
		A.randomize(nRes);
		B.randomize(nRes);
		randomize(A, B, nRes, nDivisions);
	}

private:

	//! Typedef for vector of indices
	typedef std::vector<PYXIcosIndex> IndexVector;

private:

	//! The vector of nodes
	IndexVector m_vecNode;

	//! Count of nodes dropped.
	int m_nNodeDrop;

	//! Curve Length in radians
	double m_length;

private:

	/*!
	The PYXCurveIterator iterates over all the cells in a PYXCurve.
	*/
	//! Iterates over the cells in a curve
	class PYXCurveIterator : public PYXIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<PYXCurveIterator> create(const IndexVector& vecNode)
		{
			return PYXNEW(PYXCurveIterator, vecNode);
		}

		//! Constructor
		PYXCurveIterator(const IndexVector& vecNode);

		//! Destructor
		virtual ~PYXCurveIterator() {;}

		//! Move to the next cell.
		virtual void next();

		//! Calculate the next cell.
		bool getNextIndex();

		//! See if we have covered all the cells.
		virtual bool end() const;

		/*!
		Get the PYXIS index for the current cell.

		\return	The index.
		*/
		//! Get the PYXIS index for the current cell.
		virtual const PYXIcosIndex& getIndex() const { return m_index; }

		/*!
		\return	The direction of movement (knDirectionZero if no previous index).
		*/
		//! Returns the movement direction from the previous index.
		PYXMath::eHexDirection getDirection() const { return m_nTheDirection; }

	private:

		//! First destination, so we can check back to examine previous destinations.
		IndexVector::const_iterator m_itDestBegin;

		//! The index to which we are moving
		IndexVector::const_iterator m_itDest;

		//! The iterator end condition
		IndexVector::const_iterator m_itEnd;

		//! The current index.
		PYXIcosIndex m_index;

		//! The destination index.
		PYXIcosIndex m_indexDest;

		//! The next calculated cell.
		PYXIcosIndex m_nextIndex;

		//! Vector to hold the last 3 or so calculated cells on the curve.  Used for culling bad points.
		std::vector<PYXIcosIndex> m_vecIndex;
			
		//! Vector to hold the last 3 or so calculated directions for cells on the curve.
		std::vector<PYXMath::eHexDirection> m_vecDir;

		//! The direction move from the previous index (knDirectionZero if no previous index).
		PYXMath::eHexDirection m_nDir;

		//! The direction (used in getDirection) move from the previous index (knDirectionZero if no previous index).
		PYXMath::eHexDirection m_nTheDirection;

		// Variables for great circle arc
		std::auto_ptr<GreatCircleArc> m_apGca;
		double m_fPos;
		double m_fStep;
		double m_fStepDist;
	};
};

//! Allows PYXCurve to be written to streams.
PYXLIB_DECL std::ostream& operator <<(	std::ostream& out,
										const PYXCurve& pyxcurve	);

//! Allows PYXCurve to be read from streams.
PYXLIB_DECL std::istream& operator >>(	std::istream& input, 
										PYXCurve& pyxcurve	);

#endif // guard
