#pragma once
#ifndef VIEW_MODEL__SURFACE_H
#define VIEW_MODEL__SRUFACE_H
/******************************************************************************
surface.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "camera.h"
#include "rhombus.h"
#include "ray.h"
#include "cml_utils.h"

#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/notifier.h"

#include <map>
#include <list>
#include <vector>

// boost includes
#include <boost/function.hpp>

//forward definition
class Ray;

/*!
SphereBBox - helper class used to create a spherical BBox around Pyxis cells and Rhombi
*/
class SphereBBox
{
protected:
	vec3	m_center;
	double	m_radius;

public:
	SphereBBox();
	SphereBBox(const PYXCoord3DDouble & center,const double & m_radius);
	SphereBBox(const SphereBBox & other);
	SphereBBox(const PYXIcosIndex & cell);
	SphereBBox(const PYXIcosIndex & cellA,const PYXIcosIndex & cellB,bool includeCellRadius = false);

	const vec3   & getCenter() { return m_center;}
	const double & getRadius() { return m_radius; }

	void setCenter(const vec3 & newCenter) { m_center = newCenter; }
	void setRadius(const double & newRadius) { m_radius = newRadius; }	
};

/*!
Surface - class represent a surface on earth with LOD PYXRhombus.

A Surface is a tree of Rhombus (called Patch) that cover the entire earth.
The prim resolution of Rhombus that cover the earth are PYXIS resolution 2 (starting from "1-0" and "12-0" - north and south pools) - which cover the earth by using 90 rhombi.
Then, each Patch can have 9 sub Patches - as PYXRhombus behave - and then we have a nice tree of Patches.

A Surface API allow you to give it a camera and the viewPortHeight to find all visible patches on the surface.
Moreover, the Surface class check the size of each Patch on the screen, and if it too big (~100 pixels) it will try use the sub Patches instead.

Note, the Surface is not render anything, or store information like elevation and texture.
The Surface is responsible for finding which Patches needed to be rendered so the screen should be made of Patches that are around 100 pixels on the screen.

Once the Surface find visible patches it also create a list of Patches that need to be divided.
The ViewOpenGLThread / RhombusRenderer can decided weather they want to divided those Patches to show more details.

A Surface work with SurfaceMemento<T> to store information about the Patches. the SurfaceMemento<T> class works with Surface class by registering to Surface Events:
1. beforePatchDivided - called before a patch is about to be divided.
2. afterPatchDivided - called after a patch was divided
3. beforePatchUnified - called before a patch is about to to be unified (remove it's sub patches from the tree)
4. afterPatchUnified - called after a patch was unified (remove it's sub patches from the tree)
5. afterPatchVisibliyBlockRemoved - called after the visibility block was removed from a patch

Note about VisiblityBlock - the surface allow a control on the level of details allow for each patch. 
A patch with Visibility block can be visible! However, it's sub Patches can't be visible. 
Therefore, even if the patch is to big on the screen, The surface would not mark it sub patches (even if they exists) as visible.

The ViewOpenGLThread / RhombusRenderer uses the visibility block to allow them to load data to the sub patches before rendering them to screen.

You can fetch a specific patch from the Surface tree by using Surface::Patch::Key - see Surface::Patch for more information
*/
class Surface : public PYXObject
{
public:
	class Patch;
	class UVOffset;
	typedef std::vector<PYXPointer<Patch>> PatchVector;	
	class Vertex;
	class VertexSet;

	class Edge;
	class EdgeSet;

	/*!
	Patch - a class that represent a visual PYXRhombus

	A Patch is the basic node inside the SurfaceTree. As a node inside a surface tree, a Patch contain the following data:
	1. isVisible - true when the Patch is visible on the screen. Note, if a patch is visible - then it's parent is also visible.
	2. isDivided - true if the Patch has children.
	3. hasVisiblityBlock - true when the patch children can't be visualized.
	4. getSizeOnScreen - the amount of pixels on the screen (~).
	5. isTooBig - return true if the surface decided the patch is to big.
	6. getPriority - return the loading priority of the patch. higher value mean load it first.

	as a node in the tree, you can ask a patch for:
	1. getSubPatch(index) - the index is as PYXRhombus index it's sub rhombi
	2. getParent() - return the parent patch. or null if has not parent (Prim Patch)

	Moreover, a patch as a Key to represent it's location inside the Surface Tree, see Surface::Patch::Key for more information

	The SurfaceMemento<T> class uses Surface::Patch as a key to store and fetch information
	*/
	class Patch : public PYXObject, ObjectMemoryUsageCounter<Patch>
	{
		friend class Surface;

