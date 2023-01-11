/******************************************************************************
coverage_geometry_mask_process.cpp

begin		: 2008-05-08
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "coverage_geometry_mask_process.h"

//  pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exception.h"

// {33DED4C7-3FEC-409e-B476-536585F595DB}
PYXCOM_DEFINE_CLSID(CoverageGeometryMaskProcess,
0x33ded4c7, 0x3fec, 0x409e, 0xb4, 0x76, 0x53, 0x65, 0x85, 0xf5, 0x95, 0xdb);

PYXCOM_CLASS_INTERFACES(CoverageGeometryMaskProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(CoverageGeometryMaskProcess, "Mask by Area", "Masks input coverage based on a mask geometry.", "Analysis/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The main data input coverage.")
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "AOI Geometry", "The Area of Interest geometry used to mask the input coverage.")
IPROCESS_SPEC_END

//! Constant string data.
const std::string CoverageGeometryMaskProcess::kstrScope = "CoverageGeometryMaskProcess";

const char* const opStr[] =
{
	"Return data inside the AOI geometry.",
	"Return data outside the AOI geometry."
	// on the boundary of?
};

//! The unit test class
Tester<CoverageGeometryMaskProcess> gTester;

/*!
The unit test method for the class.
*/
void CoverageGeometryMaskProcess::test()
{
	// TODO: add meaningful tests.
}

/*!
Default Constructor.
*/
CoverageGeometryMaskProcess::CoverageGeometryMaskProcess()
{
	m_mode = knReturnDataInside;
}


/*!
Intialize this process by querying the two input parameters for their values and 
storing those values in an input and control variable to eliminate the need for 
querying the interface every time we wish to access these values. 
*/
IProcess::eInitStatus CoverageGeometryMaskProcess::initImpl()
{
	m_strID = "Coverage Geometry Mask: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	m_spInputCoverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
	boost::intrusive_ptr<IFeature> spInputFeature; 
	spInputFeature = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IFeature>();
	m_spMaskGeometry = spInputFeature->getGeometry();
	
	// TODO: maybe preprocess both input geometries into TileCollections and see if that makes things faster.

	return knInitialized;
}

/*! 
Gets the attributes to that need to be serialized or displayed in the pipe editor
with this process. The attributes are entered into a map of string(key)
to string(values) and the map is retuned to be written out.

\return A stl map containing the attributes to be saved, or returned to the pipe editor.
*/
std::map<std::string, std::string> CoverageGeometryMaskProcess::getAttributes() const
{
	std::map<std::string, std::string> attrib;
	attrib.clear();
	attrib["OperatingMode"] = opStr[m_mode];
	return attrib;
}

/*!
Sets the attributes either when the process is deserializing or when 
the attributes have been altered in the pipe editor. The stl map 
passed in is searched for key-value pairs and the values are parsed
out into their respective variables. 

\param attribs A map of the attributes to be set in this process.
*/
void CoverageGeometryMaskProcess::setAttributes(const std::map<std::string, std::string>& attribs)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string,std::string>::const_iterator it = attribs.find("OperatingMode");
	if (it != attribs.end())
	{
		for (int n = 0; n != sizeof(opStr)/sizeof(opStr[0]); ++n)
		{
			if (strcmp(opStr[n], it->second.c_str()) == 0)
			{
				m_mode = static_cast<CoverageGeometryMaskProcess::eFilterMode>(n);
			}
		}
	}
}

/*!
Get the coverage value at specified index.
\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	Data from the input based on the exsistence of data on the control.
*/
PYXValue CoverageGeometryMaskProcess::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	if (m_mode == knReturnDataInside)
	{
		if (m_spMaskGeometry->intersects(PYXCell(index)))
		{
			return m_spInputCoverage->getCoverageValue(index, nFieldIndex);
		}
	}

	if (m_mode == knReturnDataOutside)
	{
		if (!m_spMaskGeometry->intersects(PYXCell(index)))
		{
			return m_spInputCoverage->getCoverageValue(index, nFieldIndex);
		}
	}

	// we failed the test, so return a null PYXValue
	return PYXValue(); 
}

PYXPointer<PYXValueTile> CoverageGeometryMaskProcess::getFieldTile(	const PYXIcosIndex& index,
															int nRes,
															int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// our goal tile, which we are constructing	will be a copy of the data tile
	// with the data NULLed out where it does not pass the criteria.
	PYXPointer<PYXValueTile> spValueTile;

	// Find out if this tile intersects the AOI geometery.
	bool bIntersectsTile = m_spMaskGeometry->intersects(PYXTile(index, nRes));

	// If the whole tile is outside the AOI geometry and we are passing back data inside
	// the AOI geometery then we can pass back a null tile now.
	if (m_mode == knReturnDataInside  && !bIntersectsTile)
	{
		return spValueTile;
	}

	{
		// get the data input tile
		PYXPointer<PYXValueTile> spInputTile =
			m_spInputCoverage->getFieldTile(index, nRes, nFieldIndex);

		// If the whole tile is outside, and we are returning data that is outside
		// then we can return the whole tile of data.
		if (m_mode == knReturnDataOutside && !bIntersectsTile)
		{
			return spInputTile;
		}

		// If we got a null tile from the input we are done processing.
		if (!spInputTile)
		{
			return spInputTile;
		}

		// copy all values into our output tile.
		spValueTile = spInputTile->cloneFieldTile(0);
	}

	PYXValue nullValue;
	int nCellCount = spValueTile->getNumberOfCells();
	PYXPointer<PYXIterator> it = spValueTile->getIterator();
	for (int nCell = 0; nCell != nCellCount; ++nCell)
	{
		bool bIntersects = m_spMaskGeometry->intersects(PYXCell(it->getIndex()));
		it->next();
		bool bPassValue = (m_mode == knReturnDataInside  && bIntersects) ||
						  (m_mode == knReturnDataOutside && !bIntersects);
		if (!bPassValue)
		{
			spValueTile->setValue(nCell, 0, nullValue);
		}
	}

	return spValueTile;
}

/*!
Create the geometry. Since the geometry is not altered. Then it is 
fine to just clone our input geometry.
*/
void CoverageGeometryMaskProcess::createGeometry() const
{
	assert(m_spInputCoverage && "Input coverage not initalized properly.");
	m_spGeom = m_spInputCoverage->getGeometry()->clone();
	if (m_mode == knReturnDataInside)
	{
		m_spGeom = m_spGeom->intersection(*(m_spMaskGeometry.get()));
	}
	else
	{
		// TODO:  subtract the feature geometry from the input geometry here.
	}
}


std::string CoverageGeometryMaskProcess::getAttributeSchema() const
{
	std::string strXSD = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">";

	// add an enum type for operating mode.
	std::string strOperMode = "<xs:simpleType name=\"Mode\">"
		"<xs:restriction base=\"xs:string\">";
	for (int n = 0; n != sizeof(opStr)/sizeof(opStr[0]); ++n)
	{
		strOperMode += "<xs:enumeration value=\"";
		strOperMode += opStr[n];
		strOperMode += "\" />";
	}
	strOperMode += "</xs:restriction></xs:simpleType>";
	strXSD += strOperMode;

	strXSD+= 
		"<xs:element name=\"CoverageGeometryMaskProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"OperatingMode\" type=\"Mode\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Operating Mode</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}


