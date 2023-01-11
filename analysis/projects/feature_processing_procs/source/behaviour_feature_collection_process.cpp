/******************************************************************************
behaviour_feature_collection_process.cpp

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "behaviour_feature_collection_process.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_identity.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_transform.h"

// standard includes
#include <cassert>


// {8A168956-0431-493f-B0E1-8354F65CFC82}
PYXCOM_DEFINE_CLSID(BehaviourFeatureCollectionProcess, 
0x8a168956, 0x431, 0x493f, 0xb0, 0xe1, 0x83, 0x54, 0xf6, 0x5c, 0xfc, 0x82);

PYXCOM_CLASS_INTERFACES(BehaviourFeatureCollectionProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BehaviourFeatureCollectionProcess, "Apply Behaviour Style", "A process that applies an Behaviour Scripts to its input feature collection.", "Styling/Features",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to apply an Behaviour style to.")
	//IPROCESS_SPEC_PARAMETER(IBitmap::iid, 1, 1, "Input Icon Bitmap", "a bitmap to apply for icons")
IPROCESS_SPEC_END

const std::string BehaviourFeatureCollectionProcess::kstrOnClick = "on_click";
const std::string BehaviourFeatureCollectionProcess::kstrOnDoubleClick = "on_double_click";
const std::string BehaviourFeatureCollectionProcess::kstrOnMouseEnter = "on_mouse_enter";
const std::string BehaviourFeatureCollectionProcess::kstrOnMouseLeave = "on_mouse_leave";

//! Tester class
Tester<BehaviourFeatureCollectionProcess> gTester;

//! Test method
void BehaviourFeatureCollectionProcess::test()
{
	// TODO[shatzi]: add unit test
}

//! Constructor
BehaviourFeatureCollectionProcess::BehaviourFeatureCollectionProcess()
{	
}

//! Destructor
BehaviourFeatureCollectionProcess::~BehaviourFeatureCollectionProcess()
{
}

///////////////////////////////////////////////////////////////////////////////
// IProcess
///////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus BehaviourFeatureCollectionProcess::initImpl()
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
				"BehaviourFeatureCollectionProcess: Input process' feature collection does not have a valid geometry.");			
		}
	}
	else
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"BehaviourFeatureCollectionProcess: Input process does not output a valid feature collection.");
	}

	return knFailedToInit;
	
}


void BehaviourFeatureCollectionProcess::updateStyle()
{
	PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(m_spInputFC->getStyle());

	if (!styleDoc->hasNode("/style"))
	{
		styleDoc->addChild("/","style");
	}
	
	if (m_onClickScript.size() > 0)
	{
		if (!styleDoc->hasNode("/style/on_click"))
		{
			styleDoc->addChild("/style","on_click");
		}
		styleDoc->setNodeText("/style/on_click",m_onClickScript);
	}

	if (m_onDoubleClickScript.size() > 0)
	{
		if (!styleDoc->hasNode("/style/on_double_click"))
		{
			styleDoc->addChild("/style","on_double_click");
		}
		styleDoc->setNodeText("/style/on_double_click",m_onDoubleClickScript);
	}

	if (m_onMouseEnterScript.size() > 0)
	{
		if (!styleDoc->hasNode("/style/on_mouse_enter"))
		{
			styleDoc->addChild("/style","on_mouse_enter");
		}
		styleDoc->setNodeText("/style/on_mouse_enter",m_onMouseEnterScript);
	}

	if (m_onMouseLeaveScript.size() > 0)
	{
		if (!styleDoc->hasNode("/style/on_mouse_leave"))
		{
			styleDoc->addChild("/style","on_mouse_leave");
		}
		styleDoc->setNodeText("/style/on_mouse_leave",m_onMouseLeaveScript);
	}

	m_strStyle = styleDoc->getXMLString();
}

std::string STDMETHODCALLTYPE BehaviourFeatureCollectionProcess::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

	strXSD += 
		"<xs:element name=\"BehaviourFeatureCollectionProcess\">"
		  "<xs:complexType>"
			  "<xs:sequence>"

			      "<xs:element name=\"on_click\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>On Click</friendlyName>"
						"<description>C# Script to be executed on feature annotation click.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			      "<xs:element name=\"on_double_click\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>On Double Click</friendlyName>"
						"<description>C# Script to be executed on feature annotation double click.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			      "<xs:element name=\"on_mouse_enter\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>On Mouse Enter</friendlyName>"
						"<description>C# Script to be executed when the mouse start to hover above feature annotation.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			      "<xs:element name=\"on_mouse_leave\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>On Mouse Leave</friendlyName>"
						"<description>C# Script to be executed when the mouse move and leave feature annotation.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			  "</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}

std::map<std::string, std::string> BehaviourFeatureCollectionProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr[kstrOnClick] = m_onClickScript;
	mapAttr[kstrOnDoubleClick] = m_onDoubleClickScript;
	mapAttr[kstrOnMouseEnter] = m_onMouseEnterScript;
	mapAttr[kstrOnMouseLeave] = m_onMouseLeaveScript;

	return mapAttr;
}

boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE BehaviourFeatureCollectionProcess::getOutput() const
{
	return static_cast<const IFeatureCollection*>(this);
}

boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE BehaviourFeatureCollectionProcess::getOutput()
{
	return static_cast<IFeatureCollection*>(this);
}

void BehaviourFeatureCollectionProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{		
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,kstrOnClick,m_onClickScript);	
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,kstrOnDoubleClick,m_onDoubleClickScript);	
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,kstrOnMouseEnter,m_onMouseEnterScript);	
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,kstrOnMouseLeave,m_onMouseLeaveScript);	

	if (m_spInputFC)
	{
		updateStyle();
	}
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> BehaviourFeatureCollectionProcess::getIterator() const
{
	return m_spInputFC->getIterator();
}

PYXPointer<FeatureIterator> BehaviourFeatureCollectionProcess::getIterator(const PYXGeometry& geometry) const
{
	return m_spInputFC->getIterator(geometry);
}


std::vector<FeatureStyle> BehaviourFeatureCollectionProcess::getFeatureStyles() const
{
	return m_spInputFC->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> BehaviourFeatureCollectionProcess::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFC->getFeature(strFeatureID);	
}

PYXPointer<const PYXTableDefinition> BehaviourFeatureCollectionProcess::getFeatureDefinition() const
{
	return m_spInputFC->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> BehaviourFeatureCollectionProcess::getFeatureDefinition()
{
	return m_spInputFC->getFeatureDefinition();
}

bool BehaviourFeatureCollectionProcess::canRasterize() const
{
	return m_spInputFC->canRasterize();
}