	public:
		/*!
		Surface::Patch::Key - represent an ID and a location of a Patch inside a Surface Tree.

		A Key is a string of digits, build as follows:
		1. first two digits - the index of the prim patch. [00..89]
		2. each digit that follows is the index of the patch related to it's father.

		Examples:
		1. the first PrimPatch key is "00".
		2. the second PrimPatch key is "01".
		3. the last PrimPatch key is "89".
		4. the PrimPatch["13"] first child key is "130"
		5. the PrimPatch["13"] second child key is "131"
		6. the PrimPatch["13"] last child key is "138"
		5. the the third child of PrimPatch["13"] second child key is "1312"

		The Surface::Patch::Key allow fast traversal on the SurfaceTree:
		1. the first two digits specify the index of the Prim Patch.
		2. then go inside the tree using each digit as the child patch index.

		the Key class API is:
		1. getResolution() - Prim Patch is res 0. their children is res 1, etc...
		2. getPatchIndex(int resolution) - return the index for that resolution
		3. getLastIndex() - return the index of the last resolution inside the key
		4. isPrimResolution() - return true if this is a prim patch (has not parent)
		
		A Key can be created from a Patch or from a string.
		*/
		class Key
		{
		protected:
			std::string m_key;
			bool Surface::Patch::Key::validatePatchsIndices() const;

		public:
			explicit Key(const std::string & key);
			explicit Key(int firstIndex);
			explicit Key(const Key & key,int index);
			Key(const Patch & patch);
			Key();

			Key(const Key & Key);
			const Key & operator =(const Key & Key);


			int getPatchIndex(int resolution) const;
			const std::string & toString() const { return m_key; }

			//first level 90 Patches are res '0';
			//their children is res '1';
			//their children children is res '2';
			int getResolution() const { return (int)(m_key.size())-2; }

			bool isPrimResolution() const { return m_key.size()==2; }

			int getLastIndex() const { return getPatchIndex(getResolution()); }

			bool operator==(const Key & other) const
			{
				return m_key == other.m_key;
			}

			bool operator!=(const Key & other) const
			{
				return m_key != other.m_key;
			}

			bool operator<(const Key & other) const
			{
				return m_key < other.m_key;
			}

			bool operator>(const Key & other) const
			{
				return m_key > other.m_key;
			}
		};

		/*!
		Surface::Patch::State - keep all information about the visibily paramters of the patch
		*/
		//! Surface::Patch::State - keep all information about the visibily paramters of the patch
		class State
		{
			friend class Surface;
			friend class Surface::Patch;

		protected:
			double		   m_priority;
			bool		   m_visiblityBlock;

			bool		   m_divided;
			int			   m_subPatchDividedCount;

			bool		   m_visible;
			bool           m_hidden;

			bool		   m_toBigOnScreen;
			double	       m_sizeOnScreen;

		public:
			State() :
				m_priority(0),
				m_visiblityBlock(true),
				m_divided(false),
				m_subPatchDividedCount(0),
				m_visible(false),
				m_hidden(false),
				m_toBigOnScreen(false),
				m_sizeOnScreen(0)
			{
			}

			bool hasDividedSubPatch() const
			{
				return m_subPatchDividedCount > 0;
			}

			bool neetToRefereshSurfaceVertices(const State & other) const
			{
				return	m_divided != other.m_divided || 
						m_visible != other.m_visible || 
						m_visiblityBlock != other.m_visiblityBlock ||
						m_toBigOnScreen != other.m_toBigOnScreen ||
						m_hidden != other.m_hidden || 
						hasDividedSubPatch() != other.hasDividedSubPatch();
			}

			bool hasVisibilityChange(const State & other) const
			{
				return	m_visible != other.m_visible || 
						m_hidden != other.m_hidden ||
						m_toBigOnScreen != other.m_toBigOnScreen;
			}

			bool hasPriorityChange(const State & other) const
			{
				return m_priority != other.m_priority;
			}
		};

