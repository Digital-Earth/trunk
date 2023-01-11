/******************************************************************************
catalog.cpp

begin		: 2015-09-17
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/catalog.h"

#include "pyxis/data/exceptions.h"
#include "pyxis/utility/tester.h"

#include <algorithm>
#include "derm/wgs84_coord_converter.h"

////////////////////////////////////////////////////////////////////////////////

//! Header string
const std::string kstrScopeDataSet("PYXDataSet");

//! Identifies an int32 which represents the number of seconds since the start of the Unix epoch
const std::string PYXDataSet::s_strPyxisDimensionTime("PYXIS_DIMENSION_TIME");

//! Identifies an int32 which represents the number of metres above ground level
const std::string PYXDataSet::s_strPyxisDimensionHeight("PYXIS_DIMENSION_HEIGHT");

//! Identifies an int32 which represents the model number in a GRIB ensemble
const std::string PYXDataSet::s_strPyxisDimensionGRIBModel("PYXIS_DIMENSION_GRIB_MODEL");

//! Identifies a string which is the long name for the data set
const std::string PYXDataSet::s_strPyxisLongName("PYXIS_LONG_NAME");

//! Identifies a string which is the short name for the data set
const std::string PYXDataSet::s_strPyxisShortName("PYXIS_SHORT_NAME");

//! Identifies a string that is the units of measurement for the data set
const std::string PYXDataSet::s_strPyxisUnits("PYXIS_UNITS");

//! Identifies a double which represents the number of features in the data set
const std::string PYXDataSet::s_strPyxisFeatureCount("PYXIS_FEATURE_COUNT");

//! Identifies a double which represents the number of pixels in the data set
const std::string PYXDataSet::s_strPyxisPixelCount("PYXIS_PIXEL_COUNT");

//! Tester class
Tester<PYXDataSet> gTesterDataSet;

//! Test method
void PYXDataSet::test()
{
	PYXDataSet ds;

	{
		ds.setUri("uri");
		ds.setName("name");
		ds.setLayer("layer");
		ds.addField(
			"field1",
			PYXFieldDefinition::eContextType::knContextNone,
			PYXValue::eType::knString,
			1,
			PYXValue("field1 value"));
		ds.addField(
			"field2",
			PYXFieldDefinition::eContextType::knContextNone,
			PYXValue::eType::knString,
			1,
			PYXValue("field2 value"));
		ds.getContentDefinition()->addFieldDefinition(
			"field3",
			PYXFieldDefinition::eContextType::knContextNone,
			PYXValue::eType::knNull,
			1	);
		ds.getContentDefinition()->addFieldDefinition(
			"field4",
			PYXFieldDefinition::eContextType::knContextNone,
			PYXValue::eType::knNull,
			2	);

		ds.m_vecMissingRequiredFilesAllOf.push_back("required_all.txt");
		ds.m_vecMissingRequiredFilesOneOf.push_back("required_one.txt");
		ds.m_vecMissingOptionalFiles.push_back("optional.txt");

		// test accessors
		TEST_ASSERT(ds.getUri() == "uri");
		TEST_ASSERT(ds.getName() == "name");
		TEST_ASSERT(ds.getLayer() == "layer");
		TEST_ASSERT(ds.getFieldValueByName("field1").getString() == "field1 value");	
		TEST_ASSERT(ds.getContentDefinition()->getFieldDefinition(0).getName() == "field3");
		TEST_ASSERT(ds.getMissingRequiredFilesAllOf().size() == 1);
		TEST_ASSERT(ds.getMissingRequiredFilesAllOf()[0] == "required_all.txt");
		TEST_ASSERT(ds.getMissingRequiredFilesOneOf().size() == 1);
		TEST_ASSERT(ds.getMissingRequiredFilesOneOf()[0] == "required_one.txt");
		TEST_ASSERT(ds.getMissingOptionalFiles().size() == 1);
		TEST_ASSERT(ds.getMissingOptionalFiles()[0] == "optional.txt");
	}

	{
		// test copy constructor and equality operator
		PYXDataSet ds2(ds);
		TEST_ASSERT(ds == ds2);
		TEST_ASSERT(!(ds != ds2));
	}

	{
		// test copy assignment and equality operator
		PYXDataSet ds2;
		ds2 = ds;
		TEST_ASSERT(ds == ds2);
		TEST_ASSERT(!(ds != ds2));
	}

	{
		// test streaming
		std::stringstream ss;
		ss << ds;

		PYXDataSet ds2;
		ss >> ds2;

		TEST_ASSERT(ds == ds2);
	}
}

/*!
Default constructor.
*/
PYXDataSet::PYXDataSet() :
	m_spDefn(PYXTableDefinition::create()),
	m_pContentDefinition(PYXTableDefinition::create())
{
}

