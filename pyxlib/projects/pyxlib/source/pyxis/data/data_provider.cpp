/******************************************************************************
data_provider.cpp

begin		: 2006-07-19
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "pyxis/data/data_provider.h"

/*!
Get the value of a field by name.

\param	strName	The field name.

\return	The value or null if no field with the specified name.
*/
PYXValue PYXDataProvider::getFieldValueByName(
	const std::string& strName	) const
{
	int nField = getDefinition()->getFieldIndex(strName);
	if (nField >= 0)
	{
		return getFieldValue(nField);
	}

	return PYXValue();
}

/*!
Set the value of a field by name. Not implemented in read-only data sources.
Nothing is done if the field does not exist.

\param	value	The value.
\param	strName	The field name.
*/
void PYXDataProvider::setFieldValueByName(
	PYXValue value,
	const std::string& strName	)
{
	int nField = getDefinition()->getFieldIndex(strName);
	if (nField >= 0)
	{
		return setFieldValue(value, nField);
	}
}

/*!
Get a vector of values corresponding to the definition.

\return	A vector of all field values.
*/
std::vector<PYXValue> PYXDataProvider::getFieldValues() const
{
	int nFieldCount = getDefinition()->getFieldCount();

	std::vector<PYXValue> vecValues;
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		vecValues.push_back(getFieldValue(nField));
	}

	return vecValues;
}

/*!
Set the values from a vector.

\param	vecValues	A vector of all field values.
*/
void PYXDataProvider::setFieldValues(const std::vector<PYXValue>& vecValues)
{
	int nFieldCount = getDefinition()->getFieldCount();

	assert(nFieldCount == vecValues.size());

	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		setFieldValue(vecValues[nField], nField);
	}
}

/*!
Add a field description and value to the data provider.

\param	strName		The name of the field.
\param	nContext	How the field is to be interpreted.
\param	nType		The expected data type of the field.
\param	nCount		The number of values for a single field (i.e. 3 for RGB)
\param	value		The field value.
*/
//! Add a field description and value to the data provider.
void PYXDataProvider::addField(
	const std::string& strName,
	PYXFieldDefinition::eContextType nContext,
	PYXValue::eType nType,
	int nCount,
	PYXValue value	)
{
	PYXPointer<PYXTableDefinition> spDefn(getDefinition());

	// add the field
	int nFieldIndex = spDefn->getFieldIndex(strName);
	if (nFieldIndex < 0)
	{
		nFieldIndex = spDefn->addFieldDefinition(strName, nContext, nType, nCount);
	}

	setFieldValue(value, nFieldIndex);
}
