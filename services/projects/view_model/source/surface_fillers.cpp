/******************************************************************************
surface_fillers.cpp

begin		: 2010-08-15
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "surface_fillers.h"
#include "ray.h"
#include "view_open_gl_thread.h"
#include "performance_counter.h"
#include "garbage_collector.h"

#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/procs/cache.h"

void TextureCoordData::generate(float uOffset,float uScale,float vOffset,float vScale)
{
	//native resolution text coords
	for(int u=0;u<10;u++)
	{
		for(int v=0;v<10;v++)
		{
			data[u*10+v][0] = (u/9.0f*uScale+uOffset);
			data[u*10+v][1] = (v/9.0f*vScale+vOffset);
		}
	}

	//lower resolution text coords
	for(int u=0;u<4;u++)
	{
		for(int v=0;v<4;v++)
		{
			data[10*10+u*4+v][0] = (u/3.0f*uScale+uOffset);
			data[10*10+u*4+v][1] = (v/3.0f*vScale+vOffset);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// ElevationLoader
///////////////////////////////////////////////////////////////////////

AppProperty<double> ElevationLoader::elevationExaggerationFactor("Elevation","ExaggerationFactor",2.0,"scale factor of elevation exaggeration when visualizing the globe.");


ElevationLoader::ElevationLoader(const boost::intrusive_ptr<IProcess> & spViewPointProcess) : CostBasedThreadedMementoCreator<Surface::Patch::VertexBuffer>(1,1) //make one fast threads, one slow thread
{
	if (spViewPointProcess)
	{
		boost::intrusive_ptr<IProcess> output = spViewPointProcess->getOutput()->QueryInterface<IProcess>();

		//HACK: need to strip down the channel combiner, have a better way?
		if (output->getSpec()->getClass() == strToGuid("{6BCC2C4D-1353-442b-A7B2-6D57ED42C12D}"))
		{
			//get the elevation blender out :D
			output = output->getParameter(0)->getValue(1);

			//Add a memory cache to store the blender results for us
			m_spCoverage = safeAddCacheToPipeline(output,false)->getOutput()->QueryInterface<ICoverage>();
			//m_spCoverage = spViewPointProcess->getOutput()->QueryInterface<ICoverage>();
		}
		else
		{
			PYXTHROW(PYXException,"This should never happen, the viewpoint process internal state is different then expected");			
		}
	}
	bumpTag();
}

PYXPointer<VersionedMemento<Surface::Patch::VertexBuffer>> ElevationLoader::createMemento(const PYXPointer<Surface::Patch> & patch)
{
	PYXPointer<ElevationLoader::MementoItem> memento = MementoItem::create(*this,patch);

	if (!patch->elevations)
	{
		patch->borrowElevation();
	}
	if (!patch->vertices)
	{
		patch->updateVertices();
	}
	memento->setNewData(patch->vertices,0);
	memento->refreshData();
	return memento;
}

bool ElevationLoader::willLoadFast(const PYXPointer<Surface::Patch> & patch,PYXPointer<ElevationLoader::MementoItem> & memento)
{
	int tag;
	boost::intrusive_ptr<ICoverage> coverage;

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		tag = getTag();
		//no data - return nothing!
		if (!m_spCoverage)
		{
			return true;
		}
		coverage = m_spCoverage;
	}

	PYXRhombusFiller filler(patch->getRhombus(),4,11);

	std::vector<PYXPointer<PYXTile>> tiles = filler.getAllNeededTiles();

	for(unsigned int i=0;i<tiles.size();i++)
	{
		PYXCost cost = coverage->getFieldTileCost(tiles[i]->getRootIndex(),tiles[i]->getCellResolution(),0);

		if (cost.getMaximumCost() >= PYXCost::knDefaultCost.getMaximumCost())
		{
			return false;
		}
	}	

	return true;
}

void ElevationLoader::loadMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<ElevationLoader::MementoItem> & memento)
{
	PerformanceCounter::getValuePerformanceCounter("Elv. Fast Thread",0.5f,1.0f,0.5f)->setMeasurement((int) m_fastThread.getWaitingJobsCount());
	PerformanceCounter::getValuePerformanceCounter("Elv. Slow Thread",0.5f,1.0f,0.5f)->setMeasurement((int) m_slowThread.getWaitingJobsCount());

	const double K = elevationExaggerationFactor;

	PYXPointer<Surface::Patch::ElevationData> elevationData = Surface::Patch::ElevationData::create();

	int tag;
	boost::intrusive_ptr<ICoverage> coverage;

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		tag = getTag();

		coverage = m_spCoverage;
	}

	if (!coverage)
	{
		//we have no elevation...
		for(int u=0;u<10;u++)
		{
			for(int v=0;v<10;v++)
			{
				elevationData->data[u][v] = 1.0;
			}
		}
	}
	else
	{
		//start loading data
		PYXRhombusFiller filler(patch->getRhombus(),4,11);

		while ( ! filler.isReady())
		{
			bool wasError=false;
			PYXPointer<PYXTile> tile = filler.getNeededTile();		

			try
			{
				filler.addTile(tile,coverage->getFieldTile(tile->getRootIndex(),tile->getCellResolution(),0));
			}
			catch (PYXException& ex)
			{
				TRACE_INFO("failed to receive tile, try again. error was: " << ex.getFullErrorString());
				wasError=true;
			}
			catch(...)
			{
				TRACE_INFO("failed to receive tile, try again. unknown error");
				wasError=true;
			}

			if (wasError)
			{
				return;
			}
		}

		PYXRhombusFiller::Iterator it = filler.getIterator(0);
	
		int i = 0;
		while(!it.end())
		{
			double s = 1.0;

			if (it.hasValue())
			{
				s = (SphereMath::knEarthRadius+it.getValue().getDouble()*K)/SphereMath::knEarthRadius;
			}
			else {
				s = 1.0;
			}
		
			if (s!=s || s == std::numeric_limits<double>::infinity() || s == -std::numeric_limits<double>::infinity()) //aka isnan(s) || isinf(s)
			{
				s = 1.0;
			}

			if (s<0.8)
			{
				s=0.8;
			}
			if (s>1.2)
			{
				s=1.2;
			}

			elevationData->data[it.getUCoord()][it.getVCoord()] = s;
			++it;
			++i;
		}
	}

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		if (tag == getTag())
		{
			patch->updateElevation(elevationData);
		}
	}

	memento->setNewData(patch->vertices,tag);
}

///////////////////////////////////////////////////////////////////////
// RasterLoader
///////////////////////////////////////////////////////////////////////

bool RasterLoader::willLoadFast(const PYXPointer<Surface::Patch> & patch,PYXPointer<RasterLoader::MementoItem> & memento)
{
	int tag;
	boost::intrusive_ptr<ICoverage> coverage;

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		tag = getTag();
		//no data - return nothing!
		if (!m_spCoverage)
		{
			return true;
		}
		coverage = m_spCoverage;
	}

	PYXRhombusFiller filler(patch->getRhombus(),8,11);

	std::vector<PYXPointer<PYXTile>> tiles = filler.getAllNeededTiles();

	for(unsigned int i=0;i<tiles.size();i++)
	{
		PYXCost cost = coverage->getFieldTileCost(tiles[i]->getRootIndex(),tiles[i]->getCellResolution(),0);

		if (cost.getMaximumCost() >= PYXCost::knDefaultCost.getMaximumCost())
		{
			return false;
		}
	}

	return true;
}

void RasterLoader::loadMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<RasterLoader::MementoItem> & memento)
{
	PerformanceCounter::getValuePerformanceCounter("Fast Thread",0.5f,1.0f,0.5f)->setMeasurement((int) m_fastThread.getWaitingJobsCount());
	PerformanceCounter::getValuePerformanceCounter("Slow Thread",0.5f,1.0f,0.5f)->setMeasurement((int) m_slowThread.getWaitingJobsCount());

	int tag;
	boost::intrusive_ptr<ICoverage> coverage;

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		tag = getTag();
		//no data - return nothing!
		if (!m_spCoverage)
		{
			memento->setNewData(NULL,tag);
			return;
		}
		coverage = m_spCoverage;
	}

	PYXPointer<RasterData> buffer = RasterData::create();	

	PYXRhombusFiller filler(patch->getRhombus(),8,11);

	while ( ! filler.isReady())
	{
		bool wasError=false;
		PYXPointer<PYXTile> tile = filler.getNeededTile();
		PYXPointer<PYXValueTile> valueTile;

		try
		{

			PYXPointer<PYXValueTile> valueTile;
			{
				boost::recursive_mutex::scoped_lock lock(m_mutex);
				std::list<PYXPointer<PYXValueTile>>::iterator it = m_tilesCache.begin();
				while(it!=m_tilesCache.end() && (*it)->getTile() != *tile)
				{
					++it;
				}
				if (it!=m_tilesCache.end())
				{
					valueTile = *it;
					m_tilesCache.erase(it);
					m_tilesCache.push_front(valueTile);
				}
			}

			if (!valueTile)
			{
				valueTile = coverage->getFieldTile(tile->getRootIndex(),tile->getCellResolution(),0);
				if (valueTile)
				{
					boost::recursive_mutex::scoped_lock lock(m_mutex);
					//add it into cache if we are still valid...
					if (tag == getTag())
					{
						if (m_tilesCache.size() > 10)
						{
							m_tilesCache.pop_back();
						}
						m_tilesCache.push_front(valueTile);
					}
				}
			}

			filler.addTile(tile,valueTile);
		}
		catch (PYXException& ex)
		{
			TRACE_INFO("failed to receive tile, try again. error was: " << ex.getFullErrorString());
			wasError=true;
		}
		catch(...)
		{
			TRACE_INFO("failed to receive tile, try again. unknown error");
			wasError=true;
		}

		if (wasError)
		{
			return;
		}
	}
		
	PYXRhombusFiller::IteratorWithLUT it = filler.getIteratorWithLUT(0);
	//PYXRhombus rhombus = patch->getRhombus();//.getSubRhombus(1,1);
	
	const unsigned char* pv = it.getValue().getUInt8Ptr(0);

	while(!it.end())
	{
		/*
		int u;
		int v;
		if (rhombus.isInside(it.getIndex(),&u,&v))
		{			
			if (it.getUCoord()==u && it.getVCoord()==v)
			{
				//default gray color
				buffer->rgb[it.getVCoord()][it.getUCoord()][0] = 0xE0;
				buffer->rgb[it.getVCoord()][it.getUCoord()][1] = 0xE0;
				buffer->rgb[it.getVCoord()][it.getUCoord()][2] = 0xE0;

				if (patch->getRhombus().getIndex(0).isAncestorOf(it.getIndex()))
				{
					buffer->rgb[it.getVCoord()][it.getUCoord()][0] = 0x00;
					buffer->rgb[it.getVCoord()][it.getUCoord()][1] = 0xFF;
					buffer->rgb[it.getVCoord()][it.getUCoord()][2] = 0x00;
				}
				if (patch->getRhombus().getIndex(1).isAncestorOf(it.getIndex()))
				{
					buffer->rgb[it.getVCoord()][it.getUCoord()][0] = 0x80;
					buffer->rgb[it.getVCoord()][it.getUCoord()][1] = 0x80;
					buffer->rgb[it.getVCoord()][it.getUCoord()][2] = 0x80;
				}
				if (patch->getRhombus().getIndex(2).isAncestorOf(it.getIndex()))
				{
					buffer->rgb[it.getVCoord()][it.getUCoord()][0] = 0x00;
					buffer->rgb[it.getVCoord()][it.getUCoord()][1] = 0x00;
					buffer->rgb[it.getVCoord()][it.getUCoord()][2] = 0xFF;
				}
			}
			else
			{
				buffer->rgb[it.getVCoord()][it.getUCoord()][0] = u;
				buffer->rgb[it.getVCoord()][it.getUCoord()][1] = it.getUCoord();
				buffer->rgb[it.getVCoord()][it.getUCoord()][2] = 0x00;
			}
		}
		else
		{
			//error
			buffer->rgb[it.getVCoord()][it.getUCoord()][0] = 0xE0;
			buffer->rgb[it.getVCoord()][it.getUCoord()][1] = 0x00;
			buffer->rgb[it.getVCoord()][it.getUCoord()][2] = 0x00;
		}
		*/
		
		
		if (it.hasValue())
		{
			memcpy(&buffer->rgb[it.getVCoord()][it.getUCoord()][0],pv,sizeof(RasterData::Spec::TexelType)*RasterData::Spec::Channels);
		}
		else 
		{
			//default gray color
			memset(&buffer->rgb[it.getVCoord()][it.getUCoord()][0],0xE0,sizeof(RasterData::Spec::TexelType)*RasterData::Spec::Channels);			
		}
		
				
		++it;		
	}	

	memento->setNewData(buffer,tag);
}