/*!
Constructor

\param strUri	The uri for the data set
\param strName	The name of the data set
*/
PYXDataSet::PYXDataSet(const std::string& strUri, const std::string& strName) :
	m_strUri(strUri),
	m_strName(strName),
	m_spDefn(PYXTableDefinition::create()),
	m_pContentDefinition(PYXTableDefinition::create())
{
}

/*!
Copy constructor

\param dataSet	The data set to assign to this one.
*/
PYXDataSet::PYXDataSet(const PYXDataSet& dataSet)
{
	m_strUri = dataSet.m_strUri;
	m_strName = dataSet.m_strName;
	m_strLayer = dataSet.m_strLayer;
	m_spDefn = dataSet.m_spDefn->clone();
	m_vecValues = dataSet.m_vecValues;
	m_pContentDefinition = dataSet.m_pContentDefinition->clone();
	m_vecMissingRequiredFilesAllOf = dataSet.m_vecMissingRequiredFilesAllOf;
	m_vecMissingRequiredFilesOneOf = dataSet.m_vecMissingRequiredFilesOneOf;
	m_vecMissingOptionalFiles = dataSet.m_vecMissingOptionalFiles;
	m_bbox1 = dataSet.m_bbox1;
	m_bbox2 = dataSet.m_bbox2;
}

/*!
Copy assignment.

\param dataSet	The data set to assign to this one.
*/
PYXDataSet PYXDataSet::operator=(const PYXDataSet& dataSet)
{
	if (this != &dataSet)
	{
		m_strUri = dataSet.m_strUri;
		m_strName = dataSet.m_strName;
		m_strLayer = dataSet.m_strLayer;
		m_spDefn = dataSet.m_spDefn->clone();
		m_vecValues = dataSet.m_vecValues;
		m_pContentDefinition = dataSet.m_pContentDefinition->clone();
		m_vecMissingRequiredFilesAllOf = dataSet.m_vecMissingRequiredFilesAllOf;
		m_vecMissingRequiredFilesOneOf = dataSet.m_vecMissingRequiredFilesOneOf;
		m_vecMissingOptionalFiles = dataSet.m_vecMissingOptionalFiles;
		m_bbox1 = dataSet.m_bbox1;
		m_bbox2 = dataSet.m_bbox2;
	}

	return *this;
}

void PYXDataSet::getBoundingBox(PYXRect2DDouble & bbox1, PYXRect2DDouble & bbox2) const
{
	bbox1 = m_bbox1;
	bbox2 = m_bbox2;
}

void PYXDataSet::setBoundingBox(PYXGeometry & geometry)
{
	WGS84CoordConverter converter;
	geometry.getBoundingRects(&converter, &m_bbox1, &m_bbox2);
}

/*!
Checks to see if the data sets are equal. The catalog definitions are
considered to be equal if they contain the same uri and name.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if the same, false otherwise.
*/
bool operator==(const PYXDataSet& lhs, const PYXDataSet& rhs)
{
	return (	(lhs.m_strUri == rhs.m_strUri) &&
		(lhs.m_strName == rhs.m_strName) &&
		(lhs.m_strLayer == rhs.m_strLayer) &&
		(*lhs.m_spDefn == *rhs.m_spDefn) &&
		(lhs.m_vecValues == rhs.m_vecValues) &&
		(*lhs.m_pContentDefinition == *rhs.m_pContentDefinition) &&
		(lhs.m_vecMissingRequiredFilesAllOf == rhs.m_vecMissingRequiredFilesAllOf) &&
		(lhs.m_vecMissingRequiredFilesOneOf == rhs.m_vecMissingRequiredFilesOneOf) &&
		(lhs.m_vecMissingOptionalFiles == rhs.m_vecMissingOptionalFiles)	&&
		(lhs.m_bbox1 == rhs.m_bbox1)	&&
		(lhs.m_bbox2 == rhs.m_bbox2)	);
}

/*!
Checks to see if the data sets are not equal.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if not equal, false if equal.
*/
bool operator !=(const PYXDataSet& lhs, const PYXDataSet& rhs) 
{
	return !(lhs == rhs);
}

