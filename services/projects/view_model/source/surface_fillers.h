#pragma once
#ifndef VIEW_MODEL__SURFACE_FILLERS_H
#define VIEW_MODEL__SURFACE_FILLERS_H
/******************************************************************************
surface.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "surface_memento.h"
#include "cml_utils.h"
#include "open_gl_vbo.h"

#include "pyxis/procs/viewpoint.h"
#include "pyxis/utility/app_services.h"


class TextureCoordData : public PYXObject, ObjectMemoryUsageCounter<TextureCoordData>
{
public:
	static PYXPointer<TextureCoordData> create() { return PYXNEW(TextureCoordData); }	
	
	TextureCoordData(): vbo(OpenGLVBO::knArrayBuffer,OpenGLVBO::knStatic) {};
	virtual ~TextureCoordData() {};
public:
	float data[10*10+4*4][2];

	OpenGLVBO vbo;
	
	void generate(float uOffset=0.0,float uScale=1.0,float vOffset=0.0,float vScale=1.0);
};

class TextureData : public PYXObject, ObjectMemoryUsageCounter<TextureData>
{
public:
	static PYXPointer<TextureData> create() { return PYXNEW(TextureData); }	

	OpenGLTexture m_texture;

	TextureData() : m_texture(OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGB) {};
	virtual ~TextureData() {};
};

class RasterData : public PYXObject, ObjectMemoryUsageCounter<RasterData>
{
public:
	struct Spec
	{
		typedef unsigned char TexelType;
		static const int Width = 82;
		static const int Height = 82;

		static const int PowerTwoWidth = 128;
		static const int PowerTwoHeight = 128;

		static const int Channels = 3;

		static const OpenGLTexture::TextureType TextureType   = OpenGLTexture::knTexture2D;
		static const OpenGLTexture::TextureFormat TextureFormat = OpenGLTexture::knTextureRGB;
	};

	static PYXPointer<RasterData> create() { return PYXNEW(RasterData); }

	OpenGLTexture m_texture;
	Spec::TexelType rgb[Spec::Height][Spec::Width][Spec::Channels];

	RasterData() : m_texture(Spec::TextureType,Spec::TextureFormat) {};
	virtual ~RasterData() {};
};

class VectorData : public PYXObject, ObjectMemoryUsageCounter<VectorData>
{
public:
	static PYXPointer<VectorData > create() { return PYXNEW(VectorData); }

	OpenGLTexture m_texture;
	unsigned char rgb[82][82][4];
	int generatedToken;
	bool allEmpty;

	VectorData () : m_texture(OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGBA), generatedToken(0), allEmpty(true) {};
	virtual ~VectorData () {};
};

class RGBData : public PYXObject, ObjectMemoryUsageCounter<RGBData>
{
public:
	static PYXPointer<RGBData> create() { return PYXNEW(RGBData); }

	RGBData() {};
	virtual ~RGBData() {};
public:
	//unsigned char rgb[244][244][3];
	unsigned char rgb[82][82][3];
};



/*!
ElevationLoader - create VertexBuffer from input Coverage (right now it's the ViewPointProcess field index 1)
*/
class ElevationLoader : public CostBasedThreadedMementoCreator<Surface::Patch::VertexBuffer>
{
protected:
	boost::intrusive_ptr<ICoverage> m_spCoverage;

	static AppProperty<double> elevationExaggerationFactor;

	ElevationLoader(const boost::intrusive_ptr<IProcess> & spViewPointProcess);

public:
	static PYXPointer<ElevationLoader> create(const boost::intrusive_ptr<IProcess> & spViewPointProcess)
	{
		return PYXNEW(ElevationLoader,spViewPointProcess);
	}

	virtual PYXPointer<VersionedMemento<Surface::Patch::VertexBuffer>> createMemento(const PYXPointer<Surface::Patch> & patch);	

protected:
	//virtual PYXPointer<VertexBuffer> createMemento(const PYXPointer<Surface::Patch> & patch);	
	virtual bool willLoadFast(const PYXPointer<Surface::Patch> & patch,PYXPointer<ElevationLoader::MementoItem> & memento);

	virtual void loadMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<ElevationLoader::MementoItem> & memento);	

};


/*!
RasterLoader - create RasterData from input Coverage (right now it's the ViewPointProcess field index 0)
*/
class RasterLoader : public CostBasedThreadedMementoCreator<RasterData>
{
protected:
	boost::intrusive_ptr<ICoverage> m_spCoverage;
	std::list<PYXPointer<PYXValueTile>> m_tilesCache;

	RasterLoader(const boost::intrusive_ptr<IProcess> & spViewPointProcess) : CostBasedThreadedMementoCreator<RasterData>(2,1) //make two fast threads, 1 slow thread
	{
		newViewportProcess(spViewPointProcess);
	}

public:
	void newViewportProcess(const boost::intrusive_ptr<IProcess> & spViewPointProcess)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (spViewPointProcess)
		{
			m_spCoverage = spViewPointProcess->getOutput()->QueryInterface<ICoverage>();
		}
		else
		{
			m_spCoverage.reset();
		}
		m_tilesCache.clear();
		bumpTag();
	}

public:
	static PYXPointer<RasterLoader> create(const boost::intrusive_ptr<IProcess> & spViewPointProcess)
	{
		return PYXNEW(RasterLoader,spViewPointProcess);
	}

protected:
	virtual bool willLoadFast(const PYXPointer<Surface::Patch> & patch,PYXPointer<RasterLoader::MementoItem> & memento);

	virtual void loadMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<RasterLoader::MementoItem> & memento);
};


