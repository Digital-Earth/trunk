#ifndef PYXIS__DATA__RECORD_H
#define PYXIS__DATA__RECORD_H
/******************************************************************************
record.h

begin		: 2007-02-28
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/object.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/value.h"

// standard includes
#include <string>

/*!
PYXFieldDefinition describes a field in a feature, coverage or data source.
*/
//! Describes a field in a data source, coverage or feature.
class PYXLIB_DECL PYXFieldDefinition
{
public:

	//! Test method
	static void test();

	//! The context of the data.
	enum eContextType
	{
		knContextNone = 0x0000,			//!< No context is available for the field.
		knContextRGB = 0x1001,			//!< Array of 3 values interpreted as colour.
		knContextGreyScale = 0x2001,	//!< Single value interpreted as greyscale.
		knContextElevation = 0x4001,	//!< Single value interpreted as elevation.
		knContextNormal = 0x5001,       //!< Array of 3 values interpated as a normal vector
		knContextDistance = 0x6001,         //!< Single value interpreted as distance.
		knContextArea = 0x7001,         //!< Single value interpreted as area.
		knContextCLUT = 0x8001,			//!< Value is an offset into a colour lookup table.
		knContextClass = 0x1601			//!< Value is the output from a classification algorithm.
	};

	//! Default constructor.
	PYXFieldDefinition() :
		m_strName(),
		m_nContext(knContextNone),
		m_nType(PYXValue::knNull),
		m_nCount(0)	{}

	/*!
	Constructor.

	\param	strName		The name of the field.
	\param	nContext	How the field is to be interpreted.
	\param	nType		The expected data type of the field.
	\param	nCount		The number of values for a single field (i.e. 3 for RGB)
	*/
	//! Constructor initializes member variables
	PYXFieldDefinition(	const std::string strName,
						eContextType nContext,
						PYXValue::eType nType,
						int nCount = 1	) :
		m_strName(strName),
		m_nContext(nContext),
		m_nType(nType),
		m_nCount(nCount) {}

	//! Copy constructor
	PYXFieldDefinition(const PYXFieldDefinition& defn);

	//! Copy assignment
	PYXFieldDefinition operator=(const PYXFieldDefinition& defn);

	//! Destructor
	virtual ~PYXFieldDefinition() {}

	//! Get the name of the field
	inline const std::string& getName() const {return m_strName;}

	//! Get the context of the field
	inline eContextType getContext() const {return m_nContext;}

	//! Get the data type of the field
	inline PYXValue::eType getType() const {return m_nType;}

	//! Is the field numeric
	inline bool isNumeric() const {return PYXValue::isNumeric(m_nType);}

	//! Get the expected number of values for the field (i.e. 3 for RGB)
	inline int getCount() const {return m_nCount;}

	//! get a PYXValue that is uninitialized, but of the correct type to hold this field's data.
	inline PYXValue getTypeCompatibleValue() const {return PYXValue::create(getType(), 0, getCount(), 0);}

private:

	//! The field name
	std::string m_strName;

	//! The field context
	eContextType m_nContext;

	//! The field data type
	PYXValue::eType m_nType;

	//! The expected number of values
	int m_nCount;

private:

	friend PYXLIB_DECL bool operator ==(const PYXFieldDefinition& lhs, const PYXFieldDefinition& rhs);
	friend PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXFieldDefinition& defn);
	friend PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXFieldDefinition& defn);

	friend class PYXTableDefinition;
};

//! The equality operator.
PYXLIB_DECL bool operator ==(const PYXFieldDefinition& lhs, const PYXFieldDefinition& rhs);

//! The inequality operator.
PYXLIB_DECL bool operator !=(const PYXFieldDefinition& lhs, const PYXFieldDefinition& rhs);

//! Stream operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXFieldDefinition& defn);

//! Stream operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXFieldDefinition& defn);

////////////////////////////////////////////////////////////////////////////////

/*!
PYXTableDefinition describes the fields in a feature, coverage or data source.
*/
//! Describes the fields in a data source, coverage or feature.
class PYXLIB_DECL PYXTableDefinition : public PYXObject
{
public:

	//! Typedef for vector of field definitions
	typedef std::vector<PYXFieldDefinition> FieldDefinitionVector;

