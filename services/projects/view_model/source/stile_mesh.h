#pragma once
#ifndef VIEW_MODEL__STILE_MESH_H
#define VIEW_MODEL__STILE_MESH_H
/******************************************************************************
stile_mesh.h

begin		: 2010-07-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "cml_utils.h"

// pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/coord_lat_lon.h"


// standard includes
#include <map>
#include <vector>

#define STILE_COORD_TYPE float

//forward declearation
class Ray;

class STileMesh : public PYXObject
{
protected:
	static const int knMeshSize = 10;
	static const int knLocalResMin = 23;

	static const unsigned short m_triStripIndices[(knMeshSize-1)*(knMeshSize*2)];
	static const unsigned short m_triStripInfo[knMeshSize];
	static const unsigned short m_triStripIndicesPent[knMeshSize*knMeshSize - 1];
	static const unsigned short m_triStripInfoPent[knMeshSize];

	//! Root index of this mesh
	PYXIcosIndex m_index;

	//! Origin for the mesh.
	vec3 m_origin;

	bool m_bIsHexagon;

	STILE_COORD_TYPE m_mesh[3][3*knMeshSize*knMeshSize];

public:
	bool hasOrigin() const
	{
		return knLocalResMin <= m_index.getResolution() + 11;
	}

	const vec3 & getOrigin() const
	{
		return m_origin;
	}

	bool isHexagon() const
	{
		return m_bIsHexagon;
	}

	static PYXPointer<STileMesh> create(const PYXIcosIndex & index)
	{
		return PYXNEW(STileMesh,index);
	}

	STileMesh(const PYXIcosIndex & index);

	virtual ~STileMesh();

public:
	bool intersects(const Ray & ray,double & time);
	bool getElevation(const CoordLatLon & ll,double & elevation);

public:
	void initMesh();
	void indexToVertex(const PYXIcosIndex & index,STILE_COORD_TYPE * vec);
	void scaleVertex(STILE_COORD_TYPE * vec,double scale);

	STILE_COORD_TYPE * getFirstVertex(int triIndex = 0)
	{
		return m_mesh[triIndex];
	}
};

#endif
