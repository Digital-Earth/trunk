/******************************************************************************
excel_process.h

begin      : 08/25/2010 12:00:45 PM
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"

#define EXCEL_SOURCE

// local includes
#include "excel_process.h"
#include "excel_feature.h"

// pyxlib includes
#include "pyxis/data/all_feature_iterator.h"
#include "pyxis/geometry/multi_cell.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/region/vector_point_region.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/app_services.h"

// {44978732-EB73-46a0-81DA-B556F6E19C95}
PYXCOM_DEFINE_CLSID(ExcelProcess, 
0x44978732, 0xeb73, 0x46a0, 0x81, 0xda, 0xb5, 0x56, 0xf6, 0xe1, 0x9c, 0x95);
PYXCOM_CLASS_INTERFACES(ExcelProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ExcelProcess, "Excel File Reader", "Point data from a Microsoft Excel file.", "Reader",
	IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_PARAMETER(IPath::iid, 1, 1, "Excel Path", "The input Excel file.");
IPROCESS_SPEC_END

namespace
{
	//! The unit test class
	Tester< ExcelProcess > gTester;

	/*!
	Creates the PYXGeometry object for the process, using the sampling
	resolution specified in the constructor.  It creates a PYXMultiCell
	object and adds the cell for each point in the feature collection.

	\returns The Geometry object.
	*/
	PYXPointer< PYXGeometry > createGeometry(
		boost::intrusive_ptr< IFeatureCollection > spFC)
	{
		assert(spFC);

		PYXPointer< PYXTileCollection > spTileCollection = PYXTileCollection::create();

		// Get each pyxis cell from each feature and put it into the geometry.
		PYXPointer< FeatureIterator > spIt = spFC->getIterator();
		assert(spIt);
		for (; !spIt->end(); spIt->next())
		{
			boost::intrusive_ptr< IFeature > spFeature = spIt->getFeature();

			PYXTileCollection geomAsTiles;

			spFeature->getGeometry()->copyTo(&geomAsTiles);

			spTileCollection->addGeometry(geomAsTiles);			
		}

		return boost::dynamic_pointer_cast< PYXGeometry >(spTileCollection );
	}
}

/*!
The unit test method for the class.
*/
void ExcelProcess::test()
{
	// Create a test process with a path
	boost::intrusive_ptr< IProcess > spProc(new ExcelProcess);
	TEST_ASSERT(spProc);

	// create an excel process
	ExcelProcess * pExcelProcess = dynamic_cast< ExcelProcess * >(spProc.get());
	TEST_ASSERT(pExcelProcess != 0);

	// add a path to it
	boost::intrusive_ptr< IProcess > spPathProc;
	PYXCOMCreateInstance(PathProcess::clsid, 0, IProcess::iid, (void * *)&spPathProc);
	std::map< std::string, std::string > mapAttr;
	boost::filesystem::path path = gTester.getTestDataPath() / FileUtils::stringToPath(
		"xls_process/Circumpolar1_msvd.xls");
	
	if (!FileUtils::exists(path))
	{
		TRACE_TEST("Test data not configured, skipping remaining Excel Process tests.");
		return;
	}
	mapAttr["uri"] = FileUtils::pathToString(path);

	spPathProc->setAttributes(mapAttr);
	spPathProc->initProc();
	spProc->getParameter(0)->addValue(spPathProc);

	// set the attributes
	mapAttr = spProc->getAttributes();
	mapAttr["Table"] = "Circumpolar1_msvd$";
	mapAttr["LatitudeColumn"] = "3";
	mapAttr["LongitudeColumn"] = "5";
	spProc->setAttributes(mapAttr);

	spProc->initProc();

	boost::intrusive_ptr< IFeatureCollection > spFC = 
		boost::dynamic_pointer_cast< IFeatureCollection >(spProc->getOutput());
	TEST_ASSERT(spFC);

	PYXPointer< FeatureIterator > spIt = spFC->getIterator();
	TEST_ASSERT(spIt);

	while (!spIt->end())
	{
		boost::intrusive_ptr< IFeature > spFeature = spIt->getFeature();
		std::vector< PYXValue > vals = spFeature->getFieldValues();
		TEST_ASSERT(vals.size() == spFeature->getDefinition()->getFieldCount());

		spIt->next();
	}

	// write the file out to disk
	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_TEST("Writing ExcelProcess to file: " << FileUtils::pathToString(tempPath));
	PipeManager::writePipelineToFile(FileUtils::pathToString(tempPath), spProc);
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map< std::string, std::string > STDMETHODCALLTYPE
ExcelProcess::getAttributes() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::map< std::string, std::string > mapAttr;
	mapAttr["Name"] = m_strName;
	mapAttr["Table"] = m_strTableName;
	mapAttr["Resolution"] = StringUtils::toString(m_nResolution);
	mapAttr["LabelRow"] = StringUtils::toString(m_nLabelRow);
	mapAttr["LatitudeColumn"] = StringUtils::toString(m_nLatColumn);
	mapAttr["LongitudeColumn"] = StringUtils::toString(m_nLonColumn);
	mapAttr["DataColumnStart"] = StringUtils::toString(m_nDataColumnStart);
	mapAttr["DataColumnEnd"] = StringUtils::toString(m_nDataColumnEndAttribute);
	mapAttr["DataRowStart"] = StringUtils::toString(m_nDataRowStart);
	mapAttr["DataRowEnd"] = StringUtils::toString(m_nDataRowEndAttribute);
	return mapAttr;
}

