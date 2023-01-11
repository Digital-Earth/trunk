#ifndef IFILTER_H
#define IFILTER_H
/******************************************************************************
IFilter.h

begin		: 2006-02-27
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "pyxis/data/coverage.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/utility/pyxcom.h"

// boost includes
#include <boost/shared_ptr.hpp>

// standard includes

// local forward declarations

/*!
IFilter is the interface for all classes that perform filtering on a PYXIS
coverage to produce another coverage.
*/
//! Filters a coverage to produce a new coverage.
struct PYXLIB_DECL IFilter : public IUnknown
{
		PYXCOM_DECLARE_INTERFACE();
public:
	// Constants
	static const std::string kstrScope;

	//! Return the class name of this observer class.
	virtual std::string STDMETHODCALLTYPE getObserverDescription() const = 0;

	//! Return the name of the notification class.
	virtual std::string STDMETHODCALLTYPE getNotifierDescription() const = 0;

	//! Check if the input coverage has a spatial reference system.
	virtual bool STDMETHODCALLTYPE hasSpatialReferenceSystem() const =0 ;

	//! Set the spatial reference system for the input coverage.
	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(PYXSpatialReferenceSystem::SPtr spSRS) = 0;

	//! Set the PYXIS resolution for the input coverage.
	virtual void  STDMETHODCALLTYPE setResolution(int nResolution) = 0;

	//! Set the input coverage (ownership shared with caller)
	virtual void STDMETHODCALLTYPE setInput(boost::shared_ptr<PYXCoverage> spInput) = 0;

	//! Get the input coverage
	boost::shared_ptr<PYXCoverage> STDMETHODCALLTYPE getInput() const {return m_spInput;} = 0

	//!	Get the name of the filter.
	virtual std::string STDMETHODCALLTYPE getName() const = 0;

	//! Get the geometry of the coverage.
	virtual boost::shared_ptr<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const = 0; 

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
										int nFieldIndex = 0) const = 0;

	//! Get coverage values for an entire tile.
	virtual boost::shared_ptr<PYXValueTile>
		STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const = 0;
};

#endif	// guard
