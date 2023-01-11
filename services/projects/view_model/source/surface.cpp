/******************************************************************************
surface.cpp

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "surface.h"
#include "exceptions.h"

#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/cursor.h"
#include "pyxis/utility/string_utils.h"

#include <cassert>
#include <set>
#include <map>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////
// SphereBBox
/////////////////////////////////////////////////////////////////////////////////////////

SphereBBox::SphereBBox() : m_center(0,0,0),m_radius(0)
{
}

SphereBBox::SphereBBox(const PYXCoord3DDouble & center,const double & radius) : m_radius(radius)
{
	m_center[0] = center[0];
	m_center[1] = center[1];
	m_center[2] = center[2];
}

SphereBBox::SphereBBox(const SphereBBox & other) : m_center(other.m_center),m_radius(other.m_radius)
{
}

SphereBBox::SphereBBox(const PYXIcosIndex & cell)
{
	const double kfFrustumFactor = 1.17;

	assert(2 <= cell.getResolution()); // not sure exactly what the lower limit is but 2 sounds good

	CoordLatLon ll;
	SnyderProjection::getInstance()->pyxisToNative(cell, &ll);
	PYXCoord3DDouble center = SphereMath::llxyz(ll);
	m_center[0] = center[0];
	m_center[1] = center[1];
	m_center[2] = center[2];
		
	m_radius = PYXMath::calcCircumRadius(cell.getResolution()) * Icosahedron::kfCentralAngle * kfFrustumFactor;
}

SphereBBox::SphereBBox(const PYXIcosIndex & cellA,const PYXIcosIndex & cellB,bool includeCellRadius)
{
	SphereBBox a(cellA),b(cellB);

	m_center = (a.m_center + b.m_center)/2;
	
	m_radius = (a.m_center-b.m_center).length();

	if (includeCellRadius)
	{
		m_radius += a.m_radius + b.m_radius;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Surface::Patch::Key
/////////////////////////////////////////////////////////////////////////////////////////

Surface::Patch::Key::Key()
{
}

Surface::Patch::Key::Key(const std::string & key) : m_key(key)
{
	assert(m_key.size()>=2 && "Key is to short");
	assert(getPatchIndex(0)<90 && "First resolution index is invalid");
	assert(validatePatchsIndices() && "Patch indecis is invalid");
}

Surface::Patch::Key::Key(int firstIndex) : m_key("00")
{
	assert(firstIndex>=0 && firstIndex<90 && "First resolution index is invalid");
	
	m_key[0] = firstIndex/10+'0';
	m_key[1] = firstIndex%10+'0';	
}

Surface::Patch::Key::Key(const Key & key,int index)
{
	assert(index >=0 && index <=8 && "Index is invalid");
	m_key = key.m_key + StringUtils::toString(index);
}

Surface::Patch::Key::Key(const Surface::Patch & patch)
{
	if (patch.getParent())
	{
		const Key & parentKey = patch.getParent()->getKey();
		m_key = parentKey.m_key + StringUtils::toString(patch.m_index);
	}
	else
	{
		m_key = StringUtils::toString(patch.m_index);
		if (m_key.size() < 2)
		{
			m_key = '0'+m_key;
		}
	}
}

Surface::Patch::Key::Key(const Surface::Patch::Key & key) : m_key(key.m_key)
{
}

const Surface::Patch::Key & Surface::Patch::Key::operator =(const Surface::Patch::Key & key)
{
	m_key = key.m_key;
	return *this;
}

int Surface::Patch::Key::getPatchIndex(int resolution) const
{
	assert(resolution < (int)(m_key.size())-1 && "resolution is out of range");

	if (resolution==0)
	{
		return (m_key[0]-'0')*10 + m_key[1]-'0';
	}
	else
	{
		return m_key[resolution+1]-'0';
	}
}

bool Surface::Patch::Key::validatePatchsIndices() const
{
	for(unsigned int i=2;i<m_key.size();i++)
	{
		//indices are from 0 to 8 (9 options)
		if (m_key[i]<'0' || m_key[i]>'8')
		{
			return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Surface::Patch
/////////////////////////////////////////////////////////////////////////////////////////

Surface::Patch::Patch(Surface & surface, const PYXRhombus & rhombus, const int & index) 
  : m_parent(NULL),
    m_surface(surface),
	m_rhombus(rhombus),
	m_index(index),	
	m_bbox(rhombus.getIndex(1),rhombus.getIndex(3)),
	borrowedElevationsDepth(-1)
{
	m_sizeOnScreenRdius = m_bbox.getRadius();
	m_key = Key(*this);	

	m_surfaceVertices = m_surface.m_vertexSet.addPatch(*this);
	m_surfaceEdges = m_surface.m_edgeSet.addPatch(*this);;

	coords = Coord3dData::create();
	coords->generate(m_rhombus);
}

Surface::Patch::Patch(Surface::Patch & parent, const int & index) 
  : m_parent(&parent),
    m_surface(parent.m_surface),
	m_rhombus(parent.getRhombus().getSubRhombus(index)),
	m_index(index),
	m_bbox(m_rhombus.getIndex(1),m_rhombus.getIndex(3)),
	borrowedElevationsDepth(-1)
{
	m_sizeOnScreenRdius = m_bbox.getRadius();
	m_key = Key(*this);

	m_surfaceVertices = m_surface.m_vertexSet.addPatch(*this);
	m_surfaceEdges = m_surface.m_edgeSet.addPatch(*this);;

	coords = Coord3dData::create();
	coords->generate(m_rhombus);
}

Surface::Patch::~Patch()
{
}

void Surface::Patch::setState(const State & newState)
{
	bool refreshVertices = m_state.neetToRefereshSurfaceVertices(newState);
	bool borrowPriotiry = m_state.hasPriorityChange(newState);
	double oldPriority = m_state.m_priority;
	
	m_state = newState;

	if (refreshVertices)
	{
		refreshVerticesCount();
		borrowPriotiry = true;
	}
	
	if (borrowPriotiry)
	{
		for(unsigned int i=0;i < m_surfaceVertices.size(); i++)
		{
			if (m_surfaceVertices[i]->getMaxPriority() == oldPriority)
			{
				m_surfaceVertices[i]->setMaxPriority(oldPriority*0.2);
			}

			if (m_state.m_priority < m_surfaceVertices[i]->getMaxPriority() * 0.2)
			{
				m_state.m_priority = m_surfaceVertices[i]->getMaxPriority() * 0.2;
			}

			else if (m_state.m_priority > m_surfaceVertices[i]->getMaxPriority())
			{
				//enable slow priority propogation
				m_surfaceVertices[i]->setMaxPriority(m_state.m_priority);
			}
		}
	}
}

void Surface::Patch::refreshVerticesCount()
{
	for(unsigned int i=0;i < m_surfaceVertices.size(); i++)
	{
		m_surfaceVertices[i]->refreshCount();
	}
}

const PYXRhombus & Surface::Patch::getRhombus() const
{
	return m_rhombus;
}

PYXPointer<Surface::Patch> Surface::Patch::getParent()
{
	return PYXPointer<Surface::Patch>(m_parent);
}

PYXPointer<const Surface::Patch> Surface::Patch::getParent() const
{
	return PYXPointer<Surface::Patch>(m_parent);
}

Surface::Patch * Surface::Patch::getParentPtr() const
{
	return m_parent;
}

Surface::Patch * Surface::Patch::getSubPatchPtr(int index) const
{
	assert(index>=0&&index<=8&&"index is out of range");
	return m_subPatches[index].get();
}


PYXPointer<Surface::Patch> Surface::Patch::getSubPatch(int index)
{
	assert(index>=0&&index<=8&&"index is out of range");
	return m_subPatches[index];
}

PYXPointer<const Surface::Patch> Surface::Patch::getSubPatch(int index) const
{
	assert(index>=0&&index<=8&&"index is out of range");
	return m_subPatches[index];
}

const Surface::Patch::Key & Surface::Patch::getKey() const
{
	return m_key;
}

Surface & Surface::Patch::getSurface()
{
	return m_surface;
}

const Surface & Surface::Patch::getSurface() const
{
	return m_surface;
}

void Surface::Patch::divide()
{
	if (m_subPatches.size()>0)
		return;

	getSurface().getBeforePatchDividedNotifer().notify(Surface::Event::create(this));	

	{
		boost::recursive_mutex::scoped_lock lock(m_surface.m_patchesMutex);
		for(int index=0;index<9;index++)
		{
			m_subPatches.push_back(PYXNEW(Surface::Patch,*this,index));
		}
	
		if (m_parent != 0)
		{
			m_parent->m_state.m_subPatchDividedCount++;
			m_parent->refreshVerticesCount();
		}

		m_state.m_divided = true;

		refreshVerticesCount();
	}

	getSurface().getAfterPatchDividedNotifer().notify(Surface::Event::create(this));
}

void notifyAboutHiddenPatches(PYXPointer<Surface::Patch> patch)
{	
	patch->getSurface().getPatchBecomeNotVisible().notify(Surface::Event::create(patch));		
}

void Surface::Patch::unify()
{
	if (m_subPatches.size()==0)
		return;
	
	getSurface().getBeforePatchUnifiedNotifer().notify(Surface::Event::create(this));
	
	Surface::Visitor::visitVisibleChildren(this,boost::bind(notifyAboutHiddenPatches,_1));		

	{
		boost::recursive_mutex::scoped_lock lock(m_surface.m_patchesMutex);

		for(int index=0;index<9;index++)
		{
			m_surface.m_vertexSet.removePatch(*m_subPatches[index]);
			m_surface.m_edgeSet.removePatch(*m_subPatches[index]);
			m_subPatches[index]->m_parent = NULL;
		}

		if (m_parent != 0)
		{
			m_parent->m_state.m_subPatchDividedCount--;
			m_parent->refreshVerticesCount();
		}

		m_subPatches.clear();
		m_state.m_divided = false;
		m_state.m_visiblityBlock = true; //return the visible block for future divide operations
	}

	refreshVerticesCount();

	getSurface().getAfterPatchUnifiedNotifer().notify(Surface::Event::create(this));
}

void Surface::Patch::removeVisiblityBlock() 
{
	m_state.m_visiblityBlock = false; 

	getSurface().getAfterPatchVisibliyBlockRemoved().notify(Surface::Event::create(this));
}

bool Surface::Patch::testAllNeighborPatches(const Surface::Patch::BoolVertexPropertyFunction & func) const
{
	for(unsigned int i=0;i < m_surfaceVertices.size(); i++)
	{
		const Surface::Vertex & vertex = *m_surfaceVertices[i];
	
		if (!(vertex.*func)())
		{
			return false;
		}
	}
	return true;
}

bool Surface::Patch::testAllNeighborPatches(const Surface::Patch::BoolPropertyFunction & func) const
{
	for(unsigned int i=0;i < m_surfaceVertices.size(); i++)
	{
		const Surface::Vertex & vertex = *m_surfaceVertices[i];

		if (!vertex.hasAllPatches())
		{
			return false;
		}

		for(auto & patch : vertex.getPatches())
		{
			if (patch != this && ! ((*patch).*func)())
			{
				return false;
			}
		}
	}
	return true;
}

bool Surface::Patch::testAllNeighborPatchesOfVertex(int index,const BoolPropertyFunction & func) const
{
	const Surface::Vertex & vertex = *m_surfaceVertices[index];
	if (!vertex.hasAllPatches())
	{
		return false;
	}

	for(auto & patch : vertex.getPatches())
	{
		if (patch != this && ! ((*patch).*func)())
		{
			return false;
		}
	}
	return true;
}

bool Surface::Patch::hasNeighborPatch(const Surface::Patch::BoolPropertyFunction & func) const
{
	for(const auto & edge : m_surfaceEdges)
	{
		const auto & neighbor = edge->getOtherSide(this);

		if (neighbor != nullptr && ((*neighbor).*func)())
		{
			return true;
		}
	}

	return false;
}

bool Surface::Patch::hasFullVertex(int index) const
{
	return m_surfaceVertices[index]->hasAllPatches();
}

bool Surface::Patch::hasFullVisibleVertex(int index) const
{
	return m_surfaceVertices[index]->doesAllPatchesNotHidden();
}

bool Surface::Patch::hasAllNeighbors() const
{
	for(unsigned int i=0;i < m_surfaceVertices.size(); i++)
	{
		const Surface::Vertex & vertex = *m_surfaceVertices[i];
	
		if (!vertex.hasAllPatches())
		{
			return false;
		}
	}
	return true;
}


double Surface::Patch::borrowPriorityFromSubPatchs()
{
	if (!isDivided())
	{
		return 0;
	}

	double maxPriority = m_state.m_priority;

	for(int i=0;i<9;i++)
	{
		double childPriority = getSubPatchPtr(i)->m_state.m_priority * 0.2;
		if (maxPriority < childPriority)
		{
			maxPriority = childPriority;
		}
	}

	return maxPriority;
}

bool Surface::Patch::needsToBeDivided() const
{
	return isVisible() && isTooBig();
}

bool Surface::Patch::canBeDivided() const
{
	if (!hasAllNeighbors())
	{
		return false;
	}	
	return !hasNeighborPatch(&Surface::Patch::isHidden);	
}

bool Surface::Patch::needsToBeUnified() const
{
	return ! needsToBeDivided();
}

bool Surface::Patch::canBeUnified() const
{
	if (hasDividedSubPatch())
	{
		return false;
	}
	if (hasNeighborPatch(&Surface::Patch::hasDividedSubPatch) || hasNeighborPatch(&Surface::Patch::needsToBeDivided))
	{
		return false;
	}
	return true;
}


PYXIcosIndex Surface::Patch::getIndexFromUV(const vec2 & uv,int resDepth) const
{
	assert(resDepth%2==0);

	PYXCursor c(m_rhombus.getIndex(0),m_rhombus.getDirection(0));

	for(int i=0;i<resDepth;i++)
	{
		c.zoomIn();
	}
	c.forward(static_cast<int>(m_rhombus.getUVMax(resDepth)*uv[0]));
	c.left();
	c.forward(static_cast<int>(m_rhombus.getUVMax(resDepth)*uv[1]));

	return c.getIndex();
}

vec2 Surface::Patch::getUVFromIndex(const PYXIcosIndex & index) const
{
	vec2 uv;

	if (m_rhombus.isInside(index,&uv[0],&uv[1]))
	{
		return uv;
	}
	else
	{
		return vec2(0.0,0.0);
	}
}

vec2 Surface::Patch::getUVFromLocation(const vec3 & coord) const
{
	//make it the most accurate we can
	return getUVFromIndex(CmlConvertor::toPYXIcosIndex(coord,m_rhombus.getIndex(0).getResolution()+8));
}

bool Surface::Patch::intersects(const PYXIcosIndex & index) const
{
	return m_rhombus.intersects(index);
}

void Surface::Patch::updateVertices()
{
	auto elev = elevations?elevations:borrowedElevations;
	auto newVertices = Surface::Patch::VertexBuffer::create();
	newVertices->zero = CmlConvertor::toVec3(coords->data[0][0]);

	if (elev)
	{
		newVertices->generate(coords,elev);
	}
	else 
	{
		newVertices->generate(coords);
	}

	newVertices->updateBBox();

	vertices = newVertices;
	m_bbox = vertices->bbox;
}

void Surface::Patch::updateElevation(const PYXPointer<ElevationData> newElevation)
{
	boost::recursive_mutex::scoped_lock lock(m_surface.m_patchesMutex);

	elevations = newElevation;
	matchElevationFromNeighbors();
	updateVertices();
}

void Surface::Patch::matchElevationFromNeighbors()
{
	for(auto & edge : m_surfaceEdges)
	{
		auto neighborPatch = edge->getOtherSide(this);
		if (neighborPatch != nullptr)
		{
			if (elevations)
			{
				//propagate this patch's elevation to neighbor patches...
				if (neighborPatch->elevations)
				{
					edge->copyEdgeElevation(this,*elevations,neighborPatch,*neighborPatch->elevations);
					neighborPatch->updateVertices();
				}
				if (neighborPatch->borrowedElevations)
				{
					edge->copyEdgeElevation(this,*elevations,neighborPatch,*neighborPatch->borrowedElevations);
					neighborPatch->updateVertices();
				}
			}
			else if (borrowedElevations)
			{
				if (neighborPatch->elevations)
				{
					//copy neighbor patch's elevation to this patch's borrowed elevation
					edge->copyEdgeElevation(neighborPatch,*neighborPatch->elevations,this,*borrowedElevations);
				}
				if (neighborPatch->borrowedElevations)
				{
					//propogate this patch borrowed elevation to neighbor patches...
					edge->copyEdgeElevation(this,*borrowedElevations,neighborPatch,*neighborPatch->borrowedElevations);
					neighborPatch->updateVertices();
				}
			}
		}
	}
}

void Surface::Patch::borrowElevation()
{
	boost::recursive_mutex::scoped_lock lock(m_surface.m_patchesMutex);

	if (elevations)
	{
		borrowedElevationsDepth = 0;
		return;
	}

	if (m_parent != nullptr)
	{
		if (!m_parent->elevations)
		{
			m_parent->borrowElevation();
		} 
		if (m_parent->elevations)
		{
			if (borrowedElevationsDepth != 1)
			{
				borrowedElevations = m_parent->elevations->zoomIn(m_key.getLastIndex());
				borrowedElevationsDepth = 1;
				matchElevationFromNeighbors();
				updateVertices();
			}
		} 
		else if (m_parent->borrowedElevations)
		{
			if (borrowedElevationsDepth != m_parent->borrowedElevationsDepth+1)
			{
				borrowedElevations = m_parent->borrowedElevations->zoomIn(m_key.getLastIndex());
				borrowedElevationsDepth = m_parent->borrowedElevationsDepth+1;
				matchElevationFromNeighbors();
				updateVertices();
			}
		}
	}
}

const PYXPointer<Surface::Patch::VertexBuffer> & Surface::Patch::getVertices()
{
	boost::recursive_mutex::scoped_lock lock(m_surface.m_patchesMutex);
	if (!vertices)
	{
		updateVertices();
	}
	return vertices;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Coord3dData
/////////////////////////////////////////////////////////////////////////////////////////

void Surface::Patch::Coord3dData::generate(const PYXRhombus & rhombus)
{
	PYXCursor v_c(rhombus.getIndex(0),rhombus.getDirection(0));

	v_c.zoomIn();
	v_c.zoomIn();
	v_c.zoomIn();
	v_c.zoomIn();

	v_c.left();

	for(int v=0;v<10;v++)
	{
		PYXCursor u_c(v_c);
		u_c.right();

		for(int u=0;u<10;u++)
		{
			PYXIcosIndex index = u_c.getIndex();
			CoordLatLon ll;
			SnyderProjection::getInstance()->pyxisToNative(index, &ll);
			data[u][v] = SphereMath::llxyz(ll);	

			u_c.forward();
		}

		v_c.forward();
	}	
}


/////////////////////////////////////////////////////////////////////////////////////////
// ElevationData
/////////////////////////////////////////////////////////////////////////////////////////

PYXPointer<Surface::Patch::ElevationData> Surface::Patch::ElevationData::zoomIn(int patchIndex)
{
	PYXPointer<ElevationData> result = ElevationData::create();

	int u_offset = patchIndex % 3;
	int v_offset = patchIndex / 3;

	for(int u=0;u<10;u++)
	{
		for(int v=0;v<10;v++)
		{
			int u_base = u/3+u_offset*3;
			int v_base = v/3+v_offset*3;

			int u_baseUp = (u_base + 1)%10;
			int v_baseUp = (v_base + 1)%10;

			double uoff = (u%3)/3.0;
			double voff = (v%3)/3.0;

			result->data[u][v] = 
				  (data[u_base][v_base  ]*(1-uoff) + data[u_baseUp][v_base  ]*uoff)*(1-voff) 
				+ (data[u_base][v_baseUp]*(1-uoff) + data[u_baseUp][v_baseUp]*uoff)*(voff);
		}
	}

	return result;
}


/////////////////////////////////////////////////////////////////////////////////////////
// VertexBuffer
/////////////////////////////////////////////////////////////////////////////////////////


void Surface::Patch::VertexBuffer::generate(PYXPointer<Surface::Patch::Coord3dData> spCoords)
{
	Surface::Patch::Coord3dData & coords = *spCoords;	

	for(int u=0;u<10;u++)
	{
		for(int v=0;v<10;v++)
		{
			vertices_doubles[u][v] = CmlConvertor::toVec3(coords.data[u][v]);
			CmlConvertor::fromVec3WithOrigin(vertices_doubles[u][v],zero,vertices_floats[u*10+v]);
		}
	}
}

void Surface::Patch::VertexBuffer::generate(PYXPointer<Surface::Patch::Coord3dData> spCoords, PYXPointer<Surface::Patch::ElevationData> spElevation)
{
	Surface::Patch::Coord3dData & coords = *spCoords;
	Surface::Patch::ElevationData & elevation = *spElevation;

	for(int u=0;u<10;u++)
	{
		for(int v=0;v<10;v++)
		{
			vertices_doubles[u][v] = CmlConvertor::toVec3(coords.data[u][v]);
			vertices_doubles[u][v] *= elevation.data[u][v];
			CmlConvertor::fromVec3WithOrigin(vertices_doubles[u][v],zero,vertices_floats[u*10+v]);
		}
	}
}

void Surface::Patch::VertexBuffer::generate(PYXPointer<Surface::Patch::Coord3dData>  spCoords, PYXPointer<Surface::Patch::ElevationData> spElevation,float uOffset,float uScale,float vOffset,float vScale)
{ 
	const double K = 20.0; //not used right now...

	Surface::Patch::Coord3dData & coords = *spCoords;
	Surface::Patch::ElevationData & elevation = *spElevation;

	for(int u=0;u<10;u++)
	{
		for(int v=0;v<10;v++)
		{
			double scaled_u = u*uScale+uOffset;
			double u_offset = scaled_u-floor(scaled_u);
			double scaled_v = v*vScale+vOffset;
			double v_offset = scaled_v-floor(scaled_v);

			double s = cml::bilerp(elevation.data[(int)floor(scaled_u)][(int)floor(scaled_v)],elevation.data[std::min((int)ceil(scaled_u),9)][(int)floor(scaled_v)],
				elevation.data[(int)floor(scaled_u)][std::min((int)ceil(scaled_v),9)],elevation.data[std::min((int)ceil(scaled_u),9)][std::min((int)ceil(scaled_v),9)],
								   u_offset,v_offset);
			
			vertices_doubles[u][v] = CmlConvertor::toVec3(coords.data[u][v]);
			vertices_doubles[u][v] *= s;
			CmlConvertor::fromVec3WithOrigin(vertices_doubles[u][v],zero,vertices_floats[u*10+v]);
		}
	}
}

PYXPointer<Surface::Patch::VertexBuffer> Surface::Patch::VertexBuffer::createInterpolation(const PYXRhombus & rhombus, int patchIndex,const PYXPointer<VertexBuffer> & lowerResolutionVertices)
{
	int u_offset = patchIndex % 3;
	int v_offset = patchIndex / 3;

	PYXPointer<Surface::Patch::Coord3dData> coords = Surface::Patch::Coord3dData::create();
	coords->generate(rhombus);

	PYXPointer<VertexBuffer> result = VertexBuffer::create();
	result->zero = lowerResolutionVertices->zero;

	for(int u=0;u<10;u++)
	{
		for(int v=0;v<10;v++)
		{
			result->vertices_doubles[u][v] = CmlConvertor::toVec3(coords->data[u][v]);
			CmlConvertor::fromVec3WithOrigin(result->vertices_doubles[u][v],result->zero,result->vertices_floats[u*10+v]);
		}
	}

	result->updateBBox();

	return result;
}

void Surface::Patch::VertexBuffer::borrowLowerResolutionVertices(int patchIndex,const PYXPointer<Surface::Patch::VertexBuffer> & lowerResolutionVertices)
{
	int u_offset = patchIndex % 3;
	int v_offset = patchIndex / 3;

	for(int u=0;u<4;u++)
	{
		for(int v=0;v<4;v++)
		{
			CmlConvertor::fromVec3WithOrigin(lowerResolutionVertices->vertices_doubles[u+u_offset*3][v+v_offset*3],zero,vertices_floats[10*10+u*4+v]);
		}
	}
}

void Surface::Patch::VertexBuffer::setVertex(int u,int v,const vec3 & coord)
{
	vertices_doubles[u][v] = coord;	
	CmlConvertor::fromVec3WithOrigin(vertices_doubles[u][v],zero,vertices_floats[u*10+v]);
}

double Surface::Patch::VertexBuffer::getElevation(const vec2 & uv) const
{
	assert(uv[0]>=0 && uv[0]<=1 && "u is out of range");
	assert(uv[1]>=0 && uv[1]<=1 && "v is out of range");

	int u_min = static_cast<int>(floor(uv[0]*9));
	int v_min = static_cast<int>(floor(uv[1]*9));
	int u_max = static_cast<int>(ceil(uv[0]*9));
	int v_max = static_cast<int>(ceil(uv[1]*9));
	double u_offset = uv[0]*9-u_min;
	double v_offset = uv[1]*9-v_min;

	assert(u_min>=0 && u_min <=9 && "U-Min is out range");
	assert(v_min>=0 && v_min <=9 && "V-Min is out range");
	assert(u_max>=0 && u_max <=9 && "U-Max is out range");
	assert(v_max>=0 && v_max <=9 && "V-Max is out range");

	assert(u_offset>=0 && u_offset<=1 && "U-offset is out of range");
	assert(v_offset>=0 && v_offset<=1 && "V-offset is out of range");

	if (u_offset+v_offset<1)
	{
		/*			  u_offset -->
		(u_min,v_min)	   (u_max,v_min)
		             +----+
		v_offset     |   /
			|		 |  /
			|		 | /
 			V		 |/  
					 +
					 (u_min,v_max)
		*/
		return vertices_doubles[u_min][v_min].length()*(1-u_offset-v_offset)+
			   vertices_doubles[u_max][v_min].length()*u_offset+
			   vertices_doubles[u_min][v_max].length()*v_offset;
	}
	else
	{
		/*			  u_offset -->
						   (u_max,v_min)
		                  +
		v_offset         /|
			|		    / |
			|		   /  |
 			V		  /   |
					 +----+
		(u_min,v_max)     (u_max,v_max)
		*/
		u_offset = 1-u_offset;
		v_offset = 1-v_offset;
		return vertices_doubles[u_max][v_max].length()*(1-u_offset-v_offset)+
		   vertices_doubles[u_max][v_min].length()*u_offset+
		   vertices_doubles[u_min][v_max].length()*v_offset;
	}
}

