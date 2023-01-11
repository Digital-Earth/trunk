/******************************************************************************
feature_field_rasterizer2.cpp

begin		: 2017-03-01
copyright	: (C) 2017 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "feature_field_rasterizer2.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/data/tile_aggregator.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/math_utils.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// standard includes
#include <cassert>



// {446B0C8B-2ED0-4967-A479-7138B3022D4C}
PYXCOM_DEFINE_CLSID(FeatureFieldRasterizer2, 
0x446b0c8b, 0x2ed0, 0x4967, 0xa4, 0x79, 0x71, 0x38, 0xb3, 0x2, 0x2d, 0x4c);

PYXCOM_CLASS_INTERFACES(FeatureFieldRasterizer2, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureFieldRasterizer2, "Feature Field to Coverage 2", "A coverage that rasterizes its input features.", "Analysis/Features",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to rasterize")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<FeatureFieldRasterizer2> gTester;

}

FeatureFieldRasterizer2::FeatureFieldRasterizer2()
{	
	m_outputType = "double";
	m_fieldName = "";
	m_fieldIndex = -1;
	m_exactMatch = true;
	m_aggregate = "average";
}

FeatureFieldRasterizer2::~FeatureFieldRasterizer2()
{
}

void FeatureFieldRasterizer2::test()
{
	//TODO [shatzi]: add some unit testing!
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus FeatureFieldRasterizer2::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Transform Values " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the input coverage.
	m_inputFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();
	
	if (!m_inputFC)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input feature collection not set");
		return knFailedToInit;
	}

	// Get the input coverage definition.
	PYXPointer<PYXTableDefinition> spFeaturesDef = m_inputFC->getFeatureDefinition();

	m_fieldIndex = spFeaturesDef->getFieldIndex(m_fieldName);
	if (m_fieldIndex== -1)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("failed to find field " + m_fieldName);
		return knFailedToInit;
	}
	auto field = spFeaturesDef->getFieldDefinition(m_fieldIndex);

	// Create a new field definition, in which the greyscale fields are verified and converted to rgb.
	m_spCovDefn = PYXTableDefinition::create();
	
	if (m_outputType == "byte")
	{
		m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knUInt8, 1 );		
	}
	else if (m_outputType == "int")
	{
		m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knInt16, 1 );
	}
	else if (m_outputType == "float")
	{
		m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knFloat, 1 );
	} 
	else
	{
		m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knDouble, 1 );
	}
	return knInitialized;
}

//! Get the attributes in this process.
std::map<std::string, std::string> STDMETHODCALLTYPE FeatureFieldRasterizer2::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["ExactMatch"] = m_exactMatch ? "true" : "false";

	mapAttr["Output"] = m_outputType;

	mapAttr["FieldName"] = m_fieldName;

	mapAttr["Aggregate"] = m_aggregate;

	std::stringstream transformString;

	transformString << m_transform.size();
	for(auto & keyValue : m_transform)
	{
		transformString <<  ' ' << keyValue.first << ' ' << keyValue.second;
	}

	mapAttr["Transform"] = transformString.str();

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE FeatureFieldRasterizer2::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	std::string output;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Output",output);
	
	if (output == "byte" || output == "int" || output == "float")
	{
		m_outputType = output;
	} 
	else 
	{
		m_outputType = "double";
	}

	std::string exactMatch;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"ExactMatch",exactMatch);

	if (exactMatch.empty() || exactMatch == "0" || exactMatch == "false" || exactMatch == "False")
	{
		m_exactMatch = false;
	}
	else
	{
		m_exactMatch = true;
	}

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FieldName",m_fieldName);

	std::string transform;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Transform",transform);

	if (!transform.empty())
	{
		m_transform.clear();

		int size;

		std::stringstream transformString(transform);

		transformString >> size;

		for(auto i = 0;i<size;i++)
		{
			PYXValue from;
			PYXValue to;

			transformString >> from >> to;

			m_transform[from] = to;
		}
	}

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Aggregate",m_aggregate);	

	if (m_aggregate != "min" && m_aggregate != "max")
	{
		m_aggregate = "average";
	}
}


std::string STDMETHODCALLTYPE FeatureFieldRasterizer2::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"OutputType\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"byte\" />"
				"<xs:enumeration value=\"int\" />"
				"<xs:enumeration value=\"float\" />"
				"<xs:enumeration value=\"double\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:simpleType name=\"AggregateType\">"
			"<xs:restriction base=\"xs:string\">"
			    "<xs:enumeration value=\"min\" />"
				"<xs:enumeration value=\"max\" />"
				"<xs:enumeration value=\"average\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"FeatureFieldRasterizer2\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Output\" type=\"OutputType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Output</friendlyName>"
					"<description>Define transform output type.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"		
			"<xs:element name=\"FieldName\" type=\"string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>FieldName</friendlyName>"
					"<description>name of the field to raster.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"		
			  "<xs:element name=\"Transform\" type=\"string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Transform</friendlyName>"
					"<description>Define value transform.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"		
			  "<xs:element name=\"Aggregate\" type=\"AggregateType\" >"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Aggregate</friendlyName>"
					"<description>How to aggregate multiple features values</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
	          "<xs:element name=\"ExactMatch\" type=\"xs:boolean\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Exact Match</friendlyName>"
					"<description>When true, only transform values will be transform. When false, transform values will be treated as ranges.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue FeatureFieldRasterizer2::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	PYXPointer<PYXValueTile> valueTile = getFieldTile(index,index.getResolution(),nFieldIndex);

	if (valueTile)
	{
		bool init = false;
		return valueTile->getValue(0,0,&init);
	}
	return PYXValue();
}

PYXPointer<PYXValueTile> FeatureFieldRasterizer2::getFieldTile(
	const PYXIcosIndex& index,
	int nRes,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	PYXTile tile(index,nRes);

	// create the tile to return to the caller
	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

	if (m_aggregate == "min")
	{
		generateMinTile(tile,*spValueTile);
	}
	else if (m_aggregate == "max")
	{
		generateMaxTile(tile,*spValueTile);
	}
	else
	{
		generateAverageTile(tile,*spValueTile);
	}
	
	return spValueTile;
}



void FeatureFieldRasterizer2::generateAverageTile(
	const PYXTile & tile,
	PYXValueTile & valueTile) const
{

	TileAggregator<SumAndCountAggregator<double>> aggTile(tile);

	if (m_transform.empty())
	{
		FieldAggregator<SumAndCountAggregator<double>> fieldAgg(m_fieldIndex);
		TileFeaturesFieldAggregator<SumAndCountAggregator<double>, FieldAggregator<SumAndCountAggregator<double>>> tileFeaturesAgg (aggTile, fieldAgg);

		tileFeaturesAgg.visit(m_inputFC);
	} 
	else
	{
		TransformFieldAggregator<SumAndCountAggregator<double>> fieldAgg(m_fieldIndex, m_transform, m_exactMatch);
		TileFeaturesFieldAggregator<SumAndCountAggregator<double>, TransformFieldAggregator<SumAndCountAggregator<double>>> tileFeaturesAgg (aggTile, fieldAgg);

		tileFeaturesAgg.visit(m_inputFC);
	}

	auto cellCount = tile.getCellCount();

	PYXValue value = valueTile.getTypeCompatibleValue(0);

	for(auto nPos = 0; nPos < cellCount; nPos ++)
	{
		auto cell = aggTile.getCell(nPos);

		if (cell.count > 0)
		{
			value.setDouble(cell.sum/cell.count);
			valueTile.setValue(nPos,0,value);
		}
	}
}

void FeatureFieldRasterizer2::generateMinTile(
	const PYXTile & tile,
	PYXValueTile & valueTile) const
{

	TileAggregator<MinAggregator<double>> aggTile(tile);

	if (m_transform.empty())
	{
		FieldAggregator<MinAggregator<double>> fieldAgg(m_fieldIndex);
		TileFeaturesFieldAggregator<MinAggregator<double>, FieldAggregator<MinAggregator<double>>> tileFeaturesAgg (aggTile, fieldAgg);

		tileFeaturesAgg.visit(m_inputFC);
	} 
	else
	{
		TransformFieldAggregator<MinAggregator<double>> fieldAgg(m_fieldIndex, m_transform, m_exactMatch);
		TileFeaturesFieldAggregator<MinAggregator<double>, TransformFieldAggregator<MinAggregator<double>>> tileFeaturesAgg (aggTile, fieldAgg);

		tileFeaturesAgg.visit(m_inputFC);
	}

	auto cellCount = tile.getCellCount();

	PYXValue value = valueTile.getTypeCompatibleValue(0);

	for(auto nPos = 0; nPos < cellCount; nPos ++)
	{
		auto cell = aggTile.getCell(nPos);

		if (cell.hasValue)
		{
			value.setDouble(cell.value);
			valueTile.setValue(nPos,0,value);
		}
	}
}

void FeatureFieldRasterizer2::generateMaxTile(
	const PYXTile & tile,
	PYXValueTile & valueTile) const
{

	TileAggregator<MaxAggregator<double>> aggTile(tile);

	if (m_transform.empty())
	{
		FieldAggregator<MaxAggregator<double>> fieldAgg(m_fieldIndex);
		TileFeaturesFieldAggregator<MaxAggregator<double>, FieldAggregator<MaxAggregator<double>>> tileFeaturesAgg (aggTile, fieldAgg);

		tileFeaturesAgg.visit(m_inputFC);
	} 
	else
	{
		TransformFieldAggregator<MaxAggregator<double>> fieldAgg(m_fieldIndex, m_transform, m_exactMatch);
		TileFeaturesFieldAggregator<MaxAggregator<double>, TransformFieldAggregator<MaxAggregator<double>>> tileFeaturesAgg (aggTile, fieldAgg);

		tileFeaturesAgg.visit(m_inputFC);
	}

	auto cellCount = tile.getCellCount();

	PYXValue value = valueTile.getTypeCompatibleValue(0);

	for(auto nPos = 0; nPos < cellCount; nPos ++)
	{
		auto cell = aggTile.getCell(nPos);

		if (cell.hasValue)
		{
			value.setDouble(cell.value);
			valueTile.setValue(nPos,0,value);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void FeatureFieldRasterizer2::createGeometry() const
{
	assert(m_inputFC);
	PYXPointer<PYXGeometry> spGeometry = m_inputFC->getGeometry();
	if (!spGeometry)
	{
		m_spGeom = PYXEmptyGeometry::create();
	}
	else
	{
		m_spGeom = spGeometry->clone();
	}
}