///////////////////////////////////////////////////////////////////////
// VectorLoader
///////////////////////////////////////////////////////////////////////


VectorLoader::VectorLoader(ViewOpenGLThread * viewOpenGLThread) 
	: CostBasedThreadedMementoCreator<VectorData>(2,1), //make two fast threads, 1 slow thread 
	  m_viewOpenGLThread(viewOpenGLThread)
{
	m_viewOpenGLThread->getViewPortProcessChangeNotifier().attach(this,&VectorLoader::newViewportPorcess);
}

VectorLoader::~VectorLoader()
{
	m_viewOpenGLThread->getViewPortProcessChangeNotifier().detach(this,&VectorLoader::newViewportPorcess);
}

void VectorLoader::createNewVecotrFiller(const boost::intrusive_ptr<IProcess> & spProc,boost::intrusive_ptr<IProcess> & coverageProcess)
{
	TRACE_INFO("Create new Vectors loader for " << spProc->getProcName());
	
	// Wrap with rasterizer.
	boost::intrusive_ptr<IProcess> spRasterizerProc = PYXCOMCreateInstance<IProcess>(strToGuid("{E82F5DB8-3BF9-48b5-B529-64BB7819DD38}"));
	assert(spRasterizerProc);
	spRasterizerProc->getParameter(0)->addValue(spProc);	
	spRasterizerProc->initProc();

	assert(spRasterizerProc->getInitState() == IProcess::knInitialized && "rastersizer was unable to inialized");

	boost::intrusive_ptr<IFeatureCollection> featureCollection = spProc->QueryInterface<IFeatureCollection>();

	PYXPointer<FeatureIterator> fcIt = featureCollection->getIterator();

	int count =0;
	while (count < 100 && ! fcIt->end())
	{
		count++;
		fcIt->next();
	}

	//for small features... just create the texture when needed.
	if (count < 100)
	{
		coverageProcess = spRasterizerProc;
	}
	else
	{
		TRACE_INFO(spProc->getProcName() << " has more then 100 features, creating coverageCache to speed up the raster.");
		// Add on a cache.
		boost::intrusive_ptr<IProcess> spCacheProc = PYXCOMCreateInstance<IProcess>(strToGuid("{83F35C37-5D0A-41c9-A937-F8C9C1E86850}"));
		assert(spCacheProc);
		spCacheProc->getParameter(0)->addValue(spRasterizerProc);
		spCacheProc->initProc();

		assert(spCacheProc->getInitState() == IProcess::knInitialized && "cache was unable to inialized");

		coverageProcess = spCacheProc;
	}
}