bool Surface::Patch::VertexBuffer::intersects(const Ray & ray,double & time) const
{
	for(int u=0;u<9;u++)
	{
		for(int v=0;v<9;v++)
		{
			
			const vec3 & v0 = vertices_doubles[u][v];
			const vec3 & v1 = vertices_doubles[u+1][v];
			const vec3 & v2 = vertices_doubles[u][v+1];
			const vec3 & v3 = vertices_doubles[u+1][v+1];

			if (ray.intersectsWithTriangle(v0,v1,v2,time)) 
			{
				if (time>0)
				{
					return true;
				}
			}

			if (ray.intersectsWithTriangle(v1,v2,v3,time)) 
			{
				if (time>0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void Surface::Patch::VertexBuffer::updateBBox()
{
	vec3 center;
	center.zero();
	int count = 0;

	for(int u=4;u<6;u++)
	{
		for(int v=4;v<6;v++)
		{
			center += vertices_doubles[u][v];
			count++;
		}
	}
	center /= count;

	double radius_squared=0;

	for(int u=0;u<10;u+=2)
	{
		for(int v=0;v<10;v+=2)
		{
			double v_radius_squared = (vertices_doubles[u][v] - center).length_squared();

			if (v_radius_squared > radius_squared)
			{
				radius_squared=v_radius_squared;
			}
		}
	}

	bbox.setCenter(center);
	bbox.setRadius(sqrt(radius_squared));
}

/////////////////////////////////////////////////////////////////////////////////////////
// UVOffset
/////////////////////////////////////////////////////////////////////////////////////////

Surface::UVOffset::UVOffset(const Surface::Patch::Key & source,const Surface::Patch::Key & dest)
{
	u_offset = 0.0;
	v_offset = 0.0;
	u_scale  = 1.0;
	v_scale  = 1.0;

	int minRes = std::min(dest.getResolution(),source.getResolution());

	assert(dest.toString().substr(0,minRes)==source.toString().substr(0,minRes)
		 && "indecies are not parent and son, this not work well outside the family");

	if (source.getResolution() > dest.getResolution())
	{
		for(int i=dest.getResolution()+1;i<source.getResolution()+1;i++)
		{
			addIndex(source.getPatchIndex(i));
		}
	}

	if (dest.getResolution() > source.getResolution())
	{
		for(int i=dest.getResolution();i>source.getResolution()+1;i--)
		{
			removeIndex(dest.getPatchIndex(i));
		}
	}
}

// source.resolution > dest.resolution
void Surface::UVOffset::addIndex(int index)
{
	u_scale /= 3.0;
	v_scale /= 3.0;
	u_offset += (index % 3)*u_scale;
	v_offset += (index / 3)*v_scale;
}

// dest.resolution > source.resolution
void Surface::UVOffset::removeIndex(int index)
{
	u_scale *= 3.0;
	v_scale *= 3.0;
	u_offset -= (index % 3)*u_scale;
	v_offset -= (index / 3)*v_scale;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Surface::Vertex
/////////////////////////////////////////////////////////////////////////////////////////


Surface::Vertex::Vertex(const PYXIcosIndex & index) 
	:	m_index(index),
		m_visibleCount(0),
		m_notHiddenCount(0),
		m_dividedCount(0),
		m_hasDividedSubPatchesCount(0),
		m_maxPriority(0)
{
	if (index.isHexagon())
	{
		m_fullCount = 4;
	}
	else if (index.isPolar())
	{
		m_fullCount = 5;
	}
	else
	{
		m_fullCount = 3;
	}
}

void Surface::Vertex::addPatch(Surface::Patch * patch)
{
	m_patchSet.insert(patch);
}

void Surface::Vertex::removePatch(Surface::Patch * patch)
{
	m_patchSet.erase(patch);
}

void Surface::Vertex::refreshCount()
{
	m_visibleCount = 0;
	m_notHiddenCount = 0;
	m_dividedCount = 0;
	m_hasDividedSubPatchesCount = 0;
	m_maxPriority = 0;

	for(auto & patch : m_patchSet)
	{
		if (patch->isVisible())
		{
			m_visibleCount++;
		}

		if (patch->isNotHidden())
		{
			m_notHiddenCount++;
		}

		if (patch->isDivided())
		{
			m_dividedCount++;
		}

		if (patch->hasDividedSubPatch())
		{
			m_hasDividedSubPatchesCount++;
		}

		if (patch->getPriority() > m_maxPriority)
		{
			m_maxPriority = patch->getPriority();
		}
	}
}

int Surface::Vertex::getPatchCount() const
{
	return m_patchSet.size();
}

bool Surface::Vertex::hasAllPatches() const
{
	return getPatchCount() == m_fullCount;
}

bool Surface::Vertex::doesAllPatchesNotHidden() const
{
	return m_notHiddenCount == m_fullCount;
}

bool Surface::Vertex::doesAllPatchesDivded() const
{
	return m_dividedCount == m_fullCount;
}

double Surface::Vertex::getMaxPriority() const
{
	return m_maxPriority;
}

void Surface::Vertex::setMaxPriority(double newMaxPriority)
{
	m_maxPriority = newMaxPriority;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Surface::VertexSet
/////////////////////////////////////////////////////////////////////////////////////////


std::vector<PYXPointer<Surface::Vertex>> Surface::VertexSet::addPatch(Surface::Patch & patch)
{
	std::vector<PYXPointer<Surface::Vertex>> vertices(0);
	for (int i=0;i<4;i++)
	{
		vertices.push_back(getVertex(patch.getRhombus().getIndex(i))->second);
		vertices[i]->addPatch(&patch);
	}
	return vertices;
};

void Surface::VertexSet::removePatch(Surface::Patch & patch)
{
	for (int i=0;i<4;i++)
	{
		VertexMap::iterator it = getVertex(patch.getRhombus().getIndex(i));
		it->second->removePatch(&patch);
	
		if (it->second->getPatchCount()==0)
		{
			m_vertexMap.erase(it);
		}
	}
}

Surface::VertexSet::VertexMap::iterator Surface::VertexSet::getVertex(const PYXIcosIndex & index)
{
	VertexMap::iterator it = m_vertexMap.find(index);
	if (it == m_vertexMap.end())
	{
		it = m_vertexMap.insert(m_vertexMap.end(),std::make_pair(index,Vertex::create(index)));
	}
	return it;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Surface::Edge
/////////////////////////////////////////////////////////////////////////////////////////


Surface::Edge::Edge(const PYXIcosIndex & from,const PYXIcosIndex & to) 
	:	m_from(from),
		m_to(to),
		m_left(nullptr),
		m_right(nullptr)
{
}

void Surface::Edge::addPatch(Surface::Patch * patch,int edgeIndex)
{
	if (m_from == patch->getRhombus().getIndex(edgeIndex))
	{
		m_left = patch;

		switch(edgeIndex)
		{
		case 0:
			m_leftUVBase = PYXCoord2DInt(0,0);
			m_leftUVDirection = PYXCoord2DInt(1,0);
			break;
		case 1:
			m_leftUVBase = PYXCoord2DInt(9,0);
			m_leftUVDirection = PYXCoord2DInt(0,1);
			break;
		case 2:
			m_leftUVBase = PYXCoord2DInt(9,9);
			m_leftUVDirection = PYXCoord2DInt(-1,0);
			break;
		case 3:
			m_leftUVBase = PYXCoord2DInt(0,9);
			m_leftUVDirection = PYXCoord2DInt(0,-1);
			break;
		}
	}
	else 
	{
		m_right = patch;
		switch(edgeIndex)
		{
		case 0:
			m_rightUVBase = PYXCoord2DInt(9,0);
			m_rightUVDirection = PYXCoord2DInt(-1,0);
			break;
		case 1:
			m_rightUVBase = PYXCoord2DInt(9,9);
			m_rightUVDirection = PYXCoord2DInt(0,-1);
			break;
		case 2:
			m_rightUVBase = PYXCoord2DInt(0,9);
			m_rightUVDirection = PYXCoord2DInt(1,0);
			break;
		case 3:
			m_rightUVBase = PYXCoord2DInt(0,0);
			m_rightUVDirection = PYXCoord2DInt(0,1);
			break;
		}
	}
}

void Surface::Edge::removePatch(Surface::Patch * patch,int edgeIndex)
{
	if (m_from == patch->getRhombus().getIndex(edgeIndex))
	{
		m_left = nullptr;
	}
	else 
	{
		m_right = nullptr;
	}
}

Surface::Patch * Surface::Edge::getOtherSide(Surface::Patch * side)
{
	if (side == m_left) 
	{
		return m_right;
	}
	if (side == m_right)
	{
		return m_left;
	}
	return nullptr;
}

const Surface::Patch * Surface::Edge::getOtherSide(const Surface::Patch * side) const
{
	if (side == m_left) 
	{
		return m_right;
	}
	if (side == m_right)
	{
		return m_left;
	}
	return nullptr;
}

int Surface::Edge::getPatchCount()
{
	return (m_left != nullptr?1:0) + (m_right != nullptr?1:0);
}

bool Surface::Edge::hasElevation()
{
	if (m_left != nullptr && m_left->elevations) return true;
	if (m_right != nullptr && m_right->elevations) return true;
	return false;
}

void Surface::Edge::copyEdgeElevation(Surface::Patch * source,Patch::ElevationData & sourceData,Surface::Patch * destination,Patch::ElevationData & destData)
{
	PYXCoord2DInt uvSource;
	PYXCoord2DInt uvDest;

	PYXCoord2DInt uvSourceDirection;
	PYXCoord2DInt uvDestDirection;

	if (m_left == destination)
	{
		uvSource = m_rightUVBase;
		uvDest = m_leftUVBase;
		uvSourceDirection = m_rightUVDirection;
		uvDestDirection = m_leftUVDirection;
	}
	if (m_right == destination)
	{
		uvSource = m_leftUVBase;
		uvDest = m_rightUVBase;
		uvSourceDirection = m_leftUVDirection;
		uvDestDirection = m_rightUVDirection;
	}

	for(int i=0;i<10;i++)
	{
		destData.data[uvDest[0]+uvDestDirection[0]*i][uvDest[1]+uvDestDirection[1]*i] =
			sourceData.data[uvSource[0]+uvSourceDirection[0]*i][uvSource[1]+uvSourceDirection[1]*i];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Surface::EdgeSet
/////////////////////////////////////////////////////////////////////////////////////////


std::vector<PYXPointer<Surface::Edge>> Surface::EdgeSet::addPatch(Surface::Patch & patch)
{
	std::vector<PYXPointer<Surface::Edge>> edges(0);
	for (int i=0;i<4;i++)
	{
		edges.push_back(getEdge(patch.getRhombus().getIndex(i),patch.getRhombus().getIndex((i+1)%4))->second);
		edges[i]->addPatch(&patch,i);
	}
	return edges;
};

void Surface::EdgeSet::removePatch(Surface::Patch & patch)
{
	for (int i=0;i<4;i++)
	{
		EdgeMap::iterator it = getEdge(patch.getRhombus().getIndex(i),patch.getRhombus().getIndex((i+1)%4));
		it->second->removePatch(&patch,i);
	
		if (it->second->getPatchCount()==0)
		{
			m_edgeMap.erase(it);
		}
	}
}

Surface::EdgeSet::EdgeMap::iterator Surface::EdgeSet::getEdge(const PYXIcosIndex & from,const PYXIcosIndex & to)
{
	
	std::pair<PYXIcosIndex,PYXIcosIndex> key = from<to?std::make_pair(from,to):std::make_pair(to,from);

	EdgeMap::iterator it = m_edgeMap.find(key);
	
	if (it == m_edgeMap.end())
	{
		it = m_edgeMap.insert(m_edgeMap.end(),std::make_pair(key,Surface::Edge::create(key.first,key.second)));
	}
	return it;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Surface
/////////////////////////////////////////////////////////////////////////////////////////

Surface::Surface()
{
	int index=0;

	PYXIcosIndex northPool = PYXIcosIndex("1-0");
	PYXIcosIndex southPool = PYXIcosIndex("12-0");

	popolateSurfacePatchForPool(northPool);
	popolateSurfacePatchForPool(southPool);
	
}

void Surface::popolateSurfacePatchForPool(const PYXIcosIndex & pool)
{
	for (int dir=1;dir<=6;dir++)
	{
		if (PYXIcosMath::isValidDirection(pool.getPrimaryResolution(), (PYXMath::eHexDirection)dir))
		{
			PYXCursor c(pool,(PYXMath::eHexDirection)dir);
			c.left();
			for(int v=0;v<3;v++)
			{
				PYXCursor c2(c);
				c2.right();
				for(int u=0;u<3;u++)
				{
					PYXPointer<Surface::Patch> newPatch = PYXNEW(Surface::Patch,*this,PYXRhombus(c2),m_patches.size());

					m_patches.push_back(newPatch);					
					c2.forward();
				}
				c.forward();
			}
			
		}
	}
}

Surface::~Surface()
{
}

void Surface::findVisiblePatchs(const Camera & camera,const int & viewPortHeight)
{
	struct PatchComp
	{
		bool operator()(const PYXPointer<Surface::Patch> & a,const PYXPointer<Surface::Patch> & b)
		{
			return a->getPriority() > b->getPriority();
		}
	};

	m_visiblePatches.clear();
	m_patchesNeedDividing.clear();
	m_patchesNeedUnifing.clear();

	CameraFrustum frustum(camera);
	
	findVisiblePatchsInVector(frustum,camera.getEye(),camera.getLook(),m_patches,CameraFrustum::knClassBoundary,viewPortHeight,false);	

	std::sort(m_visiblePatches.begin(),m_visiblePatches.end(),PatchComp());
	std::sort(m_patchesNeedDividing.begin(),m_patchesNeedDividing.end(),PatchComp());
	std::sort(m_patchesNeedUnifing.begin(),m_patchesNeedUnifing.end(),PatchComp()); //TODO: we might want to revese this vector...
}

double Surface::getPatchPriority(const vec3 & eye,const vec3 & look,Surface::Patch & patch)
{
	vec3 direction = (patch.m_bbox.getCenter()-eye);
	direction.normalize();
	double cosAngle = cml::dot(direction,look);
	
	double sizeFactor = patch.getSizeOnScreen();

	int renderWeight = 0;

	if (sizeFactor > 300) 
	{
		//sizes bigger then 200 will reduce thier size
		//100 -> 100
		//200 -> 200
		//300 -> 133
		//400 -> 100
		//500 ->  80
		//1000 -> 40
		sizeFactor = 300*300/sizeFactor;
	}

	return sizeFactor * (cml::clamp(cosAngle,0.6,1.0)*2-1) + renderWeight;
}

void Surface::findVisiblePatchsInVector(CameraFrustum & frustum,const vec3 & eye,const vec3 & look,Surface::PatchVector & vector,CameraFrustum::CameraFrustumBoundary boundaryHint,int viewPortHeight,bool isHidden)
{
	for(auto & it : vector)
	{
		Surface::Patch & patch = (*it);

		CameraFrustum::CameraFrustumBoundary boundary = boundaryHint;
		
		if (boundary == CameraFrustum::knClassBoundary)
		{
			boundary = frustum.classifyPoint(patch.m_bbox.getCenter(),patch.m_bbox.getRadius());
		}

		Surface::Patch::State newState(patch.m_state);

		switch(boundary)
		{
		case CameraFrustum::knClassInside:
		case CameraFrustum::knClassBoundary:

			//we are using the original radius - because elevation can make the radius realy realy big - which resuling of keep deviding patches...
			newState.m_sizeOnScreen = frustum.getSizeOnScreen(viewPortHeight,patch.m_bbox.getCenter(),patch.m_sizeOnScreenRdius); // patch.m_bbox.getRadius()) 
			newState.m_toBigOnScreen = (newState.m_sizeOnScreen  > 100);
			newState.m_visible = true;
			newState.m_hidden = isHidden;
			//Note: we set the priority after...

			if (patch.isDivided())
			{
				if (newState.m_toBigOnScreen && ! patch.hasVisiblityBlock())
				{
					findVisiblePatchsInVector(frustum,eye,look,patch.m_subPatches,boundary,viewPortHeight,isHidden);

					//propogate priority up the chain to speed up dividing neigboors patchs.
					newState.m_priority = patch.borrowPriorityFromSubPatchs();
				}
				else
				{
					if (!isHidden)
					{
						m_visiblePatches.push_back(&patch);
					}

					//mark all children as hidden...
					findVisiblePatchsInVector(frustum,eye,look,patch.m_subPatches,boundary,viewPortHeight,true);
				}
			}
			else
			{
				if (!isHidden)
				{
					m_visiblePatches.push_back(&patch);
				}
			}
			break;

		case CameraFrustum::knClassOutside:
			{
				//zero priority
				newState.m_priority = 0;
				newState.m_sizeOnScreen = 0;
				newState.m_toBigOnScreen = false; 
			}
			newState.m_visible = false;
			newState.m_hidden = isHidden;

			if (patch.isDivided())
			{
				//mark all children as hidden...
				findVisiblePatchsInVector(frustum,eye,look,patch.m_subPatches,CameraFrustum::knClassOutside,viewPortHeight,patch.hasVisiblityBlock());
			}
		}

		bool visibleChange = (newState.m_visible && !newState.m_hidden) != (patch.m_state.m_visible && !patch.m_state.m_hidden);

		patch.setState(newState);

		if (patch.isVisible())
		{
			patch.m_state.m_priority = getPatchPriority(eye,look,patch);
		}

		if (visibleChange)
		{
			if (patch.isVisible() && ! patch.isHidden())
			{
				m_patchBecomeVisible.notify(Surface::Event::create(it));
			}
			else
			{
				m_patchBecomeNotVisible.notify(Surface::Event::create(it));
			}
		}

		if (patch.isDivided())
		{
			if (patch.needsToBeUnified() && patch.canBeUnified())
			{
				m_patchesNeedUnifing.push_back(&patch);
			}
			if (patch.hasVisiblityBlock()) //AKA - it still been divided...
			{
				if ((patch.needsToBeDivided() || patch.hasNeighborPatch(&Surface::Patch::needsToBeDivided)) && patch.canBeDivided())
				{
					m_patchesNeedDividing.push_back(&patch);
				}
			}
		}
		else
		{
			if ((patch.needsToBeDivided() || patch.hasNeighborPatch(&Surface::Patch::needsToBeDivided)) && patch.canBeDivided())
			{
				m_patchesNeedDividing.push_back(&patch);
			}
		}
	}
}

PYXPointer<Surface::Patch> Surface::getPatch(const Surface::Patch::Key & key)
{
	PYXPointer<Surface::Patch> patch;
	if (key.getResolution() < 0)
	{
		return patch;
	}
	patch = m_patches[key.getPatchIndex(0)];
	int res = 1;

	while(patch && res <= key.getResolution())
	{
		if (patch->isDivided())
		{
			patch = patch->getSubPatch(key.getPatchIndex(res));
		}
		else
		{
			patch.reset();
		}
		res++;
	}

	return patch;
}

PYXPointer<const Surface::Patch> Surface::getPatch(const Surface::Patch::Key & key) const
{
	PYXPointer<const Surface::Patch> patch;
	if (key.getResolution() < 0)
	{
		return patch;
	}
	patch = m_patches[key.getPatchIndex(0)];
	int res = 1;

	while(patch && res <= key.getResolution())
	{
		if (patch->isDivided())
		{
			patch = patch->getSubPatch(key.getPatchIndex(res));
		}
		else
		{
			patch.reset();
		}
		res++;
	}

	return patch;
}