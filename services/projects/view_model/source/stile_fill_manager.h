#pragma once
#ifndef VIEW_MODEL__STILE_FILL_MANAGER_H
#define VIEW_MODEL__STILE_FILL_MANAGER_H
/******************************************************************************
stile_fill_manager.h

begin		: 2009-10-18
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "view_model_api.h"
#include "stile.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/joblistsp.h"
#include "pyxis/utility/object.h"

// forward declarations
struct IViewPoint;

// Helper storage class for vector layers.
class VectorInput
{
public:
	VectorInput(boost::intrusive_ptr<ICoverage> spCov, int rgbaColour, ProcRef procrefFC) 
	{
		m_spCov = spCov;
		m_rgbaColour = rgbaColour;
		m_procrefFC = procrefFC;
	}

	int m_rgbaColour;
	boost::intrusive_ptr<ICoverage> m_spCov;
	ProcRef m_procrefFC;
};

/*!
This class is responsible for getting data from the pipeline and putting that data into
STiles that will be used to visualize the data.  It is intended that a new instance of STileFillManager will
be created every time the pipeline changes that is being visualized.  That way, a new set of
threads will be created to service the new pipeline, and the old version of this object can be 
shut down and die gracefully (but possibly slowly) as the operating threads complete their jobs.

The second major function is to allow tuning of the entire process of filling STile data.  The
number of threads allocated to each job and how the various tasks involved in filling an STile
are broken up are handled by this class.

This class operates by maintaining a list of pending jobs for each of the STiles that are still
incomplete.  The jobs are things like "fetching raster data" or "filling raster data". 
*/
//! This class is responsible for getting data from pipelines and populating STiles.
class VIEW_MODEL_API STileFillManager : public PYXObject
{
public:

	static PYXPointer<STileFillManager> create(
		PYXPointer<IViewModel> view,
		boost::intrusive_ptr<ICoverage> spCov, 
		int mipLevel, 
		const ProcRef& procref, 
		boost::intrusive_ptr<IViewPoint> spViewPoint
		)
	{
		PYXPointer<STileFillManager> spManager = PYXNEW(STileFillManager, view, spCov, mipLevel, procref, spViewPoint);
		spManager->initialize(spManager);
		return spManager;
	}

	static void waitForThreadsToFinish();

private:
	//! Constructor
	STileFillManager(PYXPointer<IViewModel> view, boost::intrusive_ptr<ICoverage> spCov, int mipLevel, const ProcRef& procref, boost::intrusive_ptr<IViewPoint> spViewPoint);

	virtual ~STileFillManager()
	{
	}

	//! the second half of the constructor...
	void initialize(PYXPointer<STileFillManager> spThis);

public:

	//! Set a new buch of STiles to process.
	void SetTiles(std::vector< PYXPointer<STile> >& vecTiles);

	void SortQueue(JobListsp<WeakPointerSTile> & jobList);

	//! Stop all processing
	void Stop();

	ProcRef getProcRef() { return m_procRef; }

	PYXPointer<IViewModel> getViewHandle() { return m_view; }

	boost::intrusive_ptr<IViewPoint> getViewPoint() { return m_spViewPoint; }

	std::string getIdentity() { return m_strProcessIdentity; }

	int getMipLevel() { return m_mipLevel; }

	boost::intrusive_ptr<ICoverage> getCoverage() { return m_spCoverage; }

	std::vector<VectorInput> m_featureCoverages;

private:
	JobListsp<WeakPointerSTile> m_getFromCache;
	JobListsp<WeakPointerSTile> m_getRasterData;
	JobListsp<WeakPointerSTile> m_fillRasterData;
	JobListsp<WeakPointerSTile> m_getElevationData;
	JobListsp<WeakPointerSTile> m_fillElevationData;
	JobListsp<WeakPointerSTile> m_vector;
	JobListsp<WeakPointerSTile> m_writeCache;
	JobListsp<WeakPointerSTile> m_cleanUp;
	JobListsp<WeakPointerSTile> m_IconFill;


	//! Pointer to the view model
	PYXPointer<IViewModel> m_view;

	//! The raster data source
	boost::intrusive_ptr<ICoverage> m_spCoverage;

	//! the identity of the process for caching directory purposes.
	std::string m_strProcessIdentity;

	//! The mip level that we are working at -- 
	int m_mipLevel;

	ProcRef m_procRef;

	boost::intrusive_ptr<IViewPoint> m_spViewPoint;
	
	/*! 
	 * keeps the priority order of the on_screen STiles. the SetSTile get an ordered vector of most important tile first. 
	 * the m_priorities is the same vector but we just need to keep IcosIndex to keep the order (memory issues)
	 */
	//! keeps the priority order of the on_screen STiles
	std::vector<PYXIcosIndex> m_priorities;
	bool STileFillManager::CompareJobs(const PYXPointer<WeakPointerSTile> & a, const PYXPointer<WeakPointerSTile> & b);
};

#endif