void VectorLoader::newViewportPorcess(PYXPointer<NotifierEvent> e)
{
	ProcessEvent * processEvent = dynamic_cast<ProcessEvent*>(e.get());

	if (processEvent != NULL)
	{		
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		m_tilesCache.clear();
		bumpTag();
		boost::intrusive_ptr<IViewPoint> viewPoint = processEvent->getProcess()->QueryInterface<IViewPoint>();

		assert(viewPoint);

		m_vectorsOrder = viewPoint->getFeatureCollectionPipelines();

		VectorsCoverageMap newCoverages;

		for(unsigned int i=0;i<m_vectorsOrder.size();i++)
		{
			boost::intrusive_ptr<IProcess> process = m_vectorsOrder[i];
			if (m_vectorsCoverage.find(process) != m_vectorsCoverage.end())
			{
				newCoverages[process] = m_vectorsCoverage[process];
			}
			else
			{
				boost::intrusive_ptr<IProcess> coverageProcess;

				createNewVecotrFiller(process,coverageProcess);

				newCoverages[process] = coverageProcess;
			}
		}
		std::swap(m_vectorsCoverage,newCoverages);

		if (m_vectorsOrder.size()>0)
		{
			boost::intrusive_ptr<IProcess> spBlenderProcess = PYXCOMCreateInstance<IProcess>(strToGuid("{00B7D55E-433A-4767-9C77-B5E276762A97}"));
			assert(spBlenderProcess);

			for(unsigned int i=0;i<m_vectorsOrder.size();i++)
			{
				boost::intrusive_ptr<IProcess> process = m_vectorsOrder[i];
				boost::intrusive_ptr<IProcess> coverageProcess = m_vectorsCoverage[process];

				spBlenderProcess->getParameter(0)->addValue(coverageProcess);
			}

			spBlenderProcess->initProc();

			assert(spBlenderProcess->getInitState() == IProcess::knInitialized && "cache was unable to inialized");

			m_blenderProcess = spBlenderProcess;
			m_blenderProcessOutput = m_blenderProcess->getOutput()->QueryInterface<ICoverage>();
		}
		else
		{
			m_blenderProcess.reset();
			m_blenderProcessOutput.reset();
		}

		assert(m_vectorsOrder.size() == m_vectorsCoverage.size() && "the list are worng. this is bad");
	}
}

