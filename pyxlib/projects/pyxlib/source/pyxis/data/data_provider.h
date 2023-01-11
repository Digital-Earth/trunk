#ifndef PYXIS__DATA_SOURCE__DATA_PROVIDER_H
#define PYXIS__DATA_SOURCE__DATA_PROVIDER_H
/******************************************************************************
data_provider.h

begin		: 2006-07-19
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/table_definition.h"

//! Abstract base for classes that have a definition and values.
/*!
PYXDataProvider is the abstract base for all classes that have a definition
and values associated with the definition.
*/
class PYXLIB_DECL PYXDataProvider
{
public:

	//! Destructor
	virtual ~PYXDataProvider() {}

	/*!
	Get the definition that describes the fields in this data provider.

	\return	The definition.
	*/
	//! Get the table definition
	virtual PYXPointer<const PYXTableDefinition> getDefinition() const = 0;

	/*!
	Get the definition that describes the fields in this data provider.

	\return	The definition.
	*/
	//! Get the table definition
	virtual PYXPointer<PYXTableDefinition> getDefinition() = 0;

	/*!
	Get the value of a field by index. Use getDefinition->getFieldIndex() to
	get the appropriate field index.

	\param	nFieldIndex	The field index.

	\return	The value.
	*/
	//! Get a field value by index.
	virtual PYXValue getFieldValue(int nFieldIndex) const = 0;

	/*!
	Set the value of a field by index. Use getDefinition()->getFieldIndex() to
	get the appropriate field index. Not implemented in read-only data sources.

	\param	nFieldIndex	The field index.
	\param	value	The value.
	*/
	//! Set a field value by index.
	virtual void setFieldValue(PYXValue value, int nFieldIndex)
	{
		assert(false && "Not implemented.");
	}

	//! Get a field value by name.
	PYXValue getFieldValueByName(const std::string& strName) const;

	//! Set a field value by name.
	void setFieldValueByName(PYXValue value, const std::string& strName);

	//! Get a vector of all field values.
	std::vector<PYXValue> getFieldValues() const;

	//! Get a vector of all field values.
	void setFieldValues(const std::vector<PYXValue>& vecValues);

	//! Add a field description and value to the data provider.
	void addField(
		const std::string& strName,
		PYXFieldDefinition::eContextType nContext,
		PYXValue::eType nType,
		int nCount = 1,
		PYXValue value = PYXValue()	);

protected:

	//! Constructor.
	PYXDataProvider() {}
};

#endif // guard