		class ElevationData : public PYXObject, ObjectMemoryUsageCounter<ElevationData>
		{
		public:
			static PYXPointer<ElevationData> create() { return PYXNEW(ElevationData); }
			ElevationData() {}

			PYXPointer<ElevationData> zoomIn(int index);

		public:
			double data[10][10];
		};

		class Coord3dData : public PYXObject, ObjectMemoryUsageCounter<Coord3dData>
		{
		public:
			static PYXPointer<Coord3dData> create() { return PYXNEW(Coord3dData); }

			Coord3dData() {};
			virtual ~Coord3dData() {};
		public:
			PYXCoord3DDouble data[10][10];

			void generate(const PYXRhombus & rhombus);
		};

		class VertexBuffer : public PYXObject, ObjectMemoryUsageCounter<VertexBuffer>
		{
		public:
			static PYXPointer<VertexBuffer> create() { return PYXNEW(VertexBuffer); }
			static PYXPointer<VertexBuffer> createInterpolation(const PYXRhombus & rhombus,int patchIndex,const PYXPointer<VertexBuffer> & lowerResolutionVertices);

			VertexBuffer()  { zero.zero(); };
			virtual ~VertexBuffer() {};
		public:
			float vertices_floats[10*10+4*4][3];
			vec3  vertices_doubles[10][10];
			vec3  zero;
			SphereBBox bbox;

			void generate(PYXPointer<Surface::Patch::Coord3dData> coords);
			void generate(PYXPointer<Surface::Patch::Coord3dData> coords, PYXPointer<Surface::Patch::ElevationData> elevation);
			void generate(PYXPointer<Surface::Patch::Coord3dData> coords, PYXPointer<Surface::Patch::ElevationData> elevation,float uOffset,float uScale,float vOffset,float vScale);

			void borrowLowerResolutionVertices(int patchIndex,const PYXPointer<VertexBuffer> & lowerResolutionVertices);

			void setVertex(int u,int v,const vec3 & coord);

			bool intersects(const Ray & ray,double & time) const;

			double getElevation(const vec2 & uv) const;

			void updateBBox();
		};

		PYXPointer<Coord3dData> coords;
		PYXPointer<ElevationData> elevations;
		PYXPointer<ElevationData> borrowedElevations;
		int borrowedElevationsDepth;

		PYXPointer<VertexBuffer> vertices;


		void updateElevation(const PYXPointer<ElevationData> newElevation);
		void matchElevationFromNeighbors();

		void borrowElevation();
		void updateVertices();

		const PYXPointer<VertexBuffer> & getVertices();

	protected:
		PYXRhombus     m_rhombus;
		int			   m_index;
		Key			   m_key;
		Patch		 * m_parent;

		State		   m_state;

		PatchVector	   m_subPatches;

		SphereBBox	   m_bbox;
		double		   m_sizeOnScreenRdius;
		Surface		 & m_surface;

		std::vector<PYXPointer<Vertex>> m_surfaceVertices;
		std::vector<PYXPointer<Edge>> m_surfaceEdges;

	protected:
		//used to create top level patches
		Patch(Surface & surface, const PYXRhombus & rhombus, const int & index);
		//used to create child patches
		Patch(Patch & parent, const int & index);

		void setState(const State & newState);

		void refreshVerticesCount();

	public:
		virtual ~Patch();

		//! return a PYXRhombus for this Patch
		const PYXRhombus & getRhombus() const;

		//! return the parent Patch
		PYXPointer<Patch> getParent();

		//! return the parent Patch
		PYXPointer<const Patch> getParent() const;

		//! return the Surface containing this patch
		Surface & getSurface();

		//! return the Surface containing this patch
		const Surface & getSurface() const;

		//! return the Key for this patch
		const Key & getKey() const;

		//! return the index of this patch == getKey().getLastIndex()
		const int & getIndex() const { return m_index; } ;

		//! return the loading priority - higher mean load faster
		double getPriority() const { return m_state.m_priority; }

		//! return true if patch has children
		bool isDivided() const { return m_state.m_divided; }

		//! return true if patch is visible on the screen. note, if a patch is visible, then it parent is also visible
		bool isVisible() const { return m_state.m_visible; }

		bool isHidden() const { return m_state.m_hidden; }

		bool isNotHidden() const { return ! m_state.m_hidden; }

		//! return true if the patch is to big on the screen.
		bool isTooBig() const { return m_state.m_toBigOnScreen; };