/*!
VectorLoader - create VectorData from input feature collection pipeliens of the viewPoint process
*/
class VectorLoader : public CostBasedThreadedMementoCreator<VectorData>
{
	friend class ViewOpenGLThread;
protected:
	typedef std::map<boost::intrusive_ptr<IProcess>,boost::intrusive_ptr<IProcess>> VectorsCoverageMap;
	typedef std::vector<boost::intrusive_ptr<IProcess>> VectorsOrder;
	VectorsCoverageMap m_vectorsCoverage;
	VectorsOrder	   m_vectorsOrder;
	ViewOpenGLThread * m_viewOpenGLThread;

	PYXPointer<IProcess> m_blenderProcess;
	PYXPointer<ICoverage> m_blenderProcessOutput;

	std::list<PYXPointer<PYXValueTile>> m_tilesCache;

	void newViewportPorcess(PYXPointer<NotifierEvent> e);

	void createNewVecotrFiller(const boost::intrusive_ptr<IProcess> & process,boost::intrusive_ptr<IProcess> & coverageProcess);

	VectorLoader(ViewOpenGLThread * viewOpenGLThread);
	virtual ~VectorLoader();

public:
	static PYXPointer<VectorLoader> create(ViewOpenGLThread * viewOpenGLThread)
	{
		return PYXNEW(VectorLoader,viewOpenGLThread);
	}

protected:
	virtual bool willLoadFast(const PYXPointer<Surface::Patch> & patch,PYXPointer<VectorLoader::MementoItem> & memento);

	virtual void loadMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<VectorLoader::MementoItem> & memento);	
};

/*!
ViewPointFiller - manage the creation of memento from a ViewPoint process.

This class is responsible for creating all needed mementos to generate the infromation from a view point
1. Elevation Memento - to load data from elevations pipelines
2. Coverage Memento - to load coverage from coverage pipelines
3. Vector Memeneto - to raster feature collection pipelines.
4. Annotation Memento - to create annotation from feature collection pipelines.

This class responsible to follow the changes of the ViewPoint Process inputs. 
The class track changes and create/destory the needed mementos. This class suppose to make
as few changes in the mementos, depened on the input changes, to make view more stable
*/
class ViewPointFiller : public PYXObject
{
protected:
	/*!
	ProcessSet - helper class to track changes in the input pipelines
	
	This class keep track of the ProcRef() of the process vector. 
	Tracks any pipelines that were added or removed from the list.
	
	Note: This class doesn't track change of order. 
		For example, if the pipeline vector is (p1,p2),
		and we update the the pipeline vector to (p2,p1) - hasChaged would return false.
		
	Note: This class keep track of ProcRef() and not Ptr<IPorcess>.
		For example, if the pipeline vector is (p1). and we inc the version of p1.
		and then update the ppiilines vector to (p1) - hasChanged would return true.
		and the old procref p1 would be in removedPipelines.
		and the new procref p1 would be in newPipelines.
	*/
	class ProcessSet
	{
	protected:
		std::vector<boost::intrusive_ptr<IProcess>> m_pipelines;
		std::set<ProcRef> m_processSet;
		std::set<ProcRef> m_removedPipelines;
		std::set<ProcRef> m_newPipelines;

	public:
		//! update the pipeline list, track changes from last update
		void setPipelines(const std::vector<boost::intrusive_ptr<IProcess>> & updatedPipelines);
		
		//! get the vector of the piplines
		const std::vector<boost::intrusive_ptr<IProcess>> & getPipelines();
		
		//! return true if the last call to setPipelines has change the pipeline set
		bool hasChanged() const;

		bool empty() const;
		
		//! return a set of all new pipelines added (new procref if process was modified)
		const std::set<ProcRef> & getNewPipelines();
		
		//! return a set of all removed pipelines (old procref if process was modified)
		const std::set<ProcRef> & getRemovedPipelines();

		//! return the Ptr<IProcess> from a given ProcRef. return null if not found
		boost::intrusive_ptr<IProcess> ViewPointFiller::ProcessSet::operator[](const ProcRef & procRef);

		//! create an empty process set
		ProcessSet();
		
		//! create a process set with inital process vector
		ProcessSet(const std::vector<boost::intrusive_ptr<IProcess>> & updatedPipelines);
	};

protected:
	ViewOpenGLThread *  m_thread;
	

protected:
	boost::intrusive_ptr<IProcess> m_viewPointPorcess;
	boost::intrusive_ptr<IViewPoint> m_viewPoint;

protected:
	PYXPointer<Surface> m_surface;	
	bool m_hasElevation;

	ProcessSet m_vectorsPipelines;
	ProcessSet m_coveragePipelines;
	ProcessSet m_elevationPipelines;

	PYXPointer<SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>> m_elevationsMemento;
	
public:
	void validateViewpoint(const boost::intrusive_ptr<IProcess> & m_viewPointPorcess);

public:
	const PYXPointer<Surface> & getSurface() { return m_surface; }
	const PYXPointer<SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>> & getElevations() { return m_elevationsMemento; };

	bool hasElevation() const;

public:
	void collectMementos();

public:
	static PYXPointer<ViewPointFiller> create(ViewOpenGLThread * thread)
	{
		return PYXNEW(ViewPointFiller,thread);
	}

	virtual ~ViewPointFiller();

protected:
	ViewPointFiller(ViewOpenGLThread * thread);
};


//helper function
boost::intrusive_ptr<IProcess> safeAddCacheToPipeline(const boost::intrusive_ptr<IProcess> process,bool persistence = false);

#endif