/******************************************************************************
styled_feature_collection_process.cpp

begin		: June 11, 2008
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "styled_feature_collection_process.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_transform.h"

// standard includes
#include <cassert>

// {70CC2EDF-8F7B-4e44-BD67-CA480FE2059A}
PYXCOM_DEFINE_CLSID(StyledFeatureCollectionProcess, 
0x70cc2edf, 0x8f7b, 0x4e44, 0xbd, 0x67, 0xca, 0x48, 0xf, 0xe2, 0x5, 0x9a);

PYXCOM_CLASS_INTERFACES(StyledFeatureCollectionProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(StyledFeatureCollectionProcess, "Apply Style", "A process that applies a style to its input feature collection.", "Development/Old",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to apply a style to.")
IPROCESS_SPEC_END

const std::string StyledFeatureCollectionProcess::kstrLineColour = "line_colour";
const std::string StyledFeatureCollectionProcess::kstrIcon = "icon";

//! Tester class
Tester<StyledFeatureCollectionProcess> gTester;

//! Test method
void StyledFeatureCollectionProcess::test()
{
	// TODO[kabiraman]: add unit test
}

//! Constructor
StyledFeatureCollectionProcess::StyledFeatureCollectionProcess()
{
	m_strLineColour = "uint8_t[3] 0 0 255";
}

//! Destructor
StyledFeatureCollectionProcess::~StyledFeatureCollectionProcess()
{
}

///////////////////////////////////////////////////////////////////////////////
// IProcess
///////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus StyledFeatureCollectionProcess::initImpl()
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
				"StyledFeatureCollectionProcess: Input process' feature collection does not have a valid geometry.");			
		}
	}
	else
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"StyledFeatureCollectionProcess: Input process does not output a valid feature collection.");
	}

	return knFailedToInit;
	
}

void StyledFeatureCollectionProcess::updateStyle()
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

	if (!styleDoc->hasNode("/style/LineColour"))
	{
		styleDoc->addChild("/style","LineColour");
	}
	styleDoc->setNodeText("/style/LineColour",m_strLineColour );

	if (!styleDoc->hasNode("/style/Icon"))
	{
		styleDoc->addChild("/style","Icon");
	}
	if (!styleDoc->hasNode("/style/Icon/Bitmap"))
	{
		styleDoc->addChild("/style/Icon","Bitmap");
	}
	styleDoc->setNodeText("/style/Icon/Bitmap",m_strIconID );

	m_strStyle = styleDoc->getXMLString();
}

std::string STDMETHODCALLTYPE StyledFeatureCollectionProcess::getAttributeSchema() const
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
		"<xs:element name=\"StyledFeatureCollectionProcess\">"
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
			  "<xs:element name=\"" + kstrIcon + "\" type=\"icon\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Icon</friendlyName>"
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

std::map<std::string, std::string> StyledFeatureCollectionProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr[kstrLineColour] = m_strLineColour;
	mapAttr[kstrIcon] = m_strIconID;

	return mapAttr;
}

boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE StyledFeatureCollectionProcess::getOutput() const
{
	return static_cast<const IFeatureCollection*>(this);
}

boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE StyledFeatureCollectionProcess::getOutput()
{
	return static_cast<IFeatureCollection*>(this);
}

void StyledFeatureCollectionProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
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

	it = mapAttr.find(kstrIcon);
	if (it != mapAttr.end())
	{
		m_strIconID = it->second;
	}
	else
	{
		m_strLineColour.clear();
	}

	updateStyle();
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> StyledFeatureCollectionProcess::getIterator() const
{
	return m_spInputFC->getIterator();
}

PYXPointer<FeatureIterator> StyledFeatureCollectionProcess::getIterator(const PYXGeometry& geometry) const
{
	return m_spInputFC->getIterator(geometry);
}


std::vector<FeatureStyle> StyledFeatureCollectionProcess::getFeatureStyles() const
{
	return m_spInputFC->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> StyledFeatureCollectionProcess::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFC->getFeature(strFeatureID);	
}

PYXPointer<const PYXTableDefinition> StyledFeatureCollectionProcess::getFeatureDefinition() const
{
	return m_spInputFC->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> StyledFeatureCollectionProcess::getFeatureDefinition()
{
	return m_spInputFC->getFeatureDefinition();
}

bool StyledFeatureCollectionProcess::canRasterize() const
{
	return m_spInputFC->canRasterize();
}
