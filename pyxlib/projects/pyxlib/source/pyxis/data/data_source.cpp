/******************************************************************************
data_source.cpp

begin		: 2004-06-22
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "pyxis/data/data_source.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/geometry/geometry.h"

//! Class name
const std::string PYXDataSource::kstrScope = "PYXDataSource";

// Field names
const std::string PYXDataSource::kstrDataSourceName = "name";
const std::string PYXDataSource::kstrDataSourceType = "type";

/*!
Constructor creates the fields required by all data sources.
*/
PYXDataSource::PYXDataSource()
{
	addRequiredDataSourceFields();
}

/*!
Add required fields to the data source definition if not already present.
*/
void PYXDataSource::addRequiredDataSourceFields()
{
	// add name field to definition if it does not exist
	if (getDefinition()->getFieldIndex(kstrDataSourceName) < 0)
	{
		addField(	kstrDataSourceName,
					PYXFieldDefinition::knContextNone,
					PYXValue::knString	);
	}

	// add type field to definition
	if (getDefinition()->getFieldIndex(kstrDataSourceType) < 0)
	{
		addField(	kstrDataSourceType,
					PYXFieldDefinition::knContextNone,
					PYXValue::knInt32	);
	}
}

/*!
Get the ID of the feature.  Not supported for this data source.

\return the ID of the feature.
*/
//!	Get the ID of the feature.
const std::string& PYXDataSource::getID() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	PYXTHROW(PYXDataSourceException, "Function not yet supported on data source.");

	const static std::string strResult("Not yet supported");
	return strResult;
}

/*!
Set the field definitions for this data source. Overridden to resize the value
vector. This method clears all values.

\param	spDefn	The new definition.
*/
void PYXDataSource::setDefinition(PYXPointer<PYXTableDefinition> spDefn)
{
	// call base class
	PYXFeature::setDefinition(spDefn);

	// ensure the required fields are present
	addRequiredDataSourceFields();

	// recreate the values
	m_vecValues.clear();
	m_vecValues.resize(spDefn->getFieldCount());
}

/*!
Convenience method for getting the data source name.

\return	The data source name.
*/
std::string PYXDataSource::getName() const
{
	return getFieldValueByName(kstrDataSourceName).getString();
}

/*!
Convenience method for setting the data source name.

\param	strName	The data source name.
*/
void PYXDataSource::setName(const std::string& strName)
{
	setFieldValueByName(PYXValue(strName), kstrDataSourceName);
}

/*!
Convenience method for getting the data source type.

\return	The data source type.
*/
PYXDataSource::eType PYXDataSource::getType() const
{
	return static_cast<eType>(getFieldValueByName(kstrDataSourceType).getInt());
}

/*!
Convenience method for setting the data source type.

\param	nType	The data source type.
*/
void PYXDataSource::setType(PYXDataSource::eType nType)
{
	setFieldValueByName(PYXValue(static_cast<int>(nType)), kstrDataSourceType);
}

/*!
Convenience method for getting the resolution for the data source.

\return	The resolution or -1 if no geometry has been set.
*/
int PYXDataSource::getResolution() const
{
	PYXPointer<const PYXGeometry> spGeometry(getGeometry());
	if (0 == spGeometry)
	{
		return -1;
	}
	return spGeometry->getCellResolution();
}

#if 0
/*!
Specify the spatial reference for the data source. Call this method to set
the spatial reference if after the data source is opened
hasSpatialReference() returns false. PYXIS data sources do not require a
spatial reference system.

\param	spSRS	The spatial reference system.
*/
void PYXDataSource::setSpatialReferenceSystem(
	PYXPointer<PYXSpatialReferenceSystem> spSRS	)
{
	assert(false);
}
#endif