bool VectorLoader::willLoadFast(const PYXPointer<Surface::Patch> & patch,PYXPointer<VectorLoader::MementoItem> & memento)
{
	boost::intrusive_ptr<ICoverage> coverage;
	int tag;
	
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		tag = getTag();
		coverage = m_blenderProcessOutput;
	}

	if (!coverage)
	{
		return true;
	}


	PYXRhombusFiller filler(patch->getRhombus(),8,11);

	std::vector<PYXPointer<PYXTile>> tiles = filler.getAllNeededTiles();

	for(unsigned int i=0;i<tiles.size();i++)
	{
		PYXCost cost = coverage->getFieldTileCost(tiles[i]->getRootIndex(),tiles[i]->getCellResolution(),0);

		if (cost.getMaximumCost() >= PYXCost::knDefaultCost.getMaximumCost())
		{
			return false;
		}
	}

	return true;
}

void VectorLoader::loadMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<VectorLoader::MementoItem> & memento)
{
	PYXPointer<VectorData> buffer = VectorData::create();
	memset(buffer->rgb,0,sizeof(buffer->rgb));
	buffer->allEmpty = true;

	boost::intrusive_ptr<ICoverage> coverage;
	int tag;
	
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		tag = getTag();
		coverage = m_blenderProcessOutput;
	}

	if (!coverage)
	{
		memento->setNewData(buffer,tag);
		return;
	}

	PYXRhombusFiller filler(patch->getRhombus(),8,11);

	assert(coverage && "Coverage is null. can't raster that !");

	while ( ! filler.isReady())
	{
		bool wasError=false;
		PYXPointer<PYXTile> tile = filler.getNeededTile();
		PYXPointer<PYXValueTile> valueTile;

		try
		{

			PYXPointer<PYXValueTile> valueTile;
			{
				boost::recursive_mutex::scoped_lock lock(m_mutex);
				std::list<PYXPointer<PYXValueTile>>::iterator it = m_tilesCache.begin();
				while(it!=m_tilesCache.end() && (*it)->getTile() != *tile)
				{
					++it;
				}
				if (it!=m_tilesCache.end())
				{
					valueTile = *it;
					m_tilesCache.erase(it);
					m_tilesCache.push_front(valueTile);
				}
			}

			if (!valueTile)
			{
				valueTile = coverage->getFieldTile(tile->getRootIndex(),tile->getCellResolution(),0);
				if (valueTile)
				{
					boost::recursive_mutex::scoped_lock lock(m_mutex);
					//add it into cache if we are still valid...
					if (tag == getTag())
					{
						if (m_tilesCache.size() > 10)
						{
							m_tilesCache.pop_back();
						}
						m_tilesCache.push_front(valueTile);
					}
				}
			}

			filler.addTile(tile,valueTile);
		}
		catch (PYXException& ex)
		{
			TRACE_INFO("failed to receive tile, try again. error was: " << ex.getFullErrorString());
			wasError=true;
		}
		catch(...)
		{
			TRACE_INFO("failed to receive tile, try again. unknown error");
			wasError=true;
		}

		if (wasError)
		{
			return;
		}
	}

	//PYXRhombusFiller::Iterator it = filler.getIterator(0);
	PYXRhombusFiller::IteratorWithLUT it = filler.getIteratorWithLUT(0);
	
	buffer->allEmpty = false;

	while(!it.end())
	{
		if (it.hasValue())
		{	
			const unsigned char* pv = it.getValue().getUInt8Ptr(0);

			unsigned char newColor[4];
			unsigned char * bufferPos = buffer->rgb[it.getVCoord()][it.getUCoord()];
			if (bufferPos[3]==0)
			{
				//copy the vector color
				memcpy(bufferPos,pv,3);
				bufferPos[3]=0xCC;
			}
			else
			{
				//blend colors
				memcpy(newColor,pv,3);
				for(int i=0; i<3; i++)
				{
					bufferPos[i] = (bufferPos[i]+newColor[i])/2;
				}
			}
		}
		++it;
	}
	
	memento->setNewData(buffer,tag);
}