void STDMETHODCALLTYPE ExcelProcess::setAttributes(
	std::map< std::string, std::string > const & mapAttr)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::map< std::string, std::string >::const_iterator it;

	// Name
	{
		it = mapAttr.find("Name");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No worksheet name (\"Name\") specified for process");
		}
		m_strName = it->second;
	}

	// Table
	{
		it = mapAttr.find("Table");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No table name (\"Table\") specified for process");
		}
		m_strTableName = it->second;
	}

	// Resolution
	{
		it = mapAttr.find("Resolution");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No resolution (\"Resolution\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nResolution);
	}

	// Labels
	{
		it = mapAttr.find("LabelRow");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No label row (\"LabelRow\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nLabelRow);
	}

	// Latitude
	{
		it = mapAttr.find("LatitudeColumn");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No latitude column (\"LatitudeColumn\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nLatColumn);
	}
	
	// Longitude
	{
		it = mapAttr.find("LongitudeColumn");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No longitude column (\"LongitudeColumn\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nLonColumn);
	}

	// First column of data
	// TODO: Rename to "ColumnStart" for consistency with ExcelImportWizard.Page.
	{
		it = mapAttr.find("DataColumnStart");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No data start column (\"DataColumnStart\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nDataColumnStart);
	}
	
	// Last column of data
	// TODO: Rename to "ColumnEnd" for consistency with ExcelImportWizard.Page.
	{
		it = mapAttr.find("DataColumnEnd");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No data end column (\"DataColumnEnd\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nDataColumnEndAttribute);
	}

	// First row of data
	{
		it = mapAttr.find("DataRowStart");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No data start row (\"DataRowStart\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nDataRowStart);
	}
	
	// Last row of data
	{
		it = mapAttr.find("DataRowEnd");
		if (it == mapAttr.end())
		{
			PYXTHROW(AttributeException, "No data end row (\"DataRowEnd\") specified for process");
		}
		StringUtils::fromString(it->second, &m_nDataRowEndAttribute);
	}
}

std::string STDMETHODCALLTYPE ExcelProcess::getAttributeSchema() const
{
    return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"ExcelProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Name\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Name</friendlyName>"
					"<description>A user-friendly name for the worksheet.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"Table\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Table Name</friendlyName>"
					"<description>The table name for the worksheet.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"Resolution\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"LabelRow\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Label Row</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"LatitudeColumn\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Latitude Column</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"LongitudeColumn\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Longitude Column</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"DataColumnStart\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Data Column Start</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"DataColumnEnd\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Data Column End</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"DataRowStart\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Data Row Start</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"DataRowEnd\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Data Row End</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

IProcess::eInitStatus ExcelProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (!(getParameter(0)->getValue(0)))
	{
		PYXTHROW(PYXException, "Missing input.");
	}
	
	// Get the path argument.
	boost::intrusive_ptr< IPath > spPath;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		IPath::iid, (void**) &spPath);
	if (!spPath)
	{
		PYXTHROW(PYXException, "Path parameter was not properly initialized.");
	}
	boost::filesystem::path path = FileUtils::stringToPath(spPath->getLocallyResolvedPath());
	auto pathStr = FileUtils::pathToString(path);
	TRACE_INFO("Attempting to open Excel file: " << pathStr );

	// Get the workbook.
	Excel::IExcel const * pExcel = Excel::IExcel::Implementation();
	if (pExcel == 0)
	{
		PYXTHROW(LoadFailException,"The Excel process failed: No Excel implementation has been specified.");
	}
	try
	{
		m_pWorkbook = pExcel->CreateWorkbook(pathStr);
	}
	catch (...)
	{
		PYXTHROW(LoadFailException,"The Excel process could not open the file \"" <<pathStr << "\"");
	}
	assert(m_pWorkbook);

	// Get the workbook table.
	m_pWorkbookTable = m_pWorkbook->CreateTable(m_strTableName);
	if (!m_pWorkbookTable)
	{
		PYXTHROW(LoadFailException, "The Excel process could not get table \"" << m_strTableName << "\" within file \"" << pathStr  << "\"");
	}

	// Get the workbook view.
	{
		// These calls can throw if there is a problem reading.
		size_t const nRowCount = m_pWorkbookTable->GetRowCount();
		size_t const nColumnCount = m_pWorkbookTable->GetColumnCount();

		assert(0 <= m_nDataRowStart);
		size_t const nFirstRowOffset = m_nDataRowStart;
		assert(0 <= m_nDataColumnStart);
		size_t const nFirstColOffset = m_nDataColumnStart;

		m_nDataRowEnd = m_nDataRowEndAttribute;
		m_nDataColumnEnd = m_nDataColumnEndAttribute;

		// Set row and column end if not yet specified.
		if (m_nDataRowEnd < 0)
		{
			m_nDataRowEnd = nRowCount - 1;
		}
		if (m_nDataColumnEnd < 0)
		{
			m_nDataColumnEnd = nColumnCount - 1;
		}

		size_t const nLastRowOffsetPlusOne = m_nDataRowEnd + 1;
		size_t const nLastColOffsetPlusOne = m_nDataColumnEnd + 1;

		// verify row range
		if (nLastRowOffsetPlusOne <= nFirstRowOffset ||
			nLastRowOffsetPlusOne - nFirstRowOffset > nRowCount)
		{
			PYXTHROW(AttributeException, "Invalid row settings of start: " <<
				m_nDataRowStart << " and end: " << m_nDataRowEnd << 
				" for worksheet with a total row count of: " << nRowCount);
		}

		// verify column range
		if (nLastColOffsetPlusOne <= nFirstColOffset ||
			nLastColOffsetPlusOne - nFirstColOffset > nColumnCount)
		{
			PYXTHROW(AttributeException, "Invalid column settings of start: " <<
				m_nDataColumnStart << " and end: " << m_nDataColumnEnd << 
				" for worksheet with a total column count of: " << nColumnCount);
		}

		// verify lat/lon columns
		{
			assert(0 <= m_nLatColumn);
			size_t const nLatColumn = m_nLatColumn;
			assert(0 <= m_nLonColumn);
			size_t const nLonColumn = m_nLonColumn;

			if (nLatColumn < nFirstColOffset ||
				nLastColOffsetPlusOne <= nLatColumn)
			{
				PYXTHROW(AttributeException, "LatitudeColumn: " << m_nLatColumn << " invalid."); 
			}
			if (nLonColumn < nFirstColOffset ||
				nLastColOffsetPlusOne <= nLonColumn)
			{
				PYXTHROW(AttributeException, "LongitudeColumn: " << m_nLonColumn << " invalid."); 
			}
		}

		// verify label row
		{
			assert(0 <= m_nLabelRow);
			size_t const nLabelRow = m_nLabelRow;

			if (nRowCount <= nLabelRow)
			{
				PYXTHROW(AttributeException, "LabelRow: " << m_nLabelRow << " not in range.");
			}
		}

		m_nRowCount = nLastRowOffsetPlusOne - nFirstRowOffset;

		// Populate data and label rows with the correct number of [empty] values.
		PYXValue empty;
		m_labelRow.assign(m_nDataColumnEnd - m_nDataColumnStart + 1, empty);
		m_dataRow.assign(m_nDataColumnEnd - m_nDataColumnStart + 1, empty);

		// Get the view as per the arguments.
		m_pWorkbookView = m_pWorkbookTable->CreateView(
			m_nDataColumnStart, m_nDataColumnEnd, m_nLabelRow, m_nDataRowStart, m_nDataRowEnd);
		if (m_pWorkbookView == 0)
		{
			PYXTHROW(AttributeException, "The workbook view could not be constructed from the arguments.");
		}
	}

	// IFeature definition for the overall data source
	{
		m_spDefn = PYXTableDefinition::create();

		addField(
			"Work Sheet",
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(m_strName));
	}

	// IFeatureCollection feature definiton
	{
		m_spFeatureDefn = PYXTableDefinition::create();

		// Get the data type for each column, and set in metadata.
		// First, get the column names.
		if (!m_pWorkbookView->GetHeadingRow(m_labelRow))
		{
			PYXTHROW(DefinitionException, "There was a problem retrieving the heading row.");
		}

		// Next, get the first row and ask the PYXValue for its type.
		if (!m_pWorkbookView->GetDataRow(0, m_dataRow))
		{
			PYXTHROW(DefinitionException, "There was a problem retrieving the first data row.");
		}

		assert(m_nDataColumnStart <= m_nDataColumnEnd);
		int columnCount = m_nDataColumnEnd - m_nDataColumnStart + 1;
		for (int nColumn = 0; nColumn < columnCount; ++nColumn)
		{
			// make the field name string unique by adding the offset to it
			std::ostringstream ost;
			ost << m_labelRow[nColumn].getString() << " (" << nColumn << ")";
			std::string fieldName = ost.str();

			PYXValue::eType nDataType = m_dataRow[nColumn].getType();

			m_spFeatureDefn->addFieldDefinition(
				fieldName, 
				PYXFieldDefinition::knContextNone, 
				nDataType);

			// store the column info
			m_offsetTypeMap[nColumn] = nDataType;
		}
	}

	// IFeature variables
	m_strID = "Excel Process: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Create geometry.
	m_spGeom = createGeometry(
		boost::dynamic_pointer_cast< IFeatureCollection >(getOutput()));

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

PYXPointer< FeatureIterator > ExcelProcess::getIterator() const
{
	return AllFeatureIterator::create(*this);
}

// Constructs an excel process feature iterator.
// Does not require the geometry to outlive the feature iterator.
PYXPointer< FeatureIterator > ExcelProcess::getIterator(PYXGeometry const & geometry) const
{
	// TODO: Deprecate this method.
	assert(0); throw std::exception("Not implemented.");
}

boost::intrusive_ptr< IFeature > ExcelProcess::getFeature(std::string const & strFeatureID) const
{
	size_t nOffset = 0;
	StringUtils::fromString(strFeatureID, &nOffset);
	return this->getFeature(nOffset);
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

ExcelProcess::ExcelProcess() :
m_bWritable(false),
m_vecStyles(),
m_mutex(),
m_strName(""),
m_strTableName(""),
m_nLabelRow(0),
m_nLatColumn(0),
m_nLonColumn(1),
m_nDataRowStart(1),
m_nDataRowEnd(-1),
m_nDataRowEndAttribute(-1),
m_nDataColumnStart(0),
m_nDataColumnEnd(-1),
m_nDataColumnEndAttribute(-1),
m_nRowCount(0),
m_pWorkbook(),
m_pWorkbookTable(),
m_pWorkbookView(),
m_nResolution(20),
m_offsetTypeMap(),
m_labelRow(),
m_dataRow()
{
}

size_t ExcelProcess::getFeatureCount() const
{
	boost::recursive_mutex::scoped_lock lock(this->m_mutex);
	return this->m_nRowCount;
}

// Returns null if invalid feature.
boost::intrusive_ptr< ExcelFeature > ExcelProcess::getFeature(size_t nOffset) const
{
	boost::recursive_mutex::scoped_lock lock(this->m_mutex);

	assert(this->m_pWorkbook != 0);
	assert(this->m_pWorkbookTable != 0);
	assert(this->m_pWorkbookView != 0);

	// Get a row of data values.
	if (!this->m_pWorkbookView->GetDataRow(nOffset, this->m_dataRow))
	{
		PYXTHROW(FeatureException, "The feature could not be retrieved.");
	}

	// Get the latitude and longitude, and construct the feature.
	PYXValue lat(this->m_dataRow[this->m_nLatColumn]);
	PYXValue lon(this->m_dataRow[this->m_nLonColumn]);
	if (lat.isNumeric() && lon.isNumeric())
	{
		assert(0 < this->m_nResolution);

		CoordLatLon coord;
		coord.setLatInDegrees(lat.getDouble());
		coord.setLonInDegrees(lon.getDouble());
		
		// Get the region.
		PYXPointer< PYXVectorPointRegion > spRegion(PYXVectorPointRegion::create(SphereMath::llxyz(coord)));

		// Get the geometry.
		PYXPointer< PYXGeometry > spGeometry = PYXVectorGeometry2::create(spRegion,this->m_nResolution);
		
		// Construct and return the feature.
		return new ExcelFeature(this->m_spFeatureDefn, spRegion, spGeometry, this->m_dataRow, nOffset);
	}

	return 0;
}
