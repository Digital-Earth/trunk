#ifndef PYXIS__PROCS__DEFAULT_FEATURE_H
#define PYXIS__PROCS__DEFAULT_FEATURE_H
/******************************************************************************
default_feature.h

begin		: 2007-05-29
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/data/feature.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/feature_iterator_linq.h"
#include "pyxis/pipe/process.h"

/*!
TODO would be nice to implement using COM object composition.
*/
//! Default feature implementation.
class PYXLIB_DECL DefaultFeature : public ProcessImpl<DefaultFeature>, public IFeature
{
	PYXCOM_DECLARE_CLASS();

public:

	DefaultFeature() :
		m_spDefn(PYXTableDefinition::create())
	{
	}

	explicit DefaultFeature(PYXPointer<PYXGeometry> spGeom) :
		m_spDefn(PYXTableDefinition::create()),
		m_spGeom(spGeom)
	{
	}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(DefaultFeature, IProcess);

public: //IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeature*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeature*>(this);
	}

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const
	{
		return std::map<std::string, std::string>();
	}

protected:

	virtual IProcess::eInitStatus initImpl()
	{
		// Set the ID.
		m_strID = "DefaultFeature" + procRefToStr(ProcRef(getProcID(), getProcVersion()));
		return IProcess::knInitialized;
	}

public: // IRecord

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const
	{
		return m_spDefn;
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition()
	{
		return m_spDefn;
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const
	{
		return m_vecValue[nFieldIndex];
	}

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		m_vecValue[nFieldIndex] = value;
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		PYXValue value;
		if (0 <= nFieldIndex)
		{
			value = getFieldValue(nFieldIndex);
		}
		return value;
	}

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName)
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		if (0 <= nFieldIndex)
		{
			setFieldValue(value, nFieldIndex);
		}
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_vecValue;
	}

	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues)
	{
		m_vecValue = vecValues;
	}

	virtual void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	)
	{
		PYXPointer<PYXTableDefinition> spDefn(getDefinition());
		int nFieldIndex = spDefn->getFieldIndex(strName);
		if (nFieldIndex < 0)
		{
			nFieldIndex = spDefn->addFieldDefinition(strName, nContext, nType, nCount);
			m_vecValue.resize(m_vecValue.size() + 1);
		}
		setFieldValue(value, nFieldIndex);
	}

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return true;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spGeom;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
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

public: // misc

	static void test();

private:

	//! Table definition.
	PYXPointer<PYXTableDefinition> m_spDefn;

	//! Values.
	std::vector<PYXValue> m_vecValue;

	//! Geometry.
	PYXPointer<PYXGeometry> m_spGeom;

	//! The styles associated with the feature.
	std::vector<FeatureStyle> m_vecStyles;

	//! The unique ID.
	std::string m_strID;
};

/*!
TODO would be nice to implement using COM object composition.
*/
//! Default heterogeneous feature collection implementation.
class PYXLIB_DECL DefaultFeatureCollection : ProcessImpl<DefaultFeatureCollection>, public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public:

	DefaultFeatureCollection() :
		m_spDefn(PYXTableDefinition::create())
	{
	}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(DefaultFeatureCollection, IProcess);

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
	virtual IProcess::eInitStatus initImpl()
	{
		// Set the ID.
		m_strID = "DefaultFeatureCollection" + procRefToStr(ProcRef(getProcID(), getProcVersion()));
		return knInitialized;
	}

public: // IRecord

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const
	{
		return m_spDefn;
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition()
	{
		return m_spDefn;
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const
	{
		return m_vecValue[nFieldIndex];
	}

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		m_vecValue[nFieldIndex] = value;
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		PYXValue value;
		if (0 <= nFieldIndex)
		{
			value = getFieldValue(nFieldIndex);
		}
		return value;
	}

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName)
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		if (0 <= nFieldIndex)
		{
			setFieldValue(value, nFieldIndex);
		}
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_vecValue;
	}

	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues)
	{
		m_vecValue = vecValues;
	}

	virtual void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	)
	{
		PYXPointer<PYXTableDefinition> spDefn(getDefinition());
		int nFieldIndex = spDefn->getFieldIndex(strName);
		if (nFieldIndex < 0)
		{
			nFieldIndex = spDefn->addFieldDefinition(strName, nContext, nType, nCount);
			m_vecValue.resize(m_vecValue.size() + 1);
		}
		setFieldValue(value, nFieldIndex);
	}

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return true;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spGeom;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
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

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const
	{
		return createDefaultFeatureIterator(m_vecFeature.begin(), m_vecFeature.end());
	}

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const
	{
		return PYXFeatureIteratorLinq(getIterator()).filter(geometry);		
	}

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const 
	{ 
		return PYXPointer<PYXTableDefinition>(); 
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() 
	{ 
		return PYXPointer<PYXTableDefinition>(); 
	} 
	
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const
	{
		return m_vecStyles;
	}

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const
	{
		return boost::intrusive_ptr<IFeature>();
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const 
	{
		return true;
	}

	IFEATURECOLLECTION_IMPL_HINTS();

public: // misc

	void addFeature(boost::intrusive_ptr<IFeature> spFeature)
	{
		assert(spFeature);
		m_vecFeature.push_back(spFeature);
	}

private:

	//! Table definition.
	PYXPointer<PYXTableDefinition> m_spDefn;

	//! Values.
	std::vector<PYXValue> m_vecValue;

	//! Geometry.
	PYXPointer<PYXGeometry> m_spGeom;

	//! Children.
	std::vector<boost::intrusive_ptr<IFeature> > m_vecFeature;

	//! The styles associated with the feature.
	std::vector<FeatureStyle> m_vecStyles;

	//! The unique ID.
	std::string m_strID;
};

#endif // guard