///////////////////////////////////////////////////////////////////////
// ViewPointFiller::ProcessSet
///////////////////////////////////////////////////////////////////////

ViewPointFiller::ProcessSet::ProcessSet()
{
}

ViewPointFiller::ProcessSet::ProcessSet(const std::vector<boost::intrusive_ptr<IProcess>> & updatedPipelines)
{
	std::set<ProcRef> updatedPipelinesSet;

	for(unsigned int i=0;i<updatedPipelines.size();i++)
	{
		 updatedPipelinesSet.insert(ProcRef(updatedPipelines[i]));
	}

	m_processSet.swap(updatedPipelinesSet);
	m_pipelines = updatedPipelines;
}

void ViewPointFiller::ProcessSet::setPipelines(const std::vector<boost::intrusive_ptr<IProcess>> & updatedPipelines)
{
	std::set<ProcRef> updatedPipelinesSet;

	for(unsigned int i=0;i<updatedPipelines.size();i++)
	{
		 updatedPipelinesSet.insert(ProcRef(updatedPipelines[i]));
	}

	m_removedPipelines.clear();
	m_newPipelines.clear();
		
	std::set_difference(m_processSet.begin(),m_processSet.end(),
						updatedPipelinesSet.begin(),updatedPipelinesSet.end(),
						std::inserter(m_removedPipelines,m_removedPipelines.begin()));

	std::set_difference(updatedPipelinesSet.begin(),updatedPipelinesSet.end(),
						m_processSet.begin(),m_processSet.end(),
						std::inserter(m_newPipelines,m_newPipelines.begin()));

	m_processSet.swap(updatedPipelinesSet);
	m_pipelines = updatedPipelines;
}

