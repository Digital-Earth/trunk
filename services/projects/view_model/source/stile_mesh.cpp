/******************************************************************************
stile_mesh.cpp

begin		: 2010-07-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "stile_mesh.h"

// view model includes
#include "performance_counter.h"
#include "tuv.h"
#include "ray.h"

// pyxlib includes
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/sphere_math.h"


const unsigned short STileMesh::m_triStripIndices[(STileMesh::knMeshSize-1)*(STileMesh::knMeshSize*2)] =
{
	10,  0, 11,  1, 12,  2, 13,  3, 14,  4, 15,  5, 16,  6, 17,  7, 18,  8, 19,  9,
	20, 10, 21, 11, 22, 12, 23, 13, 24, 14, 25, 15, 26, 16, 27, 17, 28, 18, 29, 19,
	30, 20, 31, 21, 32, 22, 33, 23, 34, 24, 35, 25, 36, 26, 37, 27, 38, 28, 39, 29,
	40, 30, 41, 31, 42, 32, 43, 33, 44, 34, 45, 35, 46, 36, 47, 37, 48, 38, 49, 39,
	50, 40, 51, 41, 52, 42, 53, 43, 54, 44, 55, 45, 56, 46, 57, 47, 58, 48, 59, 49,
	60, 50, 61, 51, 62, 52, 63, 53, 64, 54, 65, 55, 66, 56, 67, 57, 68, 58, 69, 59,
	70, 60, 71, 61, 72, 62, 73, 63, 74, 64, 75, 65, 76, 66, 77, 67, 78, 68, 79, 69,
	80, 70, 81, 71, 82, 72, 83, 73, 84, 74, 85, 75, 86, 76, 87, 77, 88, 78, 89, 79,
	90, 80, 91, 81, 92, 82, 93, 83, 94, 84, 95, 85, 96, 86, 97, 87, 98, 88, 99, 89
};

const unsigned short STileMesh::m_triStripIndicesPent[STileMesh::knMeshSize*STileMesh::knMeshSize - 1] =
{
	 0,  1, 10, 11, 19, 20, 27, 28, 34, 35, 40, 41, 45, 46, 49, 50, 52, 53, 54,
	 1,  2, 11, 12, 20, 21, 28, 29, 35, 36, 41, 42, 46, 47, 50, 51, 53,
     2,  3, 12, 13, 21, 22, 29, 30, 36, 37, 42, 43, 47, 48, 51,
	 3,  4, 13, 14, 22, 23, 30, 31, 37, 38, 43, 44, 48,
	 4,  5, 14, 15, 23, 24, 31, 32, 38, 39, 44,
	 5,  6, 15, 16, 24, 25, 32, 33, 39,
	 6,  7, 16, 17, 25, 26, 33,
	 7,  8, 17, 18, 26,
	 8,  9, 18
};

const unsigned short STileMesh::m_triStripInfo[STileMesh::knMeshSize] =
{
	9, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

const unsigned short STileMesh::m_triStripInfoPent[STileMesh::knMeshSize] =
{
	9, 19, 17, 15, 13, 11, 9, 7, 5, 3
};



STileMesh::STileMesh(const PYXIcosIndex & index) : m_origin(0,0,0), m_index(index), m_bIsHexagon(index.isHexagon())
{
	if (hasOrigin())
	{
		m_origin = CmlConvertor::toVec3(m_index);
	}

	initMesh();
}

STileMesh::~STileMesh()
{	
}


void STileMesh::initMesh()
{
	STILE_COORD_TYPE* p[3] = {m_mesh[0],m_mesh[1],m_mesh[2]};

	// Set up cursors in v dir.
	PYXCursor cv[3];
	cv[0].reset(m_index, static_cast<PYXMath::eHexDirection>(m_index.isNorthern() ? 2 : 5));
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[1] = cv[0];
	cv[1].left();
	cv[1].left();
	cv[2] = cv[1];
	cv[2].left();
	cv[2].left();

	for (int v = 0; v != knMeshSize; ++v)
	{
		// Set up cursors in u dir.
		PYXCursor cu[3] = { cv[0], cv[1], cv[2] };
		cu[0].right();
		if (isHexagon())
		{
			cu[0].right();
		}
		cu[1].right();
		cu[1].right();
		cu[2].right();
		cu[2].right();

		const int upent = knMeshSize - v;

		for (int u = 0; u != knMeshSize; ++u)
		{
			if (isHexagon() || u < upent)
			{
				indexToVertex(cu[0].getIndex(),p[0]);
				p[0]+=3;
				cu[0].forward();
			}

			indexToVertex(cu[1].getIndex(),p[1]);
			p[1]+=3;
			cu[1].forward();

			indexToVertex(cu[2].getIndex(),p[2]);
			p[2]+=3;			
			cu[2].forward();
		}

		cv[0].forward();
		cv[1].forward();
		cv[2].forward();
	}
}

void STileMesh::indexToVertex(const PYXIcosIndex & index,STILE_COORD_TYPE * vec)
{
	vec3 cord(CmlConvertor::toVec3(index));
	CmlConvertor::fromVec3WithOrigin(cord,m_origin,vec);
}

void STileMesh::scaleVertex(STILE_COORD_TYPE * vec,double scale)
{
	vec[0] = static_cast<STILE_COORD_TYPE>((static_cast<double>(vec[0]) + m_origin[0]) * scale - m_origin[0]);
	vec[1] = static_cast<STILE_COORD_TYPE>((static_cast<double>(vec[1]) + m_origin[1]) * scale - m_origin[1]);
	vec[2] = static_cast<STILE_COORD_TYPE>((static_cast<double>(vec[2]) + m_origin[2]) * scale - m_origin[2]);
}


#define GET_COORD(TARGET,T,INDEX) \
	TARGET[0] = m_mesh[T][(INDEX)*3+0]+m_origin[0];\
	TARGET[1] = m_mesh[T][(INDEX)*3+1]+m_origin[1];\
	TARGET[2] = m_mesh[T][(INDEX)*3+2]+m_origin[2];

bool STileMesh::intersects(const Ray & ray,double & resultTime)
{	
	vec3 center;
	GET_COORD(center,0,0);

	vec3 edge;
	GET_COORD(edge,0,9);
	
	//do a fast sphere BBox check before we go over the mesh, radius is 1.5 just to be sure
	if (! ray.intersectsWithSphere(center,(center-edge).length()*1.5))
	{
		return false;
	}

	bool   foundIntersection=false;
	double minTime = 0;

	for(int t=0;t<3;t++)
	{
		const unsigned short * index  = &m_triStripIndices[0];
		const unsigned short * strips = &m_triStripInfo[0];


		vec3 v0,v1,v2;

		if (t==0 && ! isHexagon())
		{
			index = m_triStripIndicesPent;
			strips = m_triStripInfoPent;
		}

		for(int strip=0;strip<strips[0];strip++)
		{
			GET_COORD(v0,t,index[0]);
			GET_COORD(v1,t,index[1]);
			GET_COORD(v2,t,index[2]);
			for(int i=0;i<strips[strip+1]-2;i++)
			{
				double time;
								
				if (ray.intersectsWithTriangle(v0,v1,v2,time))
				{
					if (time>0 && (!foundIntersection || minTime > time))
					{
						minTime = time;
						foundIntersection = true;
					}
				}

				v0 = v1;
				v1 = v2;
				GET_COORD(v2,t,index[3]);

				index++;
			}
			index+=2;
		}
	}

	resultTime = minTime;

	return foundIntersection;
}

bool STileMesh::getElevation(const CoordLatLon & ll,double & elevation)
{
	//find current intex in resultion 11 - which suppose to be pixel-size resolution	
	PYXIcosIndex elevationTargetIndex;
	SnyderProjection::getInstance()->nativeToPYXIS(ll,&elevationTargetIndex,m_index.getResolution()+11);

	//convert into TUV cordinate inside STile
	int t,u,v;
	tuvFromIndex(m_index,elevationTargetIndex,t,u,v);

	bool isPentagonCase = false;

	if (t==0 && ! isHexagon())
	{
		isPentagonCase = true;
		v-=u;	
	}
	
	//covert back to resolution 5
	int u_res5_low = (int)(floor(u/27.0));
	int v_res5_low = (int)(floor(v/27.0));
	int u_res5_high = (int)(ceil(u/27.0));
	int v_res5_high = (int)(ceil(v/27.0));
	float u_res5_offset = (u%27)/27.0f;
	float v_res5_offset = (v%27)/27.0f;

	if (u_res5_low < 0 || u_res5_high > 9 || v_res5_low < 0 || v_res5_high > 9)
	{		
		return false;
	}

	//get right vertex
	vec3 coord;
	
	int vertex_offset_low = 0;
	int vertex_offset_high= 0;

	if (! isPentagonCase )
	{
		vertex_offset_low = v_res5_low*10;
		vertex_offset_high = v_res5_high*10;
	}
	else
	{
		for(int i=0;i<v_res5_low;i++)
		{
			vertex_offset_low +=10-i;
		}

		for(int i=0;i<v_res5_high;i++)
		{
			vertex_offset_high +=10-i;
		}
	}

	//convert back into double
	for (int i=0;i<3;i++)
	{		
		if (isPentagonCase && v_res5_high+u_res5_high>=9)
		{
			coord[i] = cml::bilerp(m_mesh[t][(vertex_offset_low  + u_res5_low )*3+i],
								   m_mesh[t][(vertex_offset_low  + u_res5_high)*3+i],
								   m_mesh[t][(vertex_offset_high + u_res5_low )*3+i],
								   m_mesh[t][(vertex_offset_high + u_res5_low )*3+i], //NOTE: using u_res5_low insted of u_res5_high
								   u_res5_offset,v_res5_offset);
		}
		else
		{
			coord[i] = cml::bilerp(m_mesh[t][(vertex_offset_low  + u_res5_low )*3+i],
								   m_mesh[t][(vertex_offset_low  + u_res5_high)*3+i],
								   m_mesh[t][(vertex_offset_high + u_res5_low )*3+i],
								   m_mesh[t][(vertex_offset_high + u_res5_high)*3+i],
								   u_res5_offset,v_res5_offset);
		}
	}	
	
	if (hasOrigin())
	{
		coord[0]+=m_origin[0];
		coord[1]+=m_origin[1];
		coord[2]+=m_origin[2];
	}

	elevation = (coord.length()-1) * SphereMath::knEarthRadius;
	return true;
}
