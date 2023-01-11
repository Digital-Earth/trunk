#ifndef EXCEL__EXCEL_FEATURE_H
#define EXCEL__EXCEL_FEATURE_H

/******************************************************************************
excel_feature.h

begin      : 15/11/2007 9:56:58 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "excel.h"

// pyxlib includes
#include "pyxis/data/feature.h"

// standard includes

// local forward declarations
class CoordLatLon;
class PYXVectorPointRegion;

/*!
A Point feature produced by the Excel process
*/
//! A Point feature produced by the Excel process
class ExcelFeature :
public IFeature
{
// TODO should have PYXCOM class etc.?

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

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
		assert(m_vecValue.size() <= boost::integer_traits< int >::const_max);
		if (0 <= nFieldIndex && nFieldIndex < static_cast<int>(m_vecValue.size()))
		{
			return m_vecValue[nFieldIndex];
		}
		return PYXValue();
	}

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		assert(false && "Excel Features are never writable.");
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(std::string const & strName) const
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		PYXValue value;
		if (0 <= nFieldIndex)
		{
			value = getFieldValue(nFieldIndex);
		}
		return value;
	}

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, std::string const & strName)
	{
		assert(false && "Excel Features are never writable");
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_vecValue;
	}

	virtual void STDMETHODCALLTYPE setFieldValues(std::vector<PYXValue> const & vecValues)
	{
		assert(false && "Excel Features are never writable");
	}

	virtual void STDMETHODCALLTYPE addField(
		const std::string& strName,
		PYXFieldDefinition::eContextType nContext,
		PYXValue::eType nType,
		int nCount = 1,
		PYXValue value = PYXValue())
	{
		assert(false && "Excel Features are never writable");
	}

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual std::string const & STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer< IRegion const > STDMETHODCALLTYPE getRegion() const;

	virtual PYXPointer< PYXGeometry const > STDMETHODCALLTYPE getGeometry() const
	{
		return m_spGeometry;
	}

	virtual PYXPointer< PYXGeometry > STDMETHODCALLTYPE getGeometry()
	{
		return m_spGeometry;
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return "";
	}

	virtual std::string STDMETHODCALLTYPE getStyle(std::string const & strStyleToGet) const
	{
		return "";
	}
	
public:

	//! Constructor
	ExcelFeature(
		PYXPointer< PYXTableDefinition > spDefn,
		PYXPointer< PYXVectorPointRegion const > spRegion,
		PYXPointer< PYXGeometry > spGeometry,
		std::vector< PYXValue > const & values,
		size_t nID);

private:

	//! Disable copy constructor
	ExcelFeature(ExcelFeature const &);

	//! Disable copy assignment
	void operator =(ExcelFeature const &);

private:

	//! Table definition.
	PYXPointer< PYXTableDefinition > m_spDefn;

	//! The ID of the feature in numeric form.
	size_t m_nID;

	//! The ID of the feature in string form.
	std::string m_strID;

	//! Region.
	// TODO: Change this over to VectorPointRegion.
	PYXPointer< PYXVectorPointRegion const > m_spRegion;

	//! Geometry.
	PYXPointer< PYXGeometry > m_spGeometry;

	//! The vector of values for the feature.
	std::vector< PYXValue > m_vecValue;

	//! The feature style.
	std::string m_featureStyle;
};

#endif
