/******************************************************************************
are_style_feature_collection_process.cpp

begin		: Nov 24, 2010
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "area_style_feature_collection_process.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_transform.h"

// standard includes
#include <cassert>

// {A7B8FC7A-6041-4a15-BD97-7F4A976D91AD}
PYXCOM_DEFINE_CLSID(AreaStyleFeatureCollectionProcess, 
0xa7b8fc7a, 0x6041, 0x4a15, 0xbd, 0x97, 0x7f, 0x4a, 0x97, 0x6d, 0x91, 0xad);

PYXCOM_CLASS_INTERFACES(AreaStyleFeatureCollectionProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(AreaStyleFeatureCollectionProcess, "Apply Area Style", "A process that applies a area style to its input feature collection.", "Styling/Features",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to apply a style to.")
IPROCESS_SPEC_END

const std::string AreaStyleFeatureCollectionProcess::kstrAreaColour = "area_colour";

//! Tester class
Tester<AreaStyleFeatureCollectionProcess> gTester;

//! Test method
void AreaStyleFeatureCollectionProcess::test()
{
	// TODO[kabiraman]: add unit test
}

//! Constructor
AreaStyleFeatureCollectionProcess::AreaStyleFeatureCollectionProcess()
{
	m_strAreaColour = "uint8_t[3] 127 127 255";
	m_areaOpacity = 60;
}

//! Destructor
AreaStyleFeatureCollectionProcess::~AreaStyleFeatureCollectionProcess()
{
}

///////////////////////////////////////////////////////////////////////////////
// IProcess
///////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus AreaStyleFeatureCollectionProcess::initImpl()
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
				"AreaStyleFeatureCollectionProcess: Input process' feature collection does not have a valid geometry.");			
		}
	}
	else
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"AreaStyleFeatureCollectionProcess: Input process does not output a valid feature collection.");
	}

	return knFailedToInit;
	
}

void AreaStyleFeatureCollectionProcess::updateStyle()
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

	if (!styleDoc->hasNode("/style/Area"))
	{
		styleDoc->addChild("/style","Area");
	}

	if (!styleDoc->hasNode("/style/Area/Colour"))
	{
		styleDoc->addChild("/style/Area","Colour");
	}
	styleDoc->setNodeText("/style/Area/Colour",m_strAreaColour );

	if (!styleDoc->hasNode("/style/Area/Opacity"))
	{
		styleDoc->addChild("/style/Area","Opacity");
	}
	styleDoc->setNodeText("/style/Area/Opacity",StringUtils::toString(m_areaOpacity));

	m_strStyle = styleDoc->getXMLString();
}

std::string STDMETHODCALLTYPE AreaStyleFeatureCollectionProcess::getAttributeSchema() const
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
		"<xs:element name=\"AreaStyleFeatureCollectionProcess\">"
		  "<xs:complexType>"

		    "<xs:sequence>"
			  "<xs:element name=\"" + kstrAreaColour + "\" type=\"colour\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Area Colour</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			  "<xs:element name=\"AreaOpacity\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Area Opactiy</friendlyName>"
					"<description>Specify the opactiy of the area fill color (between 0 to 100).</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}

std::map<std::string, std::string> AreaStyleFeatureCollectionProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr[kstrAreaColour] = m_strAreaColour;
	mapAttr["AreaOpacity"] = StringUtils::toString(m_areaOpacity);

	return mapAttr;
}

boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE AreaStyleFeatureCollectionProcess::getOutput() const
{
	return static_cast<const IFeatureCollection*>(this);
}

boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE AreaStyleFeatureCollectionProcess::getOutput()
{
	return static_cast<IFeatureCollection*>(this);
}

void AreaStyleFeatureCollectionProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,kstrAreaColour,m_strAreaColour);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"AreaOpacity",int,m_areaOpacity);

	updateStyle();
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> AreaStyleFeatureCollectionProcess::getIterator() const
{
	return m_spInputFC->getIterator();
}

PYXPointer<FeatureIterator> AreaStyleFeatureCollectionProcess::getIterator(const PYXGeometry& geometry) const
{
	return m_spInputFC->getIterator(geometry);
}


std::vector<FeatureStyle> AreaStyleFeatureCollectionProcess::getFeatureStyles() const
{
	return m_spInputFC->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> AreaStyleFeatureCollectionProcess::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFC->getFeature(strFeatureID);	
}

PYXPointer<const PYXTableDefinition> AreaStyleFeatureCollectionProcess::getFeatureDefinition() const
{
	return m_spInputFC->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> AreaStyleFeatureCollectionProcess::getFeatureDefinition()
{
	return m_spInputFC->getFeatureDefinition();
}

bool AreaStyleFeatureCollectionProcess::canRasterize() const
{
	return m_spInputFC->canRasterize();
}