boost::intrusive_ptr<IProcess> ViewPointFiller::ProcessSet::operator[](const ProcRef & procRef)
{
	for(unsigned int i=0;i<m_pipelines.size();i++)
	{
		if (procRef == ProcRef(m_pipelines[i]))
		{
			return m_pipelines[i];
		}
	}
	return boost::intrusive_ptr<IProcess>();
}

const std::vector<boost::intrusive_ptr<IProcess>> & ViewPointFiller::ProcessSet::getPipelines()
{
	return m_pipelines;
}

bool ViewPointFiller::ProcessSet::hasChanged() const
{
	return m_newPipelines.size() > 0 || m_removedPipelines.size() > 0;
}

bool ViewPointFiller::ProcessSet::empty() const 
{
	return m_pipelines.empty();
}

const std::set<ProcRef> & ViewPointFiller::ProcessSet::getNewPipelines()
{
	return m_newPipelines;
}

const std::set<ProcRef> & ViewPointFiller::ProcessSet::getRemovedPipelines()
{
	return m_removedPipelines;
}

///////////////////////////////////////////////////////////////////////
// ViewPointFiller
///////////////////////////////////////////////////////////////////////

void ViewPointFiller::validateViewpoint(const boost::intrusive_ptr<IProcess> & viewPointPorcess)
{
	m_viewPointPorcess = viewPointPorcess;

	if (!m_viewPointPorcess)
	{
		m_viewPoint.reset();
		collectMementos();
		return;
	}

	m_viewPoint = m_viewPointPorcess->QueryInterface<IViewPoint>();

	m_vectorsPipelines.setPipelines(m_viewPoint->getFeatureCollectionPipelines());
	m_elevationPipelines.setPipelines(m_viewPoint->getElevationPipelines());
	m_coveragePipelines.setPipelines(m_viewPoint->getCoveragePipelines());
	
	if (m_elevationPipelines.hasChanged())
	{
		collectMementos();
		if (!m_elevationPipelines.empty())
		{
			m_hasElevation = true;
			m_elevationsMemento = SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>::create(m_surface,
									ElevationLoader::create(m_viewPointPorcess));
		}
		else
		{
			m_hasElevation = false;
			m_elevationsMemento = SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>::create(m_surface,
									ElevationLoader::create(NULL));
		}
		GarbageCollector::getInstance()->startDestroyObjects();
	}
	else
	{
		bool hasCoverages = m_coveragePipelines.getPipelines().size() > 0;

		GarbageCollector::getInstance()->startDestroyObjects();
	}
}