/*!
Get the value of a field by index. Use getDefinition->getFieldIndex() to
get the appropriate field index.

\param	nFieldIndex	The field index.

\return	The value.
*/
PYXValue PYXDataSource::getFieldValue(int nFieldIndex) const
{
	assert((nFieldIndex >= 0) && "Invalid argument");

	if (nFieldIndex >= static_cast<int>(m_vecValues.size()))
	{
		return PYXValue();
	}

	return m_vecValues[nFieldIndex];
}

/*!
Set the value of a field by index. Use getDefinition()->getFieldIndex() to
get the appropriate field index. Not implemented in read-only data sources.

\param	value		The value.
\param	nFieldIndex	The field index.
*/
void PYXDataSource::setFieldValue(PYXValue value, int nFieldIndex)
{
	assert((nFieldIndex >= 0) && "Invalid argument");

	int nFieldCount = getDefinition()->getFieldCount();
	assert((nFieldIndex < nFieldCount) && "Invalid argument.");

	if (nFieldCount > static_cast<int>(m_vecValues.size()))
	{
		m_vecValues.resize(nFieldCount);
	}

	m_vecValues[nFieldIndex] = value;
}

/*!
Update fields based on the definitions and field values from an input data
source. This method is useful for data sources that are composed from a number
of sub-data sources. This method tracks the number of inputs, the minimum and
maximum of numeric fields and the first instance of all other fields.

\param	ds	The input data source.
*/
void PYXDataSource::updateFields(const PYXDataSource& ds)
{
	static const std::string kstrInputs("inputs");

	// for each field in the input data source
	int nFieldCount = ds.getDefinition()->getFieldCount();
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		const PYXFieldDefinition& defn = ds.getDefinition()->getFieldDefinition(
			nField	);

		// if it is the name field, count the number of data sources
		std::string strFieldName = defn.getName();
		if (strFieldName == kstrDataSourceName)
		{
			int nNewField = getDefinition()->getFieldIndex(kstrInputs);
			if (nNewField < 0)
			{
				addField(
					kstrInputs,
					PYXFieldDefinition::knContextNone,
					PYXValue::knInt32,
					1,
					PYXValue(1)	);
			}
			else
			{
				int nCount = getFieldValue(nNewField).getInt();
				++nCount;
				setFieldValue(PYXValue(nCount), nNewField);
			}
		}
		// assume data type is set by other means
		else if (strFieldName == kstrDataSourceType)
		{
			// ignore
		}
		// if it is a numeric field track the minimum and maximum values
		else if (defn.isNumeric())
		{
			// get value of input field
			double fValue = ds.getFieldValue(nField).getDouble();

			// keep track of the minimum value so far
			std::string strFieldNameMin = strFieldName + " min";
			int nNewField = getDefinition()->getFieldIndex(strFieldNameMin);
			if (nNewField < 0)
			{
				addField(
					strFieldNameMin,
					PYXFieldDefinition::knContextNone,
					PYXValue::knDouble,
					1,
					PYXValue(fValue)	);
			}
			else
			{
				double fNewValue = getFieldValue(nNewField).getDouble();
				if (fValue < fNewValue)
				{
					setFieldValue(PYXValue(fValue), nNewField);
				}
			}

			// keep track of the maximum value so far
			std::string strFieldNameMax = strFieldName + " max";
			nNewField = getDefinition()->getFieldIndex(strFieldNameMax);
			if (nNewField < 0)
			{
				addField(
					strFieldNameMax,
					PYXFieldDefinition::knContextNone,
					PYXValue::knDouble,
					1,
					PYXValue(fValue)	);
			}
			else
			{
				double fNewValue = getFieldValue(nNewField).getDouble();
				if (fValue > fNewValue)
				{
					setFieldValue(PYXValue(fValue), nNewField);
				}
			}
		}
		// for all other fields, track the first value found
		else
		{
			// keep track of the maximum value so far
			int nNewField = getDefinition()->getFieldIndex(strFieldName);
			if (nNewField < 0)
			{
				addField(
					strFieldName,
					defn.getContext(),
					defn.getType(),
					defn.getCount(),
					ds.getFieldValue(nField)	);
			}
		}
	}
}
