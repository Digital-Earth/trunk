#ifndef FEATURE_PROCESSING_PROCS__ATTRIBUTE_QUERY_H
#define FEATURE_PROCESSING_PROCS__ATTRIBUTE_QUERY_H

/******************************************************************************
attribute_query.h

begin		: 2008-01-31
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

/*!
Inputs: feature data set
Output: feature data set containing only the features that satisfy the query
*/
//! Filters out features that do not satisfy the query.
class MODULE_FEATURE_PROCESSING_PROCS_DECL AttributeQuery : public ProcessImpl<AttributeQuery>, public IFeatureCollection
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

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry();

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spFeaturesInput->getGeometry();
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

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;
	
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;
	
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

private:

	//! The aggregate geometry for the returned query results.
	PYXPointer<PYXGeometry> m_spGeom;

	//! indication to use the input geometry or the aggregate geometry (true == input)
	bool m_bInputGeometry;

	//! A mutex to protect against multi threaded access
	mutable boost::recursive_mutex m_mutex;

public:

	/*!
	Base class to implement specialied feature queries objects
	*/
	class FeatureQuery : public PYXObject
	{
	public:
		//! Returns true if the feature fulfills the query.
		virtual bool match(const boost::intrusive_ptr<IFeature> & spFeature) const = 0;
	};

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
			const PYXPointer<FeatureQuery> query )
		{
			return PYXNEW(FilteredFeatureIterator, spIterator, query);
		}

		//! Default Constructor.
		FilteredFeatureIterator(
			const PYXPointer<FeatureIterator> spIterator,
			const PYXPointer<FeatureQuery> query ) :
			m_spIterator(spIterator),
			m_query(query)
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
			while (!m_spIterator->end() && !m_query->match(m_spIterator->getFeature()))
			{
				m_spIterator->next();
			}
		}

	private:

		//! The feature iterator.
		const PYXPointer<FeatureIterator> m_spIterator;

		//! The feature iterator.
		const PYXPointer<FeatureQuery> m_query;

		//! The query.
		const std::string m_strQuery;
	};

	//! Default Constructor
	AttributeQuery();

	//! Destructor
	~AttributeQuery();

	//! Test
	static void test();

private:

	//! create a query object from the query string
	PYXPointer<FeatureQuery> createQuery(const std::string& strQuery);

private:

	//! The query.
	std::string m_strQuery;
	
	//! The input feature collection.
	boost::intrusive_ptr<IFeatureCollection> m_spFeaturesInput;

	PYXPointer<FeatureQuery> m_query;
};

#endif
