#ifndef FEATURE_PROCESSING_PROCS__INTERSECTION_FILTER_H
#define FEATURE_PROCESSING_PROCS__INTERSECTION_FILTER_H

/******************************************************************************
intersection_filter.h

begin		: 2011-02-18
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"

#include <pyxis/grid/dodecahedral/resolution.hpp>

#include <boost/scoped_ptr.hpp>

class FeatureCollectionCache;
namespace Pyxis
{
	namespace Grid
	{
		namespace Dodecahedral
		{
			class Tree;
		}
		template < typename Model > class Geometry;
	}
}

/*!
Inputs: AOI feature, feature data set
Output: feature data set containing only the features intersecting the AOI
*/
//! Outputs only features that intersect an AOI.
class MODULE_FEATURE_PROCESSING_PROCS_DECL IntersectionFilter :
public ProcessImpl< IntersectionFilter >,
public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

	typedef Pyxis::Grid::Geometry< Pyxis::Grid::Dodecahedral::Tree > Geometry;

public: // PYXCOM_IUnknown
	 
	IUNKNOWN_QI_BEGIN
	IUNKNOWN_QI_CASE(IProcess)
	IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature
	
	virtual bool STDMETHODCALLTYPE isWritable() const;

	virtual std::string const & STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer< PYXGeometry > STDMETHODCALLTYPE getGeometry();

	virtual PYXPointer< PYXGeometry const > STDMETHODCALLTYPE getGeometry() const;

	virtual std::string STDMETHODCALLTYPE getStyle() const;

	virtual std::string STDMETHODCALLTYPE getStyle(std::string const & strStyleToGet) const;

public: // IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr< PYXCOM_IUnknown const > STDMETHODCALLTYPE getOutput() const
	{
		return static_cast< IFeatureCollection const * >(this);
	}

	virtual boost::intrusive_ptr< PYXCOM_IUnknown > STDMETHODCALLTYPE getOutput()
	{
		return static_cast< IFeatureCollection * >(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map< std::string, std::string > STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(std::map< std::string, std::string > const & mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IFeatureCollection

	//! Get an iterator to all the features in this collection.
	virtual PYXPointer< FeatureIterator > STDMETHODCALLTYPE getIterator() const;
	
	// TODO: Remove PYXGeometry from feature; deprecated.
	//! Get an iterator to all the features in this collection that intersect this geometry.
	virtual PYXPointer< FeatureIterator > STDMETHODCALLTYPE getIterator(PYXGeometry const & geometry) const;

	//! Get styles that determine how to visualize features in this collection.
	virtual std::vector< FeatureStyle > STDMETHODCALLTYPE getFeatureStyles() const;

	//! Get the feature with the specified ID.
	virtual boost::intrusive_ptr< IFeature > STDMETHODCALLTYPE getFeature(std::string const & strFeatureID) const;

	//! Get the feature definition.
	virtual PYXPointer< PYXTableDefinition > STDMETHODCALLTYPE getFeatureDefinition();

	//! Get the feature definition.
	virtual PYXPointer< PYXTableDefinition > STDMETHODCALLTYPE getFeatureDefinition() const;

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

public:

	//! Default Constructor
	//! This class may ONLY be safely created on the heap.
	IntersectionFilter();

	//! Destructor
	~IntersectionFilter();

	//! Test
	static void test();

protected: // IFeature

	mutable std::string m_strID;

private:

	//! The input feature collection.
	boost::intrusive_ptr< IFeatureCollection > m_spFeatureCollection;

	//! A feature collection cache.
	mutable PYXPointer< FeatureCollectionCache > m_spFeatureCollectionCache;

	//! The intersecting region.
	boost::intrusive_ptr< Geometry > m_spAOIGeometry;

	//! The default resolution.
	Pyxis::Grid::Dodecahedral::Resolution const m_defaultResolution;

	//! The resolution of intersection.
	Pyxis::Grid::Dodecahedral::Resolution m_resolution;
};

#endif
