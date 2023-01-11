#ifndef LIBRARY_FEATURE_H
#define LIBRARY_FEATURE_H
/******************************************************************************
library_feature.h

begin      : 11/12/2007 10:02:00 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "library_initializer.h"

// pyxlib includes
#include "pyxis/data/feature.h"

// standard includes

// local forward declarations
class ProcRef;

/*!
A feature that represents a single process entry in the library. The feature is
valid at the time of its construction, any subsequent changes to the library
are not reflected in the process.
*/
//! A process entry in the library
class LIBRARY_DECL LibraryFeature : public IFeature
{
// TODO should have PYXCOM class etc.?

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
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
		if (0 <= nFieldIndex && nFieldIndex < static_cast<int>(m_vecValue.size()))
		{
			return m_vecValue[nFieldIndex];
		}
		return PYXValue();
	}

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		assert(false && "Library Features are never writable.");
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
		assert(false && "Library Features are never writable");
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_vecValue;
	}

	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues)
	{
		assert(false && "Library Features are never writable");
	}

	virtual void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	)
	{
		assert(false && "Library Features are never writable");
	}

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return getGeometry();
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry();	

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return "";
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return "";
	}

// LibraryFeature
public:

	//! Constructor
	explicit STDMETHODCALLTYPE LibraryFeature(const ProcRef& procref, int nResolution);

	//! Destructor
	~LibraryFeature() {;}

	static PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE createFieldDefn();

private:

	//! Disable copy constructor
	LibraryFeature(const LibraryFeature&);

	//! Disable copy assignment
	void operator=(const LibraryFeature&);

private:

	//! Table definition.
	PYXPointer<PYXTableDefinition> m_spDefn;

	//! Storage for ID of the feature.
	std::string m_strID;
	
	//! The library data that populates the data fields
	std::vector<PYXValue> m_vecValue;

	//! The feature style.
	std::string m_featureStyle;

	//! Geometry is cached after the first request.
	PYXPointer<PYXGeometry> m_spGeometry;

	//! The desired resolution of the feature
	int m_nResolution;

	// friend classes
	friend class LibraryInitializer;
};

#endif