	//! Test method
	static void test();

	//! Create method
	static PYXPointer<PYXTableDefinition> create()
	{
		return PYXNEW(PYXTableDefinition);
	}

	//! Copy create method
	static PYXPointer<PYXTableDefinition> create(const PYXTableDefinition& rhs)
	{
		return PYXNEW(PYXTableDefinition, rhs);
	}

	//! Clone method
	virtual PYXPointer<PYXTableDefinition> clone() const
	{
		return create(*this);
	}

	//! Clear method
	virtual void clear() { m_vecFieldDefns.clear(); }

	//! Add a field definition.
	int addFieldDefinition(	const std::string& strName,
							PYXFieldDefinition::eContextType nContext,
							PYXValue::eType nType,
							int nCount = 1	);

	//! Add a field definition.
	int addFieldDefinition(const PYXFieldDefinition& fieldDefn);

	/*!
	This method was added to that in Blender, Colourizer and FNN, if in the initProc method, when
	we are setting up our coverage, if we don't have any valid coverages and m_spCovDefn is set
	to a PYXTableDefinition that has been initialized only, then we take this "initialized only"
	PYXTableDefinition and add a null-field. This was part of a "policy" decision made when the PIPE
	was being reworked in July 2007. This rework addressed many issues, one of the key issues being
	that getCoverageDefinition() would not return a valid Coverage when invoked in FNN::getFieldTile.
	*/
	void addNullField() {addFieldDefinition("rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);}

	/*!
	Get the number of fields in this table.

	\return	The number of fields in this table.
	*/
	//! Get the number of fields in this table.
	int getFieldCount() const {return static_cast<int>(m_vecFieldDefns.size());}

	//! Get a field definition by index.
	const PYXFieldDefinition& getFieldDefinition(int nFieldIndex) const;

	//! Get a field index by name.
	int getFieldIndex(const std::string& strName) const;

	//! Get a vector containing the field names
	std::vector<std::string> getFieldNames() const;

protected:

	//! Constructor.
	PYXTableDefinition() {}

	//! Copy constructor
	PYXTableDefinition(const PYXTableDefinition& defn);

	//! Copy assignment
	PYXTableDefinition operator=(const PYXTableDefinition& defn);

	//! Destructor
	virtual ~PYXTableDefinition() {}

private:

	//! Vector of field definitions
	mutable FieldDefinitionVector m_vecFieldDefns;

private:

	friend PYXLIB_DECL bool operator ==(const PYXTableDefinition& lhs, const PYXTableDefinition& rhs);
	friend PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXTableDefinition& defn);
	friend PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXTableDefinition& defn);
};

//!The equality operator.
PYXLIB_DECL bool operator ==(const PYXTableDefinition& lhs, const PYXTableDefinition& rhs);

//! The inequality operator.
PYXLIB_DECL bool operator !=(const PYXTableDefinition& lhs, const PYXTableDefinition& rhs);

//! Stream operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXTableDefinition& defn);

//! Stream operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXTableDefinition& defn);

////////////////////////////////////////////////////////////////////////////////

/*!
A record has fields.
*/
//! A record.
struct PYXLIB_DECL IRecord : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	/*!
	Get the definition that describes the fields in this data provider.

	\return	The definition.
	*/
	//! Get the table definition
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const = 0;

	/*!
	Get the definition that describes the fields in this data provider.

	\return	The definition.
	*/
	//! Get the table definition
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition() = 0;

	/*!
	Get the value of a field by index. Use getDefinition->getFieldIndex() to
	get the appropriate field index.

	\param	nFieldIndex	The field index.

	\return	The value.
	*/
	//! Get a field value by index.
	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const = 0;

	/*!
	Set the value of a field by index. Use getDefinition()->getFieldIndex() to
	get the appropriate field index. Not implemented in read-only data sources.

	\param	nFieldIndex	The field index.
	\param	value	The value.
	*/
	//! Set a field value by index.
	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex) = 0;

	//! Get a field value by name.
	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const = 0;

	//! Set a field value by name.
	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName) = 0;

	//! Get a vector of all field values.
	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const = 0;

	//! Get a vector of all field values.
	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues) = 0;

	//! Add a field description and value to the data provider.
	virtual void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	) = 0;
};

