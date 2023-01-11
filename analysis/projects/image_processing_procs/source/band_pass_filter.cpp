/******************************************************************************
band_pass_filter.cpp

begin      : 05/04/2008 9:02:17 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "band_pass_filter.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <algorithm>
#include <cassert>

// {70000EED-659A-4903-B54D-13A22AF5A5EC}
PYXCOM_DEFINE_CLSID(BandPassFilter, 
0x70000eed, 0x659a, 0x4903, 0xb5, 0x4d, 0x13, 0xa2, 0x2a, 0xf5, 0xa5, 0xec);
PYXCOM_CLASS_INTERFACES(BandPassFilter,
						IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BandPassFilter, "Band Pass Filter", "Only allow a certain range of values through the filter.", "Analysis/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The coverage to mask.")
IPROCESS_SPEC_END

namespace
{
const int knDefaultRes = 13;

const char* const opStr[] =
{
	"no_operation",
	"less_than",
	"less_than_or_equal",
	"equals",
	"not_equal",
	"greater_than",
	"greater_than_or_equal"
};

BandPassFilter::Op getOpFromStr(const char* str)
{
	for (int n = 0; n != sizeof(opStr)/sizeof(opStr[0]); ++n)
	{
		if (strcmp(opStr[n], str) == 0)
		{
			return static_cast<BandPassFilter::Op>(n);
		}
	}

	return BandPassFilter::knNoop;
}

bool passes(const PYXValue& v1, const PYXValue& v2, BandPassFilter::Op op)
{
	if (op == BandPassFilter::knEqualTo)
	{
		return v1 == v2;
	}

	if (op == BandPassFilter::knNotEqualTo)
	{
		return v1 != v2;
	}

	if (v2.isArray())
	{
		const int nCount = std::min(v1.getArraySize(), v2.getArraySize());
		for (int n = 0; n != nCount; ++n)
		{
			if (!passes(v1.getValue(n), v2.getValue(n), op))
			{
				return false;
			}
		}
		return true;
	}

	double fCmp;

	if (v2.isString())
	{
		fCmp = strcmp(v1.getString().c_str(), v2.getString().c_str());
	}
	else
	{
		fCmp = v1.getDouble() - v2.getDouble();
	}

	switch (op)
	{
		case BandPassFilter::knLess:
			return fCmp < 0;
		case BandPassFilter::knLessEqual:
			return fCmp <= 0;
		case BandPassFilter::knGreater:
			return fCmp > 0;
		case BandPassFilter::knGreaterEqual:
			return fCmp >= 0;
		default:
			return false; // should never happen
	}

	return false; // should never happen
}

}

BandPassFilter::BandPassFilter() :
	m_op1(knNoop),
	m_op2(knNoop)
{
}

BandPassFilter::~BandPassFilter()
{
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue BandPassFilter::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);
	if (nFieldIndex != 0)
	{
		PYXTHROW(PYXException, "Field index cannot be less then zero");
	}

	PYXValue controlValue = m_spCov->getCoverageValue(index, nFieldIndex);
	if (!controlValue.isNull())
	{
		if ((m_op1 && !passes(controlValue, m_v1, m_op1)) ||
			(m_op2 && !passes(controlValue, m_v2, m_op2)))
		{
			// Failed to pass filters so nullify.
			controlValue.swap(PYXValue());
		}
	}
	return controlValue;
}

PYXPointer<PYXValueTile> STDMETHODCALLTYPE BandPassFilter::getFieldTile(	const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// get the input field tile
	PYXPointer<PYXValueTile> spInputTile = 
		m_spCov->getFieldTile(index, nRes, nFieldIndex);

	// Return null tiles right away.
	if (!spInputTile)
	{
		return spInputTile;
	}

	// create a field tile to return
	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spOutputTile = PYXValueTile::create(index, nRes, spCovDefn);

	// fill in the output tile
	int nCellCount = spInputTile->getTile().getCellCount();
	PYXValue v;
	for (int nIndexOffset = 0; nIndexOffset < nCellCount; ++nIndexOffset)
	{
		v.swap(spInputTile->getValue(nIndexOffset, 0));
		if ((!m_op1 || passes(v, m_v1, m_op1)) &&
			(!m_op2 || passes(v, m_v2, m_op2)))
		{
			// Passed filters.
			spOutputTile->setValue(nIndexOffset, nFieldIndex, v);
		}
	}
	return spOutputTile;
}

/*!
Create the geometry. Since the geometry is not altered. Then it is 
fine to just clone our input geometry.
*/
void BandPassFilter::createGeometry() const
{
	assert(m_spCov && "Input coverage not initalized properly.");
	m_spGeom = m_spCov->getGeometry()->clone();
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE BandPassFilter::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["value_1_operation"] = opStr[m_op1];
	mapAttr["value_1"] = StringUtils::toString(m_v1);

	mapAttr["value_2_operation"] = opStr[m_op2];
	mapAttr["value_2"] = StringUtils::toString(m_v2);

	return mapAttr;
}

