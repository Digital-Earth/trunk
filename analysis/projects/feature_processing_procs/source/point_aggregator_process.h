#ifndef POINT_AGGREGATOR_PROCESS_H
#define POINT_AGGREGATOR_PROCESS_H
/******************************************************************************
point_aggregator_process.h

begin		: 2008-01-31
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_feature_processing_procs.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/feature_style.h"
#include "pyxis/pipe/process.h"
#include "pyxis/geometry/multi_cell.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

/*!
An Aggregated point feature is a feature collection that represents all the features cotained in a larger cell.
Containment of features in a larger cell may either be Pyxis containment or spatial containmetn based on 
the aggregation algorithm applied to points being aggregated.
*/
class AggregatedPointFeature : public IFeatureCollection 
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Default Cosntructor.
	AggregatedPointFeature();

	//! Destructor.
	~AggregatedPointFeature(){;}

public:

	 IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

	IUNKNOWN_DEFAULT_CAST(AggregatedPointFeature, IFeature);

public: //IRecord

	IRECORD_IMPL();

public: //IFeature
	
	virtual bool STDMETHODCALLTYPE isWritable() const;

	virtual const std::string& STDMETHODCALLTYPE getID() const;

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry();

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const;

	virtual std::string STDMETHODCALLTYPE getStyle() const;

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const;

public: //IFeatureCollection

	//! Get an iterator to all the features in this collection.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;
	
	//! Get an iterator to all the features in this collection that intersect this geometry.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;
	
	//! Get styles that determine how to visualize features in this collection.
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE 
		getFeatureStyles() const
	{
		return std::vector<FeatureStyle>();
	}

	//! Get the feature with the specified ID.
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(
		const std::string& strFeatureID) const;

	//! Get the feature definition.
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const
	{
		return m_spDefn;
	}

	//! Get the feature definition.
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition()
	{
		return m_spDefn;
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const
	{
		return false;
	}

	IFEATURECOLLECTION_IMPL_HINTS();
	
	// TODO: Remove PYXGeometry from feature; deprecated.
	//! Set the geometry of this aggregated feature.
	virtual void setGeometry(PYXPointer<PYXGeometry> spGeom);

	//! Add a feature into the aggregated feature.
	virtual void addFeature(boost::intrusive_ptr<IFeature> spFeature);

private:

	//! Calculates the meta data for feature picking.
	void calcMetaData();

	PYXPointer<PYXGeometry> m_spGeom;

	std::vector<boost::intrusive_ptr<IFeature> > m_vecFeatures;

	mutable std::string m_strID;
};


/*!
Aggregates a input feature collection to a set aggregate resolution. 
*/
class MODULE_FEATURE_PROCESSING_PROCS_DECL PointAggregatorProcess : public ProcessImpl<PointAggregatorProcess>,  
																  public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public: //PYXCOM_IUnknown
	
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_END
	
	IUNKNOWN_RC_IMPL_FINALIZE();
	
	IUNKNOWN_DEFAULT_CAST(PointAggregatorProcess, IProcess);

public: //IRecord

	IRECORD_IMPL();

public: //IFeature
	
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return true;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const;

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry();

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const;

	virtual std::string STDMETHODCALLTYPE getStyle() const;
	
	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const;

public: //IProcess 

	IPROCESS_GETSPEC_IMPL();

	//! Get the output type of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCollection*>(this);
	}

	//! Get the output type of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCollection*>(this);
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IFeatureCollection

	//! Get an iterator to all the features in this collection.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;
	
	//! Get an iterator to all the features in this collection that intersect this geometry.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;
	
	//! Get styles that determine how to visualize features in this collection.
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE 
		getFeatureStyles() const
	{
		return std::vector<FeatureStyle>();
	}

	//! Get the feature with the specified ID.
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(
		const std::string& strFeatureID) const;

	//! Get the feature definition.
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const
	{
		return m_spDefn;
	}

	//! Get the feature definition.
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition()
	{
		return m_spDefn;
	}

	//! Determine if we can rasterize this feature collection or not.
	virtual bool STDMETHODCALLTYPE canRasterize() const
	{
		return false;
	}

	IFEATURECOLLECTION_IMPL_HINTS();

	//! Return the map of name, value attribute pairs.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& attrMap);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

public: //PointAggregatorProcess
	
	static void test();
	
	//! Default Constructor
	PointAggregatorProcess();

	//! Destructor
	~PointAggregatorProcess(){;}

private:

	//! Aggregate all the features from resolution n to resolution n - x.
	std::vector<boost::intrusive_ptr<AggregatedPointFeature> > aggregate(int nAggregateResolution) const;

	void calcGeometry() const;

	//! Utility method to convert from a vector of boost::intrusive_ptr<AggregatedPointFeature> to boost::intrusive_ptr<IFeature>
	std::vector<boost::intrusive_ptr<IFeature> > convert(std::vector<boost::intrusive_ptr<AggregatedPointFeature> > vecFeatures) const;

private: 

	//! Member to determine if we have aggregated or not.
	mutable bool m_bIsAggregated;

	//! The resolution to aggregate to.
	mutable int m_nAggregatedResolution;

	//! A Vector of boost intrusive_ptr to AggregatedPointFeature
	mutable	std::vector<boost::intrusive_ptr<AggregatedPointFeature> > m_vecAggregatedFeatures;

	//! The input feature collection.
	mutable boost::intrusive_ptr<IFeatureCollection> m_spInputFeatureCollection;

	//! The string ID of this feature.
	std::string m_strId;

	//! The geometry of this feature collection
	mutable PYXPointer<PYXMultiCell> m_spGeom;
	
	//! The mutex.
	mutable boost::recursive_mutex m_mutex;

};



#endif