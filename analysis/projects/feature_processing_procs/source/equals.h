#ifndef FEATURE_PROCESSING_PROCS__EQUALS_H
#define FEATURE_PROCESSING_PROCS__EQUALS_H

/******************************************************************************
equals.h

begin		: 2008-01-23
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"

class FeatureCollectionProcess;

/*!
Inputs: AOI feature, feature data set
Output: feature data set containing only the features equal to the AOI
*/
//! Filters out features not equal to an AOI.
class MODULE_FEATURE_PROCESSING_PROCS_DECL Equals : public ProcessImpl<Equals>, public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown
	 
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord

	IRECORD_IMPL();

protected: // IFeature

	mutable std::string m_strID;

public: // IFeature
	
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return m_spFeaturesInput->isWritable();
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_spAOIGeometry;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spAOIGeometry;
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_spFeaturesInput->getStyle();
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return m_spFeaturesInput->getStyle(strStyleToGet);
	}

public: // IProcess 

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

public: // IFeatureCollection

	//! Get an iterator to all the features in this collection.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;
	
	//! Get an iterator to all the features in this collection that intersect this geometry.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;
	
	//! Get styles that determine how to visualize features in this collection.
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	//! Get the feature with the specified ID.
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	//! Get the feature definition.
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	//! Get the feature definition.
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

public:

	/*!
	A Feature iterator to iterate over all the features in the Feature collection process,
	skipping over those that don't fulfill the criteria.
	*/
	class FilteredFeatureIterator : public FeatureIterator
	{
	public:

		//! Dynamic Creator.
		static PYXPointer<FilteredFeatureIterator> create(
			const PYXPointer<FeatureIterator> spIterator,
			const PYXPointer<const PYXGeometry> spAOIGeometry	)
		{
			return PYXNEW(FilteredFeatureIterator, spIterator, spAOIGeometry);
		}

		//! Default Constructor.
		FilteredFeatureIterator(
			const PYXPointer<FeatureIterator> spIterator,
			const PYXPointer<const PYXGeometry> spAOIGeometry	) :
			m_spIterator(spIterator),
			m_spAOIGeometry(spAOIGeometry)
		{
			findMatch();
		}

		//! Destructor
		virtual ~FilteredFeatureIterator()
		{
		}

		//! Determine if we are done iterating over the features.
		virtual bool end() const
		{
			return m_spIterator->end();
		}

		//! Get the current feature the iterator is on.
		virtual boost::intrusive_ptr<IFeature> getFeature() const
		{
			return m_spIterator->getFeature();
		}

		//! Move to the next feature.
		virtual void next()
		{
			if (!m_spIterator->end())
			{
				m_spIterator->next();
				findMatch();
			}
		}

	private:

		void findMatch()
		{
			while (!m_spIterator->end() && !match(*m_spAOIGeometry, m_spIterator->getFeature()))
			{
				m_spIterator->next();
			}
		}

	private:

		//! The feature iterator.
		const PYXPointer<FeatureIterator> m_spIterator;

		//! The geometry to compare to.
		const PYXPointer<const PYXGeometry> m_spAOIGeometry;
	};

	//! Default Constructor
	Equals();

	//! Destructor
	~Equals();

	//! Test
	static void test();

private:

	//! Returns true if the feature fulfills the criteria.
	static bool match(const PYXGeometry& aoiGeometry, boost::intrusive_ptr<IFeature> spFeature)
	{
		assert(spFeature);
		PYXPointer<PYXGeometry> spGeometryToCheck = spFeature->getGeometry();
		return spGeometryToCheck && aoiGeometry == *spGeometryToCheck;
	}

private:

	//! The geometry to compare to.
	PYXPointer<PYXGeometry> m_spAOIGeometry;

	//! The input feature collection.
	boost::intrusive_ptr<IFeatureCollection> m_spFeaturesInput;
};

#endif