void STDMETHODCALLTYPE BandPassFilter::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;

	m_op1 = knNoop;
	it = mapAttr.find("value_1_operation");
	if (it != mapAttr.end())
	{
		m_op1 = getOpFromStr(it->second.c_str());
	}

	m_v1.swap(PYXValue());
	it = mapAttr.find("value_1");
	if (it != mapAttr.end())
	{
		m_v1 = StringUtils::fromString<PYXValue>(it->second);
	}

	m_op2 = knNoop;
	it = mapAttr.find("value_2_operation");
	if (it != mapAttr.end())
	{
		m_op2 = getOpFromStr(it->second.c_str());
	}

	m_v2.swap(PYXValue());
	it = mapAttr.find("value_2");
	if (it != mapAttr.end())
	{
		m_v2 = StringUtils::fromString<PYXValue>(it->second);
	}
}

std::string BandPassFilter::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">";

    // add the operation type
    strXSD +=  "<xs:simpleType name=\"operationType\"><xs:restriction base=\"xs:string\">";
	for (int n = 0; n != sizeof(opStr)/sizeof(opStr[0]); ++n)
	{
		strXSD += "<xs:enumeration value=\"";
		strXSD += opStr[n];
		strXSD += "\" />";
	}
	strXSD += "</xs:restriction></xs:simpleType>";

	// build the actual schema
	strXSD += 
		  "<xs:element name=\"BandPassFilter\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"value_1_operation\" type=\"operationType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Value 1 Operation</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"value_1\" type=\"xs:string\" nillable=\"true\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Value 1</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"value_2_operation\" type=\"operationType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Value 2 Operation</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"value_2\" type=\"xs:string\" nillable=\"true\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Value 2</friendlyName>"
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

IProcess::eInitStatus BandPassFilter::initImpl()
{
	m_strID = "Band Pass Filter: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	m_spCov = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &m_spCov);
	if (!m_spCov)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Failed to get the coverage from the parameter.");
		return knFailedToInit;
	}

	PYXValue::eType nType = m_spCov->getCoverageDefinition()->getFieldDefinition(0).getType();

	if (PYXValue::isNumeric(nType))
	{
		if (m_op1 && !PYXValue::isNumeric(m_v1.getArrayType()))
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Value 1 numeric type doesn't match coverage definition.");
			return knFailedToInit;
		}
		if (m_op2 && !PYXValue::isNumeric(m_v2.getArrayType()))
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Value 2 numeric type doesn't match coverage definition.");
			return knFailedToInit;
		}
	}
	else
	{
		if (m_op1 && m_v1.getType() != nType)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Value 1 non numeric type doesn't match coverage definition.");
			return knFailedToInit;
		}
		if (m_op2 && m_v2.getType() != nType)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Value 2 non numeric type doesn't match coverage definition.");
			return knFailedToInit;
		}
	}
	m_bDirty = true;
	return knInitialized;
}