		//! return the amount of pixels the patch has on the screen (approximate)
		const double & getSizeOnScreen() const { return m_state.m_sizeOnScreen; }

		bool hasDividedSubPatch() const { return m_state.m_subPatchDividedCount > 0; }

		//! return true when the patch children can't be visualized.
		bool hasVisiblityBlock() const { return m_state.m_visiblityBlock; }

		//! create sub patches
		void divide();

		//! remove sub patches
		void unify();

		//! remove the visibility block - so the patch children can be visualized.
		void removeVisiblityBlock();

		//! get the list of subPatches
		PatchVector & getSubPatches() { return m_subPatches; }

		//! get the list of subPatches
		const PatchVector & getSubPatches() const { return m_subPatches; }

		//! get a subPatches from it's index
		PYXPointer<Patch> getSubPatch(int index);

		//! get a subPatches from it's index
		PYXPointer<const Patch> getSubPatch(int index) const;

		//! get a pointer to the subPatches from it's index (for faster access - used by SurfaceMemento)
		Patch * getParentPtr() const;

		//! get a pointer to the subPatches from it's index (for faster access - used by SurfaceMemento)
		Patch * getSubPatchPtr(int index) const;

		//! get the BBox for this patch - used to decide if this patch is visible on the screen 
		SphereBBox & getBBox() { return m_bbox; };

		//! get index from given normalized UV
		PYXIcosIndex getIndexFromUV(const vec2 & uv,int resDepth) const;

		//! get normalized UV from index
		vec2 getUVFromIndex(const PYXIcosIndex & index) const;

		//! get normalized UV from 3D location
		vec2 getUVFromLocation(const vec3 & coord) const;

		bool intersects(const PYXIcosIndex & index) const;

		//Divide and unify logic API
	public:
		bool needsToBeDivided() const;

		bool canBeDivided() const;

		bool needsToBeUnified() const;

		bool canBeUnified() const;

	public:
		bool hasAllNeighbors() const;

		bool hasFullVertex(int index) const;

		bool hasFullVisibleVertex(int index) const;

	protected:
		double borrowPriorityFromSubPatchs();

		typedef bool (Surface::Patch::*BoolPropertyFunction)() const;
		typedef bool (Surface::Vertex::*BoolVertexPropertyFunction)() const;

		bool testAllNeighborPatches(const BoolPropertyFunction & func) const;
		bool testAllNeighborPatches(const BoolVertexPropertyFunction & func) const;

		bool testAllNeighborPatchesOfVertex(int index,const BoolPropertyFunction & func) const;

		bool hasNeighborPatch(const BoolPropertyFunction & func) const;

	public:
		struct PriorityCompare
		{
			bool operator()(const Patch & left, const Patch & right) const
			{
				return left.m_state.m_priority > right.m_state.m_priority;
			}
		};

		struct ScreenSizeCompare
		{
			bool operator()(const Patch & left, const Patch & right) const
			{
				return left.m_state.m_sizeOnScreen > right.m_state.m_sizeOnScreen;
			}
		};
	};

	/*!
	UVOffset - helper class to transform UV coordinates from one Patch to another (parent or child)

	to create a UVOffset use must specify a source and destination Patches keys:
		UVOffset(SourcePatchKey,DestinationPatchKey).

	Then you can use and UV coordinate created to the Source Patch and transform it (using UVOffset::transform()) to a UV coordinate inside the Destination Patch.

	Moreover, you can ask for the transformation parameters UOffset,UScale,VOffset and VScale
	
	The transformation is:
		destU = sourceU * ScaleU + OffsetU
		destV = sourceV * ScaleV + OffsetV
	*/
	class UVOffset
	{
	protected:
		double u_offset;
		double v_offset;
		double u_scale;
		double v_scale;

	public:
		UVOffset(const Patch::Key & source,const Patch::Key & dest);
		
	protected:
		// source.resolution > dest.resolution
		void addIndex(int index);

		// dest.resolution > source.resolution
		void removeIndex(int index);

	public:
		inline double getUOffset() const { return u_offset; }
		inline double getVOffset() const { return v_offset; }
		inline double getUscale() const { return u_scale; }
		inline double getVscale() const { return v_scale; }
	
		inline vec2 transform(const vec2 & coord) const
		{
			return vec2(coord[0]*u_scale+u_offset,coord[1]*v_scale+v_offset);
		}

