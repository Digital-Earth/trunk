#ifndef FEATURE_PROCESSING_PROCS__FEATURE_COLLECTION_CALCULATOR_H
#define FEATURE_PROCESSING_PROCS__FEATURE_COLLECTION_CALCULATOR_H
/******************************************************************************
feature_collection_calculator.h

begin      : 07/04/2008 8:42:51 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/feature_calculator.h"
#include "pyxis/utility/object.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

/*!
Perform a calculation on each of the features in the collection, then append the 
calculation result to each of the features.
*/
//! Appends a calculation result to each of the features within the input calculation.
class MODULE_FEATURE_PROCESSING_PROCS_DECL FeatureCollectionCalculator : 
	public ProcessImpl<FeatureCollectionCalculator>, public IFeatureCollection
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

	PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const
	{
		return m_spFeaturesInput->getDefinition();
	}

	PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition()
	{
		return m_spFeaturesInput->getDefinition()->clone();
	}

	PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const
	{
		return m_spFeaturesInput->getFieldValue(nFieldIndex);
	}

	void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		PYXTHROW(PYXException, "Feature collection calculators are not writable, can't setFieldValue");
	}

	PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const
	{
		return m_spFeaturesInput->getFieldValueByName(strName);
	}

	void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName)
	{
		PYXTHROW(PYXException, "Feature collection calculators are not writable, can't setValueByName");
	}
	
	std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_spFeaturesInput->getFieldValues();
	}

	void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues)
	{
		PYXTHROW(PYXException, "Feature collection calculators are not writable, can't setFieldValues");
	}

	void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	)
	{
		PYXTHROW(PYXException, "Feature collection calculators are not writable, can't add a field.");
	}

protected: // IFeature

	mutable std::string m_strID;

public: // IFeature
	
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_spFeaturesInput->getGeometry()->clone();
	}

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

private:

	// The name of the field to place the calculation in.
	std::string m_strCalculatedFieldName;

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

private:

	//! A mutex to protect against multi threaded access
	mutable boost::recursive_mutex m_mutex;

public:

	/*!
	A Feature iterator to iterate over all the features in the Feature collection process,
	skipping over those that don't fulfill the criteria.
	*/
	class CalculatedFeatureIterator : public FeatureIterator
	{
	public:

		//! Dynamic Creator.
		static PYXPointer<CalculatedFeatureIterator> create(
			boost::intrusive_ptr<const IFeatureCollection> spFeatureCollection,
			PYXPointer<FeatureIterator> spIterator
			)
		{
			return PYXNEW(CalculatedFeatureIterator, spFeatureCollection, spIterator);
		}

		//! Default Constructor.
		CalculatedFeatureIterator(
			boost::intrusive_ptr<const IFeatureCollection> spFeatureCollection,
			PYXPointer<FeatureIterator> spIterator	) :
				m_spFC(spFeatureCollection),
				m_spIterator(spIterator),
				m_pFCCalculator(0)
		{
			// get a pointer to the actual fc calculator. Not a smart pointer because that is 
			// an ambiguous reference with PYXCOM_IUnknown
			m_pFCCalculator = dynamic_cast<const FeatureCollectionCalculator*>(m_spFC.get());
			assert(m_pFCCalculator != 0 && "Must always be true");
		}

		//! Destructor
		virtual ~CalculatedFeatureIterator()
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
			return m_pFCCalculator->createOutputFeature(m_spIterator->getFeature());
		}

		//! Move to the next feature.
		virtual void next()
		{
			if (!m_spIterator->end())
			{
				m_spIterator->next();
			}
		}

	private:

		//! The feature iterator for the input process.
		const PYXPointer<FeatureIterator> m_spIterator;

		//! The input feature collection.
		boost::intrusive_ptr<const IFeatureCollection> m_spFC;
		const FeatureCollectionCalculator* m_pFCCalculator;
	};

	//! Default Constructor
	FeatureCollectionCalculator();

	//! Destructor
	~FeatureCollectionCalculator();

private:

	//! The input feature collection.
	boost::intrusive_ptr<const IFeatureCollection> m_spFeaturesInput;

	//! The feature definition of the post calculation output features
	PYXPointer<PYXTableDefinition> m_spFeatureDefn;

	//! The calculation to perform on each feature.
	boost::intrusive_ptr<IFeatureCalculator> m_spCalculator;

	//! Create an output feature from an input feature, performing the appropriate calculation
	boost::intrusive_ptr<IFeature> createOutputFeature(boost::intrusive_ptr<IFeature> spInputFeature) const;

	friend class CalculatedFeatureIterator;
};

//! This feature delegates to the input feature with the exception of the calculated value
class MODULE_FEATURE_PROCESSING_PROCS_DECL CalculatedFeature : public IFeature
{
public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

	//! Default Constructor.
	CalculatedFeature(
		PYXPointer<const PYXTableDefinition> spDefn,
		boost::intrusive_ptr<const IFeature> spInputFeature,
		const std::string& strCalculatedFieldName, 
		PYXValue calculatedValue	) :
			m_spDefn(spDefn),
			m_spInputFeature(spInputFeature),
			m_strCalculatedFieldName(strCalculatedFieldName),
			m_calculatedValue(calculatedValue)
	{
	}

// IFeature

	bool STDMETHODCALLTYPE isWritable() const 
	{
		return false;
	}

	const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_spInputFeature->getID();
	}

	PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_spInputFeature->getGeometry()->clone();
	}

	PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spInputFeature->getGeometry();
	}

	std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_spInputFeature->getStyle();
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return m_spInputFeature->getStyle(strStyleToGet);
	}

// IRecord

	PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const
	{
		return m_spDefn;
	}

	PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition()
	{
		return m_spDefn->clone();
	}

	PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const
	{
		// the last item is the calculated value
		if (nFieldIndex == (m_spDefn->getFieldCount() - 1))
		{
			return m_calculatedValue;
		}
		return m_spInputFeature->getFieldValue(nFieldIndex);
	}

	void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		PYXTHROW(PYXException, "Calculated features are not writable, can't setFieldValue");
	}

	PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const
	{
		if (strName == m_strCalculatedFieldName)
		{
			return m_calculatedValue;
		}
		return m_spInputFeature->getFieldValueByName(strName);
	}

	void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName)
	{
		PYXTHROW(PYXException, "Calculated features are not writable, can't setValueByName");
	}
	
	std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		std::vector<PYXValue> returnValues = m_spInputFeature->getFieldValues();
		returnValues.push_back(m_calculatedValue);
		return returnValues;
	}

	void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues)
	{
		PYXTHROW(PYXException, "Calculated features are not writable, can't setFieldValues");
	}

	void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	)
	{
		PYXTHROW(PYXException, "Calculated features are not writable, can't add a feature.");
	}

private:

	PYXPointer<const PYXTableDefinition> m_spDefn;
	boost::intrusive_ptr<const IFeature> m_spInputFeature;
	std::string m_strCalculatedFieldName;
	PYXValue m_calculatedValue;
};

#endif
