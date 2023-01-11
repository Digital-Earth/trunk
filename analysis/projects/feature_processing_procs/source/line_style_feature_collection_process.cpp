/******************************************************************************
line_style_feature_collection_process.cpp

begin		: Aug 24, 2010
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "line_style_feature_collection_process.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_transform.h"

// standard includes
#include <cassert>

// {4C610229-D1EA-4307-9A53-E5BAA9FE8BB5}
PYXCOM_DEFINE_CLSID(LineStyleFeatureCollectionProcess, 
0x4c610229, 0xd1ea, 0x4307, 0x9a, 0x53, 0xe5, 0xba, 0xa9, 0xfe, 0x8b, 0xb5);

PYXCOM_CLASS_INTERFACES(LineStyleFeatureCollectionProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(LineStyleFeatureCollectionProcess, "Apply Line Style", "A process that applies a line style to its input feature collection.", "Styling/Features",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to apply a style to.")
IPROCESS_SPEC_END

const std::string LineStyleFeatureCollectionProcess::kstrLineColour = "line_colour";
const std::string LineStyleFeatureCollectionProcess::kstrLineWidth = "line_width";

//! Tester class
Tester<LineStyleFeatureCollectionProcess> gTester;

//! Test method
void LineStyleFeatureCollectionProcess::test()
{
	// TODO[kabiraman]: add unit test
}

//! Constructor
LineStyleFeatureCollectionProcess::LineStyleFeatureCollectionProcess()
{
	m_strLineColour = "uint8_t[3] 0 0 255";
	m_lineWidth = 1.0;
}

//! Destructor
LineStyleFeatureCollectionProcess::~LineStyleFeatureCollectionProcess()
{
}

///////////////////////////////////////////////////////////////////////////////
// IProcess
///////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus LineStyleFeatureCollectionProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_spInputFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();		

	if (m_spInputFC)
	{
		m_spGeom = m_spInputFC->getGeometry();

		if(m_spGeom)
		{
			m_bWritable = m_spInputFC->isWritable();
			m_strID = m_spInputFC->getID();
			updateStyle();

			return knInitialized;
		}
		else
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError(
				"LineStyleFeatureCollectionProcess: Input process' feature collection does not have a valid geometry.");			
		}
	}
	else
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"LineStyleFeatureCollectionProcess: Input process does not output a valid feature collection.");
	}

	return knFailedToInit;
	
}

void LineStyleFeatureCollectionProcess::updateStyle()
{
	PYXPointer<CSharpXMLDoc> styleDoc;
	if (m_spInputFC)
	{
		styleDoc = CSharpXMLDoc::create(m_spInputFC->getStyle());
	} 
	else 
	{
		styleDoc = CSharpXMLDoc::create("<style></style>");
	}
	
	if (!styleDoc->hasNode("/style"))
	{
		styleDoc->addChild("/","style");
	}

	if (!styleDoc->hasNode("/style/Line"))
	{
		styleDoc->addChild("/style","Line");
	}

	if (!styleDoc->hasNode("/style/Line/Colour"))
	{
		styleDoc->addChild("/style/Line","Colour");
	}
	styleDoc->setNodeText("/style/Line/Colour",m_strLineColour );

	if (!styleDoc->hasNode("/style/Line/Width"))
	{
		styleDoc->addChild("/style/Line","Width");
	}
	styleDoc->setNodeText("/style/Line/Width",StringUtils::toString(m_lineWidth));

	m_strStyle = styleDoc->getXMLString();
}

std::string STDMETHODCALLTYPE LineStyleFeatureCollectionProcess::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

	strXSD += 
		"<xs:simpleType name=\"colour\">"
			"<xs:restriction base=\"xs:string\">"
			"</xs:restriction>"
		"</xs:simpleType>"
		"<xs:simpleType name=\"icon\">"
			"<xs:restriction base=\"xs:string\">"
			"</xs:restriction>"
		"</xs:simpleType>"
		"<xs:element name=\"LineStyleFeatureCollectionProcess\">"
		  "<xs:complexType>"

		    "<xs:sequence>"
			  "<xs:element name=\"" + kstrLineColour + "\" type=\"colour\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Line Colour</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			  "<xs:element name=\"" + kstrLineWidth + "\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Line Width</friendlyName>"
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

std::map<std::string, std::string> LineStyleFeatureCollectionProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr[kstrLineColour] = m_strLineColour;
	mapAttr[kstrLineWidth] = StringUtils::toString(m_lineWidth);

	return mapAttr;
}

boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE LineStyleFeatureCollectionProcess::getOutput() const
{
	return static_cast<const IFeatureCollection*>(this);
}

boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE LineStyleFeatureCollectionProcess::getOutput()
{
	return static_cast<IFeatureCollection*>(this);
}

void LineStyleFeatureCollectionProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	std::map<std::string, std::string>::const_iterator it;

	it = mapAttr.find(kstrLineColour);
	if (it != mapAttr.end())
	{
		m_strLineColour = it->second;
	}
	else
	{
		m_strLineColour.clear();
	}

	it = mapAttr.find(kstrLineWidth);
	if (it != mapAttr.end())
	{
		m_lineWidth = StringUtils::fromString<double>(it->second);
	}
	else
	{
		m_lineWidth = 1.0;
	}

	updateStyle();
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> LineStyleFeatureCollectionProcess::getIterator() const
{
	return m_spInputFC->getIterator();
}

PYXPointer<FeatureIterator> LineStyleFeatureCollectionProcess::getIterator(const PYXGeometry& geometry) const
{
	return m_spInputFC->getIterator(geometry);
}


std::vector<FeatureStyle> LineStyleFeatureCollectionProcess::getFeatureStyles() const
{
	return m_spInputFC->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> LineStyleFeatureCollectionProcess::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFC->getFeature(strFeatureID);	
}

PYXPointer<const PYXTableDefinition> LineStyleFeatureCollectionProcess::getFeatureDefinition() const
{
	return m_spInputFC->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> LineStyleFeatureCollectionProcess::getFeatureDefinition()
{
	return m_spInputFC->getFeatureDefinition();
}

bool LineStyleFeatureCollectionProcess::canRasterize() const
{
	return m_spInputFC->canRasterize();
}