/*!
Write a PYXDataSet to a stream.
*/
std::ostream& operator <<(std::ostream& out, const PYXDataSet& dataSet)
{
	// write header
	out << kstrScopeDataSet << " " << "0.3" << std::endl;

	// write the uri
	out << dataSet.m_strUri << std::endl;

	// write the name
	out << dataSet.m_strName << std::endl;

	// write the layer
	out << dataSet.m_strLayer << std::endl;

	// write table definition for the data set
	out << *dataSet.m_spDefn << std::endl;

	// write values for the data set
	out << dataSet.m_vecValues.size() << std::endl;
	for (auto &value : dataSet.m_vecValues)
	{
		out << value << std::endl;
	}

	// write table definition for the content
	out << *dataSet.m_pContentDefinition << std::endl;

	// write values for the missing required files
	out << dataSet.m_vecMissingRequiredFilesAllOf.size() << std::endl;
	for (auto& value : dataSet.m_vecMissingRequiredFilesAllOf)
	{
		out << value << std::endl;
	}

	// write values for the missing required files
	out << dataSet.m_vecMissingRequiredFilesOneOf.size() << std::endl;
	for (auto& value : dataSet.m_vecMissingRequiredFilesOneOf)
	{
		out << value << std::endl;
	}

	// write values for the missing optional files
	out << dataSet.m_vecMissingOptionalFiles.size() << std::endl;
	for (auto& value : dataSet.m_vecMissingOptionalFiles)
	{
		out << value << std::endl;
	}

	// write table definition for the data set
	out << dataSet.m_bbox1 << std::endl;

	// write table definition for the data set
	out << dataSet.m_bbox2 << std::endl;

	return out;
}

/*!
Read a PYXDataSet from a stream.
*/
std::istream& operator >>(std::istream& in, PYXDataSet& dataSet)
{
	// clear the uri
	dataSet.m_strUri.clear();

	// clear the name
	dataSet.m_strName.clear();

	// clear the layer
	dataSet.m_strLayer.clear();

	// clear the table definition for the data set
	dataSet.m_spDefn->clear();

	// clear the values for the data set
	dataSet.m_vecValues.clear();

	// clear the table definition for the content
	dataSet.m_pContentDefinition->clear();

	// clear the missing required files
	dataSet.m_vecMissingRequiredFilesAllOf.clear();

	// clear the missing required files
	dataSet.m_vecMissingRequiredFilesOneOf.clear();

	// clear the missing optional files
	dataSet.m_vecMissingOptionalFiles.clear();

	// read and check for valid header
	std::string strArg;
	in >> strArg;
	if (strArg != kstrScopeDataSet)
	{
		PYXTHROW(PYXDefinitionException, "Invalid data set.");
	}

	// read version
	std::string strVersion;
	in >> strVersion;

	// consume trailing newline
	in.ignore(1, '\n');

	// read the uri
	getline(in, dataSet.m_strUri);

	// read the name
	getline(in, dataSet.m_strName);

	// read the layer
	getline(in, dataSet.m_strLayer);

	// read the table definition for the data set
	in >> *dataSet.m_spDefn;

	// consume trailing newline
	in.ignore(1, '\n');

	// read the values for the data set
	int nCount;
	in >> nCount;

	// consume trailing newline
	in.ignore(1, '\n');

	for (auto i = 0; i < nCount; ++i)
	{
		PYXValue value;
		in >> value;
		dataSet.m_vecValues.push_back(value);

		// consume trailing newline
		in.ignore(1, '\n');
	}

	// read the table definition for the content
	in >> *dataSet.m_pContentDefinition;

	// consume trailing newline
	in.ignore(1, '\n');

	if (strVersion > "0.1")
	{
		{
			// read the number of missing required files
			int nCount;
			in >> nCount;

			// consume trailing newline
			in.ignore(1, '\n');

			for (auto i = 0; i < nCount; ++i)
			{
				std::string value;
				in >> value;
				dataSet.m_vecMissingRequiredFilesAllOf.push_back(value);

				// consume trailing newline
				in.ignore(1, '\n');
			}
		}

		{
			// read the number of missing required files
			int nCount;
			in >> nCount;

			// consume trailing newline
			in.ignore(1, '\n');

			for (auto i = 0; i < nCount; ++i)
			{
				std::string value;
				in >> value;
				dataSet.m_vecMissingRequiredFilesOneOf.push_back(value);

				// consume trailing newline
				in.ignore(1, '\n');
			}
		}

		{
			// read the number of missing optional files
			int nCount;
			in >> nCount;

			// consume trailing newline
			in.ignore(1, '\n');

			for (auto i = 0; i < nCount; ++i)
			{
				std::string value;
				in >> value;
				dataSet.m_vecMissingOptionalFiles.push_back(value);

				// consume trailing newline
				in.ignore(1, '\n');
			}
		}
	}

	if (strVersion > "0.2")
	{
		in >> dataSet.m_bbox1;

		// consume trailing newline
		in.ignore(1, '\n');

		in >> dataSet.m_bbox2;

		// consume trailing newline
		in.ignore(1, '\n');
	}

	return in;
}