//! A class of helper functions for working with records
class PYXLIB_DECL RecordTools
{
public:

	//! Return a record in xml form.
	static std::string STDMETHODCALLTYPE getFieldsAsXml(const IRecord& record);
};

// Also need to do set m_spDefn to PYXTableDefinition::create()
#define IRECORD_IMPL() \
private: \
	std::vector<PYXValue> m_vecValues; \
protected: \
	mutable PYXPointer<PYXTableDefinition> m_spDefn; \
public: \
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const \
	{ \
		return m_spDefn; \
	} \
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition() \
	{ \
		return m_spDefn; \
	} \
	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const \
	{ \
		assert((nFieldIndex >= 0) && "Invalid argument"); \
		if (nFieldIndex >= static_cast<int>(m_vecValues.size())) \
		{ \
			return PYXValue(); \
		} \
		return m_vecValues[nFieldIndex]; \
	} \
	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex) \
	{ \
		assert((nFieldIndex >= 0) && "Invalid argument"); \
		int nFieldCount = getDefinition()->getFieldCount(); \
		assert((nFieldIndex < nFieldCount) && "Invalid argument."); \
		if (nFieldCount > static_cast<int>(m_vecValues.size())) \
		{ \
			m_vecValues.resize(nFieldCount); \
		} \
		m_vecValues[nFieldIndex] = value; \
	} \
	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const \
	{ \
		int nField = getDefinition()->getFieldIndex(strName); \
		if (0 <= nField) \
		{ \
			return getFieldValue(nField); \
		} \
		return PYXValue(); \
	} \
	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName) \
	{ \
		int nField = getDefinition()->getFieldIndex(strName); \
		if (0 <= nField) \
		{ \
			setFieldValue(value, nField); \
		} \
	} \
	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const \
	{ \
		std::vector<PYXValue> vecValues; \
		if (!getDefinition()) \
		{ \
			return vecValues; \
		} \
		int nFieldCount = getDefinition()->getFieldCount(); \
		for (int nField = 0; nField < nFieldCount; ++nField) \
		{ \
			vecValues.push_back(getFieldValue(nField)); \
		} \
		return vecValues; \
	} \
	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues) \
	{ \
		int nFieldCount = getDefinition()->getFieldCount(); \
		assert(nFieldCount == vecValues.size()); \
		for (int nField = 0; nField < nFieldCount; ++nField) \
		{ \
			setFieldValue(vecValues[nField], nField); \
		} \
	} \
	virtual void STDMETHODCALLTYPE addField(	const std::string& strName, \
												PYXFieldDefinition::eContextType nContext, \
												PYXValue::eType nType, \
												int nCount = 1, \
												PYXValue value = PYXValue()	) \
	{ \
		PYXPointer<PYXTableDefinition> spDefn(getDefinition()); \
		int nFieldIndex = spDefn->getFieldIndex(strName); \
		if (nFieldIndex < 0) \
		{ \
			nFieldIndex = spDefn->addFieldDefinition(strName, nContext, nType, nCount); \
		} \
		setFieldValue(value, nFieldIndex); \
	}


#define IRECORD_IMPL_PROXY(proxy) \
public: \
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const \
	{ \
		return (proxy).getDefinition(); \
	} \
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition() \
	{ \
		return (proxy).getDefinition(); \
	} \
	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const \
	{ \
		return (proxy).getFieldValue(nFieldIndex); \
	} \
	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex) \
	{ \
	    (proxy).setFieldValue(value,nFieldIndex); \
	} \
	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const \
	{ \
		return (proxy).getFieldValueByName(strName); \
	} \
	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName) \
	{ \
		(proxy).setFieldValueByName(value,strName); \
	} \
	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const \
	{ \
		return (proxy).getFieldValues(); \
	} \
	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues) \
	{ \
		(proxy).setFieldValues(vecValues); \
	} \
	virtual void STDMETHODCALLTYPE addField(	const std::string& strName, \
												PYXFieldDefinition::eContextType nContext, \
												PYXValue::eType nType, \
												int nCount = 1, \
												PYXValue value = PYXValue()	) \
	{ \
		(proxy).addField(strName,nContext,nType,nCount,value); \
	}

#endif // guard