	public:
		inline static int uvToIndex(const vec2 & uv)
		{
			return static_cast<int>(cml::clamp(floor(uv[0]*3),0.0,2.0)+3*cml::clamp(floor(uv[1]*3),0.0,2.0));
		}

		inline static vec2 zoomIn(int index,const vec2 & uv)
		{
			return vec2(uv[0]*3-(index%3),uv[1]*3-(index/3));
		}

		inline static vec2 zoomOut(int index,const vec2 & uv)
		{
			return vec2((uv[0]+(index%3))/3,(uv[1]+(index/3))/3);
		}
	};


	/*!
	Surface::Vertex - store information about connectivity between neighbor Surface::Patch.

	Try to thing that Surface::Vertex is a PYXIcosIndex that knows about all the Surface::Patchs that use this Index as a vertex.
	When a Surface::Patch is been inserted into a surface it conntect it self to all neded Surface::Vertex.

	but using the Surface::Vertex a patch can easly iterator on all it's neighbor Patches.

	Moreover, in order to speed some calculations, each vertex keep track on some attributes of it's Patchs:
	1. doesAllPatchesNotHidden() - return true if the Vertex as all it's patches and they are all not hidden
	2. doesAllPatchesDivded() - return true if the Vertex as all it's patches and they are all divided
	3. getMaxPriority() - return the maximum priority between the Vertex patches.
	*/
	class Vertex : public PYXObject, ObjectMemoryUsageCounter<Vertex>
	{
		friend class Surface::VertexSet;

	protected:
		int m_fullCount;
		PYXIcosIndex m_index;
		std::set<Surface::Patch*> m_patchSet;

	protected:
		int m_visibleCount;
		int m_notHiddenCount;
		int m_dividedCount;
		int m_hasDividedSubPatchesCount;
		double m_maxPriority;

	protected:
		Vertex(const PYXIcosIndex & index);
		
		static PYXPointer<Vertex> create(const PYXIcosIndex & index)
		{
			return PYXNEW(Vertex,index);
		}

		void addPatch(Surface::Patch * patch);
		void removePatch(Surface::Patch * patch);

	public:

		void refreshCount();
		int getPatchCount() const;
		const std::set<Surface::Patch*> & getPatches() const { return m_patchSet; }

		bool hasAllPatches() const;
		bool doesAllPatchesNotHidden() const;
		bool doesAllPatchesDivded() const;
		double getMaxPriority() const;
		void setMaxPriority(double newMaxPriority);
	};

	/*!
	Surface::VertexSet - the container of all Surface::Vertex used by the Surface

	This class is responsible to allocate and deallocate Surface::Vertex
	*/
	class VertexSet
	{
	protected:
		typedef std::map<PYXIcosIndex,PYXPointer<Vertex>> VertexMap;
		VertexMap m_vertexMap;
		
	public:
		VertexSet() {};

		std::vector<PYXPointer<Surface::Vertex>> addPatch(Surface::Patch & patch);	
		
		void removePatch(Surface::Patch & patch);		

	protected:
		VertexMap::iterator getVertex(const PYXIcosIndex & index);		
	};

	
	/*!
	Surface::Edge - store information about connectivity between neighbor Surface::Patch that have the share an edge.

	When a patch is been added to a surface - it would attach to all needed Surface::Edge.

	but using the Surface::Edge, a patch can easly sync elevation with enightboor patches.

	Moreover, in order to speed some calculations, each vertex keep track on some attributes of it's Patchs:
	1. doesAllPatchesNotHidden() - return true if the Vertex as all it's patches and they are all not hidden
	2. doesAllPatchesDivded() - return true if the Vertex as all it's patches and they are all divided
	3. getMaxPriority() - return the maximum priority between the Vertex patches.
	*/
	class Edge : public PYXObject, ObjectMemoryUsageCounter<Edge>
	{
		friend class Surface::EdgeSet;

	protected:
		PYXIcosIndex m_from;
		PYXIcosIndex m_to;
		Surface::Patch* m_left;
		Surface::Patch* m_right;
		PYXCoord2DInt m_leftUVBase;
		PYXCoord2DInt m_leftUVDirection;
		PYXCoord2DInt m_rightUVBase;
		PYXCoord2DInt m_rightUVDirection;

