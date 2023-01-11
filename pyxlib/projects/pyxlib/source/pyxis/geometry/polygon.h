#ifndef PYXIS__GEOMETRY__POLYGON_H
#define PYXIS__GEOMETRY__POLYGON_H
/******************************************************************************
polygon.h

begin		: 2006-01-09
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/index.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/utility/coord_3d.h"

/*!
This class implements a spherical polygon on the surface of the reference
sphere.

The polygon has an implicit exterior ring. Its vertices must be specified in
clockwise order. The first vertex need not be repeated as the last.

Each call to closeRing() finishes a ring and begins a new interior ring.
Their vertices must be specified in counter-clockwise order. The first vertex
need not be repeated as the last.

It is assumed that the normal of the first line segment is exterior to the
polygon. If this is not true, setExteriorPoint() should be called manually.
*/
//! A spherical polygon on the reference sphere.
class PYXLIB_DECL PYXPolygon : public PYXGeometry
{
public:

	//! Classification with regard to spherical polygon.
	enum eClass
	{
		knOutside = 0,	//!< Outside the spherical polygon.
		knInside = 1,	//!< Inside the spherical polygon.
		knBoundary = 2	//!< On the boundary of the spherical polygon.
	};

public:

	//! Test method
	static void test();

	//! Create a geometry.
	static PYXPointer<PYXPolygon> create()
	{
		return PYXNEW(PYXPolygon);
	}

	/*!
	Create a copy of the geometry.

	\return	A copy of the geometry.
	*/
	//! Create a copy of the geometry.
	static PYXPointer<PYXPolygon> create(const PYXPolygon& rhs)
	{
		return PYXNEW(PYXPolygon, rhs);
	}

	//! Constructor
	PYXPolygon() : m_w(0, 0, 0) {}

	//! Copy constructor
	PYXPolygon(const PYXPolygon& rhs) :
		m_vecVertex(rhs.m_vecVertex),
		m_vecRing(rhs.m_vecRing),
		m_vecPoint(rhs.m_vecPoint),
		m_vecLine(rhs.m_vecLine),
		m_w(rhs.m_w),
		m_spGeometry(rhs.m_spGeometry ? rhs.m_spGeometry->clone() : PYXPointer<PYXGeometry>())
	{
	}

	//! Destructor
	virtual ~PYXPolygon() {}

	//! Equality operator
	bool operator ==(const PYXPolygon& rhs) const;

	//! Inequality operator.
	bool operator !=(const PYXPolygon& rhs) const {return !(*this == rhs);}

	//! Less than operator
	bool operator <(const PYXPolygon& rhs) const;

	/*!
	Create a copy of the geometry.

	\return	A copy of the geometry.
	*/
	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	/*!
	Is the geometry empty?

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	//! Is the geometry empty?
	virtual bool isEmpty() const {return m_vecVertex.empty();}

	/*!
	Clears the geometry so it is empty.
	*/
	void clear();

	//! Get the cell resolution.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTileCollection& collection) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTile& tile) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Get the intersection of this geometry's boundary and the specified geometry.
	PYXPointer<PYXGeometry> boundaryIntersection(const PYXTile& tile) const;

	//! Does the specified geometry intersect with this one?
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const; 

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	//! Add a vertex to the curve.
	void addVertex(const PYXIcosIndex& index);

	//! Closes the current ring.
	void closeRing() const;

	//! Sets the exterior point.
	void setExteriorPoint(const PYXIcosIndex& index);

	//! Return the number of vertexes for the specified ring.
	int getVertexCount(int nRing) const
	{
		return m_vecRing[nRing] - (nRing == 0 ? 0 : m_vecRing[nRing - 1]);
	}

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

public:

	//! Module set-up.
	static void initStaticData();

	//! Module tear-down.
	static void freeStaticData();

private:

	//! Creates intermediate data structures.
	void createIntermediateDataStructures() const;

	//! Destroys intermediate data structures.
	void destroyIntermediateDataStructures() const;

	//! Classify a cell with regard to this spherical polygon.
	eClass classifyCellToPolygon(const PYXIcosIndex& index) const;

	//! Classify a point with regard to this spherical polygon.
	eClass classifyPointToPolygon(const PYXIcosIndex& index) const;

	//! Classify a cell with regard to this spherical polygon.
	eClass classifyCellToPolygonBoundary(const PYXIcosIndex& index) const;

	//! Create the geometry for this spherical polygon.
	void createGeometry() const;

	//! Traversal against polygon. Note that vec is modified!
	PYXPointer<PYXTileCollection> intersectionImpl(std::vector<PYXIcosIndex>& vec, int nTargetResolution) const;

	//! Traversal against boundary. Note that vec is modified!
	PYXPointer<PYXTileCollection> boundaryIntersectionImpl(std::vector<PYXIcosIndex>& vec, int nTargetResolution) const;

private:

	//! Typedef for vector of indices.
	typedef std::vector<PYXIcosIndex> IndexVector;

	//! POD type for line segments.
	struct Line
	{
		//! Convenience constructor.
		Line(int nU, int nV, double fDirDv, double fLineDw, const PYXCoord3DDouble& normal, const PYXCoord3DDouble& dir)
			: m_nU(nU), m_nV(nV), m_fDirDv(fDirDv), m_fLineDw(fLineDw), m_normal(normal), m_dir(dir)
		{}

		int m_nU;					//!< Start of line segment (index into m_vecVertex).
		int m_nV;					//!< End of line segment (index into m_vecVertex).
		double m_fDirDv;			//!< Distance to u (project to dir).
		double m_fLineDw;			//!< Distance to line (project to line normal).
		PYXCoord3DDouble m_normal;	//!< Line normal (u cross v then normalize).
		PYXCoord3DDouble m_dir;		//!< Line direction vector (normal cross u then normalize).
	};

	//! Allows PYXLibModel to initialize the static data.
	friend class PYXLibModel;

private:

	//! The vertex indices; does not repeat the first vertex as the last.
	mutable IndexVector m_vecVertex;

	//! For each ring, the "one past the end" entry in m_vecVertex.
	mutable std::vector<int> m_vecRing;

	//! The vertex 3D points on the reference sphere.
	mutable std::vector<PYXCoord3DDouble> m_vecPoint;

	//! The line segments (great circle arcs) on the reference sphere.
	mutable std::vector<Line> m_vecLine;

	//! 3D point w is guaranteed to be exterior to the spherical polygon.
	mutable PYXCoord3DDouble m_w;

	//! The geometry (tile collection).
	mutable PYXPointer<PYXGeometry> m_spGeometry;
};

#endif // guard
