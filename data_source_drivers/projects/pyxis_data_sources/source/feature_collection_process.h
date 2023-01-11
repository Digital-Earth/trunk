#ifndef FEATURE_COLLECTION_PROCESS_H
#define FEATURE_COLLECTION_PROCESS_H
/******************************************************************************
feature_collection_process.h

begin		: 2007-10-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "module_pyxis_coverages.h"

//pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/feature_style.h"
#include "pyxis/pipe/process.h"

/*!
A process which allows the user to create their own collection of features for various
purposes. This process accepts n individual features. Each feature that is added into the 
feature collection has a unique ID according its place in the feature collection. This 
feature collection process can then be plugged into a rasterizer and a colourizer, for 
rasterizing and visualizing the actual features.
*/
class MODULE_PYXIS_COVERAGES_DECL FeatureCollectionProcess : public ProcessImpl<FeatureCollectionProcess>, public IFeatureCollection
{	
	PYXCOM_DECLARE_CLASS();

public: //PYXCOM_IUnknown
	 
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: //IRecord

	IRECORD_IMPL();

public: //IFeature
	
	IFEATURE_IMPL();

public: //IProcess 

	IPROCESS_GETSPEC_IMPL();


	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCollection*>(this);
	}

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const
	{
		return std::map<std::string, std::string>();
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
		return m_vecFeatureStyles;
	}

	//! Get the feature with the specified ID.
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(
		const std::string& strFeatureID) const;

	//! Get the feature definition.
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const
	{
		return m_spTableDef;
	}

	//! Get the feature definition.
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition()
	{
		return m_spTableDef;
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const
	{
		return true;
	}

	IFEATURECOLLECTION_IMPL_HINTS();

public:
	
	//! Default Constructor
	FeatureCollectionProcess();

	//! Destructor
	~FeatureCollectionProcess(){;}

private:
	
	//! Vector of features belonging to this feature collection.
	std::vector<boost::intrusive_ptr<IFeature> > m_vecFeatures;

	std::vector<FeatureStyle> m_vecFeatureStyles;

	//! Table definition defining these features.
	PYXPointer<PYXTableDefinition> m_spTableDef;

};

/*!
A Feature iterator to iterate over all the features in the Feature collection process.
*/
class MODULE_PYXIS_COVERAGES_DECL FeatureCollectionIterator : public FeatureIterator
{
public:

	//! Dynamic Creator.
	static PYXPointer<FeatureCollectionIterator> create(const std::vector<boost::intrusive_ptr<IFeature> >& vect)
	{
		return PYXNEW(FeatureCollectionIterator, vect);
	}

	//! Default Constructor.
	FeatureCollectionIterator(const std::vector<boost::intrusive_ptr<IFeature> >& vect) :
		m_vect(vect),
		m_vecIt(m_vect.begin())
	{
	}

	//! Destructor
	virtual ~FeatureCollectionIterator(){;}

	//! Determine if we are done iterating over the features.
	virtual bool end() const
	{
		return m_vecIt == m_vect.end();
	}

	//! Move to the next feature.
	virtual void next()
	{
		++m_vecIt;
	}

	//! Get the current feature the iterator is on.
	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return *m_vecIt;
	}

private:
	
	//! A vector of features to iterate over.
	std::vector<boost::intrusive_ptr<IFeature> > m_vect;

	//! A const iterator for maintaining the position of the vector.
	std::vector<boost::intrusive_ptr<IFeature> >::const_iterator m_vecIt;

};

#endif //end guard