	protected:
		Edge(const PYXIcosIndex & from,const PYXIcosIndex & to);
		
		static PYXPointer<Edge> create(const PYXIcosIndex & from,const PYXIcosIndex & to)
		{
			return PYXNEW(Edge,from,to);
		}

		void addPatch(Surface::Patch * patch,int edgeIndex);
		void removePatch(Surface::Patch * patch,int edgeIndex);

	public:
		Surface::Patch * getOtherSide(Surface::Patch * side);
		const Surface::Patch * getOtherSide(const Surface::Patch * side) const;

		int getPatchCount();

		bool hasElevation();

		void copyEdgeElevation(Surface::Patch * source,Patch::ElevationData & sourceData,Surface::Patch * destination,Patch::ElevationData & destData);
	};

	/*!
	Surface::EdgeSet - the container of all Surface::Edge used by the Surface

	This class is responsible to allocate and deallocate Surface::Edge
	*/
	class EdgeSet
	{
	protected:
		typedef std::map<std::pair<PYXIcosIndex,PYXIcosIndex>,PYXPointer<Edge>> EdgeMap;
		EdgeMap m_edgeMap;
		
	public:
		EdgeSet() {};

		std::vector<PYXPointer<Surface::Edge>> addPatch(Surface::Patch & patch);
		
		void removePatch(Surface::Patch & patch);

	protected:
		EdgeMap::iterator getEdge(const PYXIcosIndex & from,const PYXIcosIndex & to);
	};

public:
	/*!
	Surface::Event - helper class to notify about surface tree changes
	*/
	class Event : public NotifierEvent
	{
	protected:
		PYXPointer<Surface::Patch> m_patch;

	public:
		static PYXPointer<Event> create(const PYXPointer<Surface::Patch> & patch)
		{
			return PYXNEW(Event,patch);
		}

		Event(const PYXPointer<Surface::Patch> & patch) : m_patch(patch)
		{
		}

		~Event()
		{
		}

	public:
		const PYXPointer<Surface::Patch> & getPatch()
		{
			return m_patch;
		}

		const Surface & getSurface()
		{
			return m_patch->getSurface();
		}
	};

public:
	//! Helper class used by SurfaceMemento to store information about the Surface Patches.
	template<class T> class Tree;

	//! Helper class to visit the surface
	class Visitor;

protected:
	//! first resolution patchs (there will be 90 of them)
	PatchVector m_patches;

	VertexSet m_vertexSet;
	EdgeSet m_edgeSet;

	boost::recursive_mutex m_patchesMutex;

	void popolateSurfacePatchForPool(const PYXIcosIndex & pool);

public:
	Surface();
	virtual ~Surface();

	static PYXPointer<Surface> create() { return PYXNEW(Surface); }

	PatchVector::iterator begin() {return m_patches.begin();}
	PatchVector::iterator end() {return m_patches.end();}


protected:
	PatchVector m_visiblePatches;
	PatchVector m_patchesNeedDividing;
	PatchVector m_patchesNeedUnifing;

	double getPatchPriority(const vec3 & eye,const vec3 & look,Patch & patch);
	void findVisiblePatchsInVector(CameraFrustum & frustum,const vec3 & eye,const vec3 & look,PatchVector & vector,CameraFrustum::CameraFrustumBoundary boundaryHint,int viewPortHeight,bool isHidden);

public:
	void findVisiblePatchs(const Camera & camera,const int & viewPortHeight);

	//! get sorted list of Leaf Visible Patches.
	PatchVector & getVisiblePatches() { return m_visiblePatches; }

	//! get sorted list of Leaf Visible Patches that needed to be divided.
	PatchVector & getPatchesNeedDividing() { return m_patchesNeedDividing; }

	//! get sorted list of Leaf Visible Patches that needed to be unified.
	PatchVector & getPatchesNeedUnifing() { return m_patchesNeedUnifing; }

	//! get sorted list of Leaf Visible Patches.
	const PatchVector & getVisiblePatches() const { return m_visiblePatches; }

	//! get sorted list of Leaf Visible Patches that needed to be divided.
	const PatchVector & getPatchesNeedDividing() const { return m_patchesNeedDividing; }

	//! get patch from key
	PYXPointer<Patch> getPatch(const Surface::Patch::Key & key);

