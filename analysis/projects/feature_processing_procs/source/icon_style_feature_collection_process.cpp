/******************************************************************************
styled_feature_collection_process.cpp

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "icon_style_feature_collection_process.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_identity.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>


// {705C8518-FDDD-4946-840D-96515B9D29BD}
PYXCOM_DEFINE_CLSID(IconStyleFeatureCollectionProcess, 
0x705c8518, 0xfddd, 0x4946, 0x84, 0xd, 0x96, 0x51, 0x5b, 0x9d, 0x29, 0xbd);

PYXCOM_CLASS_INTERFACES(IconStyleFeatureCollectionProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(IconStyleFeatureCollectionProcess, "Apply Icon Style", "A process that applies an Icon Style to its input feature collection.", "Styling/Features",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to apply an Icon style to.")
	//IPROCESS_SPEC_PARAMETER(IBitmap::iid, 1, 1, "Input Icon Bitmap", "a bitmap to apply for icons")
IPROCESS_SPEC_END

const std::string IconStyleFeatureCollectionProcess::kstrLineColour = "line_colour";
const std::string IconStyleFeatureCollectionProcess::kstrIcon = "icon";

//! Tester class
Tester<IconStyleFeatureCollectionProcess> gTester;

//! Test method
void IconStyleFeatureCollectionProcess::test()
{
	// TODO[shatzi]: add unit test
}

//! Constructor
IconStyleFeatureCollectionProcess::IconStyleFeatureCollectionProcess()
{
	m_bitmapPipelineDefinition = "";
	m_iconIndex = -1;
	m_iconScaling = 100;
	m_iconColour = "uint8_t[3] 255 255 255";

	m_fontStyle = "Regular";
	m_fontColor = "White";
	m_fontFamily = "Arial";
	m_fontSize = 15;
}

//! Destructor
IconStyleFeatureCollectionProcess::~IconStyleFeatureCollectionProcess()
{
}

///////////////////////////////////////////////////////////////////////////////
// IProcess
///////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus IconStyleFeatureCollectionProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_spInputFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();		

	//getParameter(1)->getValue(0)->getOutput()->QueryInterface(
	//	IBitmap::iid, (void**) &m_spBitmap);	

	if (m_bitmapPipelineDefinition.size() > 0)
	{
		 boost::intrusive_ptr<IProcess> process = PipeManager::readPipelineFromString(m_bitmapPipelineDefinition.c_str(),m_bitmapPipelineDefinition.size());

		 m_spBitmap = dynamic_cast<IBitmap*>(process.get());
				 
		 if (m_spBitmap)
		 {
			 //if we managed to load a IBitmap process, then initialize it
			 if (process->initProc(true) != IProcess::knInitialized)
			 {
				 //try to download files over pyxnet
				 FileNotificationManager::getPipelineFilesDownloadNeededNotifier().notify(PipelineFilesEvent::create(process));

				 //try to reinitalize the pipeline
				 process->initProc(true);
			 }
		 } 
		 else 
		 {			 
			 //if we failed - then we don't know what Icon we need. make it no Icon at all
			 //TODO[shatzi]: maybe auto select "Missing Icon?"
			 m_bitmapPipelineDefinition = "";
			 m_iconIndex = -1;
		 }
	}
	
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
				"IconStyleFeatureCollectionProcess: Input process' feature collection does not have a valid geometry.");			
		}
	}
	else
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"IconStyleFeatureCollectionProcess: Input process does not output a valid feature collection.");
	}

	return knFailedToInit;
	
}

std::string STDMETHODCALLTYPE IconStyleFeatureCollectionProcess::getAttributeSchema() const
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
		"<xs:simpleType name=\"AppearanceMode\">"
		  "<xs:restriction base=\"xs:string\">"
		  	"<xs:enumeration value=\"Always\" />"
			"<xs:enumeration value=\"OnMouseOver\" />"
		  "</xs:restriction>"
	    "</xs:simpleType>"
		"<xs:element name=\"IconStyleFeatureCollectionProcess\">"
		  "<xs:complexType>"
			  "<xs:sequence>"

			      "<xs:element name=\"BitmapPipeline\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>IBitmap Pipeline Definition</friendlyName>"
						"<description>The definition of the pipeline that generate the Icon Library Bitmaps.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"
		  
			      "<xs:element name=\"IconIndex\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Icon Index</friendlyName>"
						"<description>The Icon Index inside the Icon Library chosen by the user.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"				  

				  "<xs:element name=\"TextField\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Text Field</friendlyName>"
						"<description>Field name to use as a Text Note beside the icon.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"TextAlignment\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Text Aligment</friendlyName>"
						"<description>Alginment to use for the Text near the icon.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"IconScale\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Icon Scaling</friendlyName>"
						"<description>Icon Scaling to apply to the selected icon (in percent).</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				   "<xs:element name=\"IconColour\" type=\"colour\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Icon Color</friendlyName>"
						"<description>Specify the color for the features' icon.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"FontColor\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Font Color</friendlyName>"
						"<description>Font Color to use for the Icon Text.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"FontFamily\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Font Family</friendlyName>"
						"<description>Font Family to use for the Icon Text.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"FontSize\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Font Size</friendlyName>"
						"<description>Font Size to use for the Icon Text (in points).</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"FontStyle\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Font Style</friendlyName>"
						"<description>Font Style to use for the Icon Text.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"TextAppearanceMode\" type=\"AppearanceMode\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Text Appearance Mode</friendlyName>"
						"<description>Specify the appearance of the text near the icon.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			  "</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}

std::map<std::string, std::string> IconStyleFeatureCollectionProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	
	mapAttr["BitmapPipeline"] = m_bitmapPipelineDefinition;
	if (m_iconIndex>=0)
	{
		mapAttr["IconIndex"] = StringUtils::toString(m_iconIndex);
	} 
	else 
	{
		mapAttr["IconIndex"] = "-1";
	}

	if (m_iconScaling!=100)
	{	
		mapAttr["IconScale"] = StringUtils::toString(m_iconScaling);
	}

	mapAttr["IconColour"] = m_iconColour;

	if (!m_textField.empty())
	{
		mapAttr["TextField"] = m_textField;
	}

	if (!m_textAlignment.empty())
	{
		mapAttr["TextAlignment"] = m_textAlignment;
	}

	mapAttr["FontFamily"] = m_fontFamily;
	mapAttr["FontStyle"] = m_fontStyle;
	mapAttr["FontColor"] = m_fontColor;
	mapAttr["FontSize"] = StringUtils::toString(m_fontSize);

	mapAttr["TextAppearanceMode"] = m_textAppearanceMode;

	return mapAttr;
}

boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE IconStyleFeatureCollectionProcess::getOutput() const
{
	return static_cast<const IFeatureCollection*>(this);
}

boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE IconStyleFeatureCollectionProcess::getOutput()
{
	return static_cast<IFeatureCollection*>(this);
}

void IconStyleFeatureCollectionProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{	
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"BitmapPipeline",m_bitmapPipelineDefinition);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"IconIndex",int,m_iconIndex);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"IconScale",int,m_iconScaling);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"IconColour",m_iconColour);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TextField",m_textField);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TextAlignment",m_textAlignment);

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FontFamily",m_fontFamily);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FontColor",m_fontColor);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FontStyle",m_fontStyle);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"FontSize",int,m_fontSize);

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TextAppearanceMode",m_textAppearanceMode);

	try
	{
		StringUtils::fromString<PYXValue>(m_iconColour);
	}
	catch(...)
	{
		m_iconColour = "uint8_t[3] 255 255 255";
	}

	m_initState = knNeedsInit;
}

void IconStyleFeatureCollectionProcess::updateStyle()
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

	if (m_iconIndex != -1 || !m_textField.empty())
	{
		if (!styleDoc->hasNode("/style/Icon"))
		{
			styleDoc->addChild("/style","Icon");
		}
	}

	if (m_iconIndex != -1)
	{
		if (!styleDoc->hasNode("/style/Icon/Bitmap"))
		{
			styleDoc->addChild("/style/Icon","Bitmap");
		}

		if (m_spBitmap)
		{		
			try
			{
				styleDoc->setInnerXMLString("/style/Icon/Bitmap",m_spBitmap->getBitmapDefinition() );
			}
			catch(PYXException& ex)
			{
				TRACE_ERROR("Failed to extract bitmap definition: " << ex.getFullErrorString());
				styleDoc->setInnerXMLString("/style/Icon/Bitmap","<missing-icon></missing-icon>");
			}
		}
		else 
		{
			//old style demo icons
			styleDoc->setInnerXMLString("/style/Icon/Bitmap",StringUtils::toString(m_iconIndex));
		}

		if (m_iconScaling!=100)
		{
			if (!styleDoc->hasNode("/style/Icon/Scale"))
			{
				styleDoc->addChild("/style/Icon","Scale");
			}
			styleDoc->addAttribute("/style/Icon/Scale","Width",StringUtils::toString(m_iconScaling/100.0));
			styleDoc->addAttribute("/style/Icon/Scale","Height",StringUtils::toString(m_iconScaling/100.0));
		}

		if (!styleDoc->hasNode("/style/Icon/Colour"))
		{
			styleDoc->addChild("/style/Icon","Colour");
		}
		styleDoc->setNodeText("/style/Icon/Colour", m_iconColour );
	}

	if (!m_textField.empty())
	{
		if (!styleDoc->hasNode("/style/Icon/TextField"))
		{
			styleDoc->addChild("/style/Icon","TextField");
		}
		
		styleDoc->setNodeText("/style/Icon/TextField",m_textField);

		if (!m_textAlignment.empty())
		{
			if (!styleDoc->hasNode("/style/Icon/TextAlign"))
			{
				styleDoc->addChild("/style/Icon","TextAlign");
			}
			if (m_textAlignment == "Top")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","VAlign","Top");
			}
			else if (m_textAlignment == "Top Left")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","VAlign","Top");
				styleDoc->addAttribute("/style/Icon/TextAlign","Align","Left");
			}
			else if (m_textAlignment == "Top Right")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","VAlign","Top");
				styleDoc->addAttribute("/style/Icon/TextAlign","Align","Right");
			}
			else if (m_textAlignment == "Bottom")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","VAlign","Bottom");
			}
			else if (m_textAlignment == "Bottom Left")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","VAlign","Bottom");
				styleDoc->addAttribute("/style/Icon/TextAlign","Align","Left");
			}
			else if (m_textAlignment == "Bottom Right")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","VAlign","Bottom");
				styleDoc->addAttribute("/style/Icon/TextAlign","Align","Right");
			}
			else if (m_textAlignment == "Left")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","Align","Left");
			}
			else if (m_textAlignment == "Right")
			{
				styleDoc->addAttribute("/style/Icon/TextAlign","Align","Right");
			}
		}

		if (!styleDoc->hasNode("/style/Icon/FontFamily"))
		{
			styleDoc->addChild("/style/Icon","FontFamily");
		}

		styleDoc->setNodeText("/style/Icon/FontFamily",m_fontFamily);

		if (!styleDoc->hasNode("/style/Icon/FontColor"))
		{
			styleDoc->addChild("/style/Icon","FontColor");
		}

		styleDoc->setNodeText("/style/Icon/FontColor",m_fontColor);

		if (!styleDoc->hasNode("/style/Icon/FontStyle"))
		{
			styleDoc->addChild("/style/Icon","FontStyle");
		}

		styleDoc->setNodeText("/style/Icon/FontStyle",m_fontStyle);

		if (!styleDoc->hasNode("/style/Icon/FontSize"))
		{
			styleDoc->addChild("/style/Icon","FontSize");
		}

		styleDoc->setNodeText("/style/Icon/FontSize",StringUtils::toString(m_fontSize));

		if (!styleDoc->hasNode("/style/Icon/TextAppearanceMode"))
		{
			styleDoc->addChild("/style/Icon","TextAppearanceMode");
		}

		styleDoc->setNodeText("/style/Icon/TextAppearanceMode",StringUtils::toString(m_textAppearanceMode));
	}

	m_strStyle = styleDoc->getXMLString();
}


std::string IconStyleFeatureCollectionProcess::getIdentity() const
{	
	if (m_bitmapPipelineDefinition.size() > 0)
	{
		assert(this->m_initState == knInitialized && "Process must be initialized");
	}

	//The main issue in IconStyleFeatureCollectionProcess identity is that the BitmapPipeline parameter is a pipeline.
	//Therefore, we replace this parameter to be the pipeline identity instead of the actual pipeline

	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	PYXPointer<ProcessSpec> spSpec = getSpec();
	assert(spSpec);

	ProcessIdentity identity(spSpec->getClass());
	identity.setData(getData());
	
	std::map<std::string,std::string> attributes = getAttributes();

	//Replace the Bitmap pipeline definition into Bitmap pipeline identity
	if (!m_bitmapPipelineDefinition.empty() && m_spBitmap)
	{
		std::string bitmapIdentity = (dynamic_cast<IProcess*>(m_spBitmap.get()))->getIdentity();

		attributes["BitmapPipeline" ] = bitmapIdentity;
	}

	identity.setAttributes(attributes);

	const int nParameterCount = getParameterCount();
	for (int n = 0; n < nParameterCount; ++n)
	{
		PYXPointer<Parameter> spParam = getParameter(n);
		assert(spParam);
		identity.addInput(*spParam);
	}

	return identity();
}



///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> IconStyleFeatureCollectionProcess::getIterator() const
{
	return m_spInputFC->getIterator();
}

PYXPointer<FeatureIterator> IconStyleFeatureCollectionProcess::getIterator(const PYXGeometry& geometry) const
{
	return m_spInputFC->getIterator(geometry);
}


std::vector<FeatureStyle> IconStyleFeatureCollectionProcess::getFeatureStyles() const
{
	return m_spInputFC->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> IconStyleFeatureCollectionProcess::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFC->getFeature(strFeatureID);	
}

PYXPointer<const PYXTableDefinition> IconStyleFeatureCollectionProcess::getFeatureDefinition() const
{
	return m_spInputFC->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> IconStyleFeatureCollectionProcess::getFeatureDefinition()
{
	return m_spInputFC->getFeatureDefinition();
}

bool IconStyleFeatureCollectionProcess::canRasterize() const
{
	return m_spInputFC->canRasterize();
}


///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

int IconStyleFeatureCollectionProcess::getEmbeddedResourceCount() const
{
	if (m_bitmapPipelineDefinition.empty())
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

boost::intrusive_ptr<IProcess> IconStyleFeatureCollectionProcess::getEmbeddedResource(int index) const
{
	if (!m_bitmapPipelineDefinition.empty())
	{
		if (index == 0)
		{
			return (dynamic_cast<IProcess*>(m_spBitmap.get()));
		}
		else
		{
			PYXTHROW(PYXException,"Index out of range");
		}
		
	}
	else 
	{
		PYXTHROW(PYXException,"Index out of range");
	}
}