////////////////////////////////////////////////////////////////////////////////

//! Header string
const std::string kstrScopeCatalog("PYXCatalog");

//! Tester class
Tester<PYXCatalog> gTesterCatalog;

//! Test method
void PYXCatalog::test()
{
	PYXCatalog cat;

	// test accessors
	{
		cat.setName("name");
		cat.setUri("uri");

		PYXPointer<PYXDataSet> pds = PYXDataSet::create();
		pds->setUri("uri");
		pds->setName("name");
		pds->setLayer("layer");
		cat.addDataSet(pds);

		PYXPointer<PYXCatalog> psub = PYXCatalog::create();
		psub->setUri("suburi");
		psub->setName("subname");
		cat.addSubCatalog(psub);

		TEST_ASSERT(cat.getUri() == "uri");
		TEST_ASSERT(cat.getName() == "name");
		TEST_ASSERT(cat.getDataSetCount() == 1);
		TEST_ASSERT(*(cat.getDataSet(0)) == *pds);
		TEST_ASSERT(cat.getSubCatalogCount() == 1);
		TEST_ASSERT(*(cat.getSubCatalog(0)) == *psub);
	}

	{
		// test copy constructor and equality operator
		PYXCatalog cat2(cat);
		TEST_ASSERT(cat == cat2);
		TEST_ASSERT(!(cat != cat2));
	}

	{
		// test copy assignment and equality operator
		PYXCatalog cat2;
		cat2 = cat;
		TEST_ASSERT(cat == cat2);
		TEST_ASSERT(!(cat != cat2));
	}

	{
		// test streaming
		std::stringstream ss;
		ss << cat;

		PYXCatalog cat2;
		ss >> cat2;

		TEST_ASSERT(cat == cat2);
	}
}

/*!
Constructor

\param strUri	The uri for the catalog
\param strName	The name of the catalog
*/
PYXCatalog::PYXCatalog(const std::string& strUri, const std::string& strName) :
	m_strUri(strUri),
	m_strName(strName)
{
}

/*!
Copy constructor

\param catalog	The catalog to assign to this one.
*/
PYXCatalog::PYXCatalog(const PYXCatalog& catalog)
{
	m_strUri = catalog.m_strUri;
	m_strName = catalog.m_strName;
	m_vecDataSets = catalog.m_vecDataSets;
	m_vecSubCatalogs = catalog.m_vecSubCatalogs;
}

/*!
Copy assignment.

\param catalog	The catalog to assign to this one.
*/
PYXCatalog PYXCatalog::operator=(const PYXCatalog& catalog)
{
	if (this != &catalog)
	{
		m_strUri = catalog.m_strUri;
		m_strName = catalog.m_strName;

		for (	auto itDataSet = catalog.m_vecDataSets.begin();
			itDataSet != catalog.m_vecDataSets.end();
			++itDataSet	)
		{
			m_vecDataSets.push_back((*itDataSet)->clone());
		}

		for (	auto itCatalog = catalog.m_vecSubCatalogs.begin();
			itCatalog != catalog.m_vecSubCatalogs.end();
			++itCatalog	)
		{
			m_vecSubCatalogs.push_back((*itCatalog)->clone());
		}
	}

	return *this;
}

/*!
Add a data set.

\param	pDataSet	The data set.

\return	The index of the data set.
*/
int PYXCatalog::addDataSet(PYXPointer<PYXDataSet> pDataSet)
{
	m_vecDataSets.push_back(pDataSet);

	return static_cast<int>(m_vecDataSets.size() - 1);
}

/*!
Get a data set by index. Asserts if the index is out of range.

\param	nDataSetIndex	The index.

\return	The data set.
*/
PYXPointer<const PYXDataSet> PYXCatalog::getDataSet(int nDataSetIndex) const
{
	assert(nDataSetIndex >= 0 && "Invalid argument");
	assert(nDataSetIndex < static_cast<int>(m_vecDataSets.size()) && "Invalid argument.");
	return m_vecDataSets[nDataSetIndex];
}

/*!
Add a sub-catalog.

\param	pCatalog	The sub-catalog.

\return	The index of the catalog.
*/
int PYXCatalog::addSubCatalog(PYXPointer<PYXCatalog> pCatalog)
{
	m_vecSubCatalogs.push_back(pCatalog);

	return static_cast<int>(m_vecSubCatalogs.size() - 1);
}

