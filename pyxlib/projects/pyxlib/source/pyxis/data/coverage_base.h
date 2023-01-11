#ifndef PYXIS__DATA__COVERAGE_BASE_H
#define PYXIS__DATA__COVERAGE_BASE_H
/******************************************************************************
coverage_base.h

begin		: 2007-03-27
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/data/coverage.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

/*!
*/
//! A base class for coverages.
class PYXLIB_DECL CoverageBase : public ICoverage
{
public:

	//! Constructor
	CoverageBase() :
		m_spDefn(PYXTableDefinition::create()),
		m_spCovDefn(PYXTableDefinition::create())
	{
	}

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	//! Determine if the coverage can be written to or not. The default is to be not writable.
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	//! The default implementation in this base class returns an empty string.
	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	//! Get the geometry of the coverage.
	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		if (!m_spGeom)
		{
			createGeometry();
		}
		return m_spGeom;
	}

	//! Get the geometry for the coverage.
	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		if (!m_spGeom)
		{
			createGeometry();
		}
		return m_spGeom;
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return "";
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return "";
	}

protected:

	//! The geometry for the coverage.
	mutable PYXPointer<PYXGeometry> m_spGeom;

	//! The id of the feature
	std::string m_strID;

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL();

public: // ICoverage

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		return m_spCovDefn;
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return m_spCovDefn;
	}

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(		const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex = 0	) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(const PYXIcosIndex& index,
													   int nRes,
													   int nFieldIndex = 0	) const ;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

	virtual PYXCost STDMETHODCALLTYPE getTileCost(const PYXTile& tile) const;

	virtual void STDMETHODCALLTYPE setCoverageValue(	const PYXValue& value,	
														const PYXIcosIndex& index,
														int nFieldIndex = 0	);

	virtual void STDMETHODCALLTYPE setCoverageTile(PYXPointer<PYXValueTile> spValueTile);

protected:

	//! Create the geometry for the coverage.
	virtual void createGeometry() const = 0;

protected:

	//! The definiton of the data fields for the coverage.
	PYXPointer<PYXTableDefinition> m_spCovDefn;

	//! The mutex. Can be re-used by derived classes.
	mutable boost::recursive_mutex m_coverageMutex;	
};

#endif