bool ViewPointFiller::hasElevation() const
{
	return m_hasElevation;
}

void ViewPointFiller::collectMementos()
{
	GarbageCollector::getInstance()->collect(m_elevationsMemento);

	if (m_elevationsMemento)
	{
		m_elevationsMemento->clearAllMementos();
	}

	m_elevationsMemento.reset();
}

ViewPointFiller::ViewPointFiller(ViewOpenGLThread * thread) : m_hasElevation(false)
{
	m_thread = thread;
	m_surface = Surface::create();

	m_elevationsMemento = SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>::create(m_surface,
									ElevationLoader::create(NULL));
}

ViewPointFiller::~ViewPointFiller()
{
	collectMementos();
	GarbageCollector::getInstance()->startDestroyObjects();
}

///////////////////////////////////////////////////////////////////////
// addCacheToPipeline function
///////////////////////////////////////////////////////////////////////

//helper function
boost::intrusive_ptr<IProcess> safeAddCacheToPipeline(const boost::intrusive_ptr<IProcess> process,bool persistence)
{
	{
		//check if the process is a cache already
		boost::intrusive_ptr<ICache> cache = process->QueryInterface<ICache>();	
		if (cache)
		{
			//we already have a cache!!!
			return process;
		}
	}

	//Add a new cache at the end of the pipeline
	boost::intrusive_ptr<IProcess> spCacheProc = PYXCOMCreateInstance<IProcess>(strToGuid("{83F35C37-5D0A-41c9-A937-F8C9C1E86850}"));
	assert(spCacheProc);
	{
		//set persistence type
		boost::intrusive_ptr<ICache> cache = spCacheProc->QueryInterface<ICache>();
		assert(cache);
		cache->setCachePersistence(persistence);
	}

	spCacheProc->setProcName(process->getProcName() + " + temp cache");
	spCacheProc->getParameter(0)->addValue(process);
	spCacheProc->initProc();

	assert(spCacheProc->getInitState() == IProcess::knInitialized && "cache was unable to inialized");

	return spCacheProc;
}