/*!
Get a sub-catalog by index. Asserts if the index is out of range.

\param	nCatalogIndex	The index.

\return	The sub-catalog.
*/
PYXPointer<const PYXCatalog> PYXCatalog::getSubCatalog(int nCatalogIndex) const
{
	assert(nCatalogIndex >= 0 && "Invalid argument");
	assert(nCatalogIndex < static_cast<int>(m_vecSubCatalogs.size()) && "Invalid argument.");
	return m_vecSubCatalogs[nCatalogIndex];
}

/*!
Checks to see if the catalogs are equal. The catalogs are considered to be equal
if they contain the same uri, name, data sets and sub-catalogs in the same order.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if the same, false otherwise.
*/
bool operator==(const PYXCatalog& lhs, const PYXCatalog& rhs)
{
	if (lhs.m_strUri != rhs.m_strUri)
	{
		return false;
	}

	if (lhs.m_strName != rhs.m_strName)
	{
		return false;
	}

	auto nDataSetCount = lhs.getDataSetCount();
	if (rhs.getDataSetCount() != nDataSetCount)
	{
		return false;
	}

	for (int i = 0; i < nDataSetCount; i++)
	{
		if (*(lhs.getDataSet(i)) != *(rhs.getDataSet(i)))
		{
			return false;
		}
	}

	auto nSubCatalogCount = lhs.getSubCatalogCount();
	if (rhs.getSubCatalogCount() != nSubCatalogCount)
	{
		return false;
	}

	for (int i = 0; i < nSubCatalogCount; i++)
	{
		if (*(lhs.getSubCatalog(i)) != *(rhs.getSubCatalog(i)))
		{
			return false;
		}
	}

	return true;
}

/*!
Checks to see if the catalogs are not equal.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if not equal, false if equal.
*/
bool operator !=(const PYXCatalog& lhs, const PYXCatalog& rhs) 
{
	return !(lhs == rhs);
}

/*!
Write a PYXCatalog to a stream.
*/
std::ostream& operator <<(std::ostream& out, const PYXCatalog& catalog)
{
	// write header
	out << kstrScopeCatalog << " " << "0.1" << std::endl;

	// write the uri
	out << catalog.getUri() << std::endl;

	// write the name
	out << catalog.getName() << std::endl;

	int nDataSetCount = catalog.getDataSetCount();

	// write number of data set definitions
	out << nDataSetCount << std::endl;

	// write data set definitions
	for (int nDataSet = 0; nDataSet < nDataSetCount; ++nDataSet)
	{
		out << *(catalog.getDataSet(nDataSet)) << std::endl;
	}

	int nCatalogCount = catalog.getSubCatalogCount();

	// write number of catalog definitions
	out << nCatalogCount << std::endl;

	// write catalogs
	for (int nCatalog = 0; nCatalog < nCatalogCount; ++nCatalog)
	{
		out << *(catalog.getSubCatalog(nCatalog)) << std::endl;
	}

	return out;
}

/*!
Read a PYXCatalog from a stream.
*/
std::istream& operator >>(std::istream& in, PYXCatalog& catalog)
{
	// clear the uri
	catalog.m_strUri.clear();

	// clear the name
	catalog.m_strName.clear();

	// empty any previous data set definitions
	catalog.m_vecDataSets.clear();

	// empty any previous catalog definitions
	catalog.m_vecSubCatalogs.clear();

	// read and check for valid header
	std::string strArg;
	in >> strArg;
	if (strArg != kstrScopeCatalog)
	{
		PYXTHROW(PYXDefinitionException, "Invalid catalog definition.");
	}

	// read version
	std::string strVersion;
	in >> strVersion;

	// consume trailing newline
	in.ignore(1, '\n');

	// read the uri
	getline(in, catalog.m_strUri);

	// read the name
	getline(in, catalog.m_strName);

	// read number of data sets
	int nDataSetCount;
	in >> nDataSetCount;

	// consume trailing newline
	in.ignore(1, '\n');

	// read data sets
	for (int nDataSet = 0; nDataSet < nDataSetCount; ++nDataSet)
	{
		auto pDataSet = PYXDataSet::create();

		in >> *pDataSet;
		catalog.addDataSet(pDataSet);

		// consume trailing newline
		in.ignore(1, '\n');
	}

	// read number of sub-catalogs
	int nCatalogCount;
	in >> nCatalogCount;

	// consume trailing newline
	in.ignore(1, '\n');

	// read sub-catalogs
	for (int nCatalog = 0; nCatalog < nCatalogCount; ++nCatalog)
	{
		auto pSubCatalog = PYXCatalog::create();

		in >> *pSubCatalog;
		catalog.addSubCatalog(pSubCatalog);

		// consume trailing newline
		in.ignore(1, '\n');
	}

	return in;
}