	//! get patch from key
	PYXPointer<const Patch> getPatch(const Surface::Patch::Key & key) const;

protected:
	Notifier m_beforePatchDivided;
	Notifier m_afterPatchDivided;
	Notifier m_beforePatchUnified;
	Notifier m_afterPatchUnified;
	Notifier m_afterPatchVisibliyBlockRemoved;

	Notifier m_patchBecomeVisible;
	Notifier m_patchBecomeNotVisible;

public:
	Notifier & getBeforePatchDividedNotifer() { return m_beforePatchDivided; }
	Notifier & getAfterPatchDividedNotifer() { return m_afterPatchDivided; }
	Notifier & getBeforePatchUnifiedNotifer() { return m_beforePatchUnified; }
	Notifier & getAfterPatchUnifiedNotifer() { return m_afterPatchUnified; }
	Notifier & getAfterPatchVisibliyBlockRemoved() { return m_afterPatchVisibliyBlockRemoved; }

	Notifier & getPatchBecomeVisible() { return m_patchBecomeVisible; }
	Notifier & getPatchBecomeNotVisible() { return m_patchBecomeNotVisible; }
};

#include "surface_tree.h"

/*!

Surface::Visitor - helper class to visit the surface using boost::function

Example:

class DoSomething
{
	void do(const PYXPointer<Patch> & patch)
	{
		...do something on patch...
	}
}

DoSomething worker
Surface::Visitor(surface,boost::bind(&DoSomething::do,&worker,_1)); //will call worker.do(patch) on each patch on the surface

*/
class Surface::Visitor 
{
public:
	static void visitAll(Surface & surface,const boost::function<void(const PYXPointer<Surface::Patch> &)> & func)	
	{
		Surface::PatchVector::iterator it = surface.begin();

		while(it != surface.end())
		{
			func(*it);
			if ((*it)->isDivided())
			{
				visitAllChildren(*it,func);
			}
			++it;
		}
	}

	static void visitAllChildren(const PYXPointer<Surface::Patch> & patch,const boost::function<void(const PYXPointer<Surface::Patch> &)> & func)
	{
		if (!patch->isDivided())
		{
			return;
		}
		for(int i=0;i<9;i++)
		{
			const PYXPointer<Surface::Patch> & subPatch = patch->getSubPatch(i);
			func(subPatch);
			if (subPatch->isDivided())
			{
				visitAllChildren(subPatch,func);
			}
		}
	}

	//! Visit in Patch then it's Children order (DFS with visit node then child)
	static void visitVisible(Surface & surface,const boost::function<void(const PYXPointer<Surface::Patch> &)> & func)	
	{
		Surface::PatchVector::iterator it = surface.begin();

		while(it != surface.end())
		{
			if ((*it)->isVisible())
			{
				func(*it);
				if ((*it)->isDivided())
				{
					visitVisibleChildren(*it,func);
				}
			}
			++it;
		}
	}

	//! Visit in Patch then it's Children order (DFS with visit node then child)
	static void visitVisibleChildren(const PYXPointer<Surface::Patch> & patch,const boost::function<void(const PYXPointer<Surface::Patch> &)> & func)
	{
		if (!patch->isDivided())
		{
			return;
		}
		for(int i=0;i<9;i++)
		{
			const PYXPointer<Surface::Patch> & subPatch = patch->getSubPatch(i);
			if (subPatch->isVisible())
			{
				func(subPatch);
				if (subPatch->isDivided())
				{
					visitVisibleChildren(subPatch,func);
				}
			}
		}	
	}

	static void visitLeafs(Surface & surface,const boost::function<void(const PYXPointer<Surface::Patch> &)> & func)
	{
		Surface::PatchVector::iterator it = surface.begin();

		while(it != surface.end())
		{
			if ((*it)->isDivided())
			{
				visitLeafs(*it,func);
			} 
			else
			{
				func(*it);
			}
			++it;
		}
	}

	static void visitLeafs(const PYXPointer<Surface::Patch> & patch,const boost::function<void(const PYXPointer<Surface::Patch> &)> & func)
	{
		for(int i=0;i<9;i++)
		{
			PYXPointer<Surface::Patch> subPatch = patch->getSubPatch(i);
			
			if (subPatch->isDivided())
			{
				visitLeafs(subPatch,func);
			}
			else
			{
				func(subPatch);
			}
		}
	}
};

#endif