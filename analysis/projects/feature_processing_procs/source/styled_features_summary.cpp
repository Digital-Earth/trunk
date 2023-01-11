/******************************************************************************
styled_features_summary.cpp

begin		: March 07, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "styled_features_summary.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_identity.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/color_palette.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>

// {4F41A149-7EBF-41cb-B0F9-031D76EF81E0}
PYXCOM_DEFINE_CLSID(StyledFeaturesSummaryProcess, 
0x4f41a149, 0x7ebf, 0x41cb, 0xb0, 0xf9, 0x3, 0x1d, 0x76, 0xef, 0x81, 0xe0);

PYXCOM_CLASS_INTERFACES(StyledFeaturesSummaryProcess, IProcess::iid, IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(StyledFeaturesSummaryProcess, "Apply Features Style", "A process that applies an Styling to its input feature collection.", "Styling/Features",
					IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureGroup::iid, 1, 1, "Input Features", "A feature collection to apply an Icon style to.")
	//IPROCESS_SPEC_PARAMETER(IBitmap::iid, 1, 1, "Input Icon Bitmap", "a bitmap to apply for icons")
IPROCESS_SPEC_END

const std::string StyledFeaturesSummaryProcess::kstrLineColour = "line_colour";
const std::string StyledFeaturesSummaryProcess::kstrIcon = "icon";

//! Tester class
Tester<StyledFeaturesSummaryProcess> gTester;

//! Test method
void StyledFeaturesSummaryProcess::test()
{
	// TODO[shatzi]: add unit test
}

//! Constructor
StyledFeaturesSummaryProcess::StyledFeaturesSummaryProcess()
{
	//icon style
	m_iconStyle = "NoIcon";
	m_bitmapPipelineDefinition = "";
	m_iconIndex = -1;
	m_iconScaling = 100;
	m_iconColour = "uint8_t[3] 255 255 255";
	m_iconColourPalette = "";
	m_iconColourPaletteScale = "Linear";
	m_iconColourPaletteField = "";

	//text style
	m_textAppearanceMode = "OnMouseOver";
	m_textAlignment = "Center";
	m_fontStyle = "Regular";
	m_fontColor = "White";
	m_fontFamily = "Arial";
	m_fontSize = 15;

	//line styling
	m_borderStyle = "NoLine";
	m_lineColour = "uint8_t[3] 127 127 255";
	m_lineWidth = 1;
	m_lineOpacity = 100;

	//fill styling
	m_areaStyle = "SolidColor";
	m_areaColour = "uint8_t[3] 127 127 255"; 
	m_areaOpacity = 60;
	m_areaPalette = "";
	m_areaPaletteScale = "Linear";
	m_areaPaletteField = "";
}

//! Destructor
StyledFeaturesSummaryProcess::~StyledFeaturesSummaryProcess()
{
}

///////////////////////////////////////////////////////////////////////////////
// IProcess
///////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus StyledFeaturesSummaryProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_spInputFG = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureGroup>();

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

	if (m_spInputFG)
	{
		m_spGeom = m_spInputFG->getGeometry();

		if(m_spGeom)
		{
			m_bWritable = m_spInputFG->isWritable();
			m_strID = m_spInputFG->getID();

			updateStyle();

			return knInitialized;
		}
		else
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError(
				"StyledFeaturesSummaryProcess: Input process' feature collection does not have a valid geometry.");
		}
	}
	else
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"StyledFeaturesSummaryProcess: Input process does not output a valid feature group.");
	}

	return knFailedToInit;

}

std::string STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getAttributeSchema() const
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
		"<xs:simpleType name=\"IconStyleOptions\">"
		  "<xs:restriction base=\"xs:string\">"
		  	"<xs:enumeration value=\"NoIcon\" />"
			"<xs:enumeration value=\"SolidColor\" />"
			"<xs:enumeration value=\"Palette\" />"
		  "</xs:restriction>"
	    "</xs:simpleType>"
		"<xs:simpleType name=\"AppearanceMode\">"
		  "<xs:restriction base=\"xs:string\">"
		  	"<xs:enumeration value=\"Always\" />"
			"<xs:enumeration value=\"OnMouseOver\" />"
		  "</xs:restriction>"
	    "</xs:simpleType>"
		"<xs:simpleType name=\"BorderStyleOptions\">"
		  "<xs:restriction base=\"xs:string\">"
		  	"<xs:enumeration value=\"NoLine\" />"
			"<xs:enumeration value=\"SolidColor\" />"
		  "</xs:restriction>"
	    "</xs:simpleType>"
		"<xs:simpleType name=\"AreaStyleOptions\">"
		  "<xs:restriction base=\"xs:string\">"
		  	"<xs:enumeration value=\"NoFill\" />"
			"<xs:enumeration value=\"SolidColor\" />"
			"<xs:enumeration value=\"Palette\" />"
		  "</xs:restriction>"
	    "</xs:simpleType>"
		"<xs:simpleType name=\"PaletteScaleOptions\">"
		  "<xs:restriction base=\"xs:string\">"
		  	"<xs:enumeration value=\"Linear\" />"
			"<xs:enumeration value=\"Normalized\" />"
		  "</xs:restriction>"
	    "</xs:simpleType>"
		"<xs:element name=\"StyledFeaturesSummaryProcess\">"
		  "<xs:complexType>"
			  "<xs:sequence>"

				"<xs:element name=\"IconStyle\" type=\"IconStyleOptions\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Icon Style Mode</friendlyName>"
						"<description>Specify the style for the features' icon.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

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

				   "<xs:element name=\"IconColourPalette\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Icon Palette</friendlyName>"
						"<description>The palette to use to set the icon colour.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"IconColourPaletteScale\" type=\"PaletteScaleOptions\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Icon Palette Scale</friendlyName>"
						"<description>Scale to use for the palette.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"IconColourPaletteField\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Icon Palette Field</friendlyName>"
						"<description>The field to use for selecting colors from the palette for setting the icon colour.</description>"
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

				  "<xs:element name=\"BorderStyle\" type=\"BorderStyleOptions\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Border Style Mode</friendlyName>"
						"<description>Specify the style for the features' border.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"LineColour\" type=\"colour\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Line Color</friendlyName>"
						"<description>Specify the color for the features' border.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"LineWidth\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Line Width</friendlyName>"
						"<description>Specify the width of a line.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"LineOpacity\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Line Opactiy</friendlyName>"
						"<description>Specify the opactiy of a line (between 0 to 100).</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"AreaStyle\" type=\"AreaStyleOptions\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Area Style Mode</friendlyName>"
						"<description>Specify the style for the features' area.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"AreaColour\" type=\"colour\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Area Color</friendlyName>"
						"<description>Specify the color for the features' area.</description>"
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

				  "<xs:element name=\"AreaPalette\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Area Palette</friendlyName>"
						"<description>the palette to use to fill the features' area.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"AreaPaletteScale\" type=\"PaletteScaleOptions\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Area Palette Scale</friendlyName>"
						"<description>Scale to use for the palette.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				"<xs:element name=\"AreaPaletteField\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Area Palette Field</friendlyName>"
						"<description>the field to use for selecting colors from the palette in order to fill the features' area.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			  "</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}

std::map<std::string, std::string> StyledFeaturesSummaryProcess::getAttributes() const
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

	mapAttr["IconStyle"] = m_iconStyle;
	mapAttr["IconColour"] = m_iconColour;
	mapAttr["IconColourPalette"] = m_iconColourPalette;
	mapAttr["IconColourPaletteScale"] = m_iconColourPaletteScale;
	mapAttr["IconColourPaletteField"] = m_iconColourPaletteField;

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

	mapAttr["BorderStyle"] = m_borderStyle;
	mapAttr["LineColour"] = m_lineColour;
	mapAttr["LineWidth"] = StringUtils::toString(m_lineWidth);
	mapAttr["LineOpacity"] = StringUtils::toString(m_lineOpacity);

	mapAttr["AreaStyle"] = m_areaStyle;
	mapAttr["AreaColour"] = m_areaColour;
	mapAttr["AreaOpacity"] = StringUtils::toString(m_areaOpacity);
	mapAttr["AreaPalette"] = m_areaPalette;
	mapAttr["AreaPaletteScale"] = m_areaPaletteScale;
	mapAttr["AreaPaletteField"] = m_areaPaletteField;

	return mapAttr;
}

boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getOutput() const
{
	return static_cast<const IFeatureGroup*>(this);
}

boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getOutput()
{
	return static_cast<IFeatureGroup*>(this);
}

void StyledFeaturesSummaryProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{	
	//icon style
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"IconStyle",m_iconStyle);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"BitmapPipeline",m_bitmapPipelineDefinition);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"IconIndex",int,m_iconIndex);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"IconScale",int,m_iconScaling);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"IconColour",m_iconColour);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"IconColourPalette",m_iconColourPalette);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"IconColourPaletteScale",m_iconColourPaletteScale);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"IconColourPaletteField",m_iconColourPaletteField);

	//text style
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TextField",m_textField);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TextAlignment",m_textAlignment);

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FontFamily",m_fontFamily);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FontColor",m_fontColor);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FontStyle",m_fontStyle);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"FontSize",int,m_fontSize);

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TextAppearanceMode",m_textAppearanceMode);

	//line styling
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"BorderStyle",m_borderStyle);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"LineColour",m_lineColour);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"LineWidth",int,m_lineWidth);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"LineOpacity",int,m_lineOpacity);

	//fill styling
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"AreaStyle",m_areaStyle);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"AreaColour",m_areaColour);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"AreaOpacity",int,m_areaOpacity);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"AreaPalette",m_areaPalette);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"AreaPaletteScale",m_areaPaletteScale);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"AreaPaletteField",m_areaPaletteField);

	sanitizeAttributes();

	m_initState = knNeedsInit;
}

void StyledFeaturesSummaryProcess::sanitizeAttributes()
{
	//if someone set the icon index to -1 - make sure the style is NoIcon
	if (m_iconIndex == -1)
	{
		m_iconStyle = "NoIcon";
	}
	else if (m_iconStyle == "NoIcon") // if someone set the icon index to something != -1 - and the style is NoIcon - make it a simple icon
	{
		m_iconStyle = "SolidColor";
	}

	if (m_iconStyle == "SolidColor")
	{
		m_iconColourPalette = "";
		m_iconColourPaletteField = "";
		m_iconColourPaletteScale = "Normalized";
	}
	else if (m_iconStyle == "Palette")
	{
		if (m_iconColourPaletteField.empty())
		{
			m_iconStyle = "SolidColor";
			m_iconColourPalette = "";
			m_iconColourPaletteField = "";
			m_iconColourPaletteScale = "Normalized";			
		}
	}
	else //NoIcon
	{
		m_iconStyle = "NoIcon";
		m_bitmapPipelineDefinition = "";
		m_iconIndex = -1;
		m_iconColour = "uint8_t[3] 255 255 255";
		m_iconColourPalette = "";
		m_iconColourPaletteField = "";
		m_iconColourPaletteScale = "Normalized";
	}
	
	if (m_textField.empty())
	{
		m_fontStyle = "Regular";
		m_fontColor = "White";
		m_fontFamily = "Arial";
		m_fontSize = 15;
	}

	if (m_textAlignment != "Top" &&
		m_textAlignment != "Top Left" &&
		m_textAlignment != "Top Right" &&
		m_textAlignment != "Bottom" &&
		m_textAlignment != "Bottom Left" &&
		m_textAlignment != "Bottom Right" &&
		m_textAlignment != "Left" &&
		m_textAlignment != "Center" &&
		m_textAlignment != "Right")
	{
		m_textAlignment = "Center";
	}

	if (m_textAppearanceMode != "Always" &&
		m_textAppearanceMode != "OnMouseOver")
	{
		m_textAppearanceMode = "OnMouseOver";
	}

	if (m_borderStyle == "SolidColor")
	{
		try
		{
			StringUtils::fromString<PYXValue>(m_lineColour);
		}
		catch(...)
		{
			m_lineColour = "uint8_t[3] 127 127 255";
		}

		//force width between 1 ... 2
		m_lineWidth = std::max(1,std::min(2,m_lineWidth));
		m_lineOpacity = std::max(0,std::min(100,m_lineOpacity));
	}
	else
	{
		m_borderStyle == "NoLine";
		m_lineColour = "uint8_t[3] 127 127 255";
		m_lineWidth = 1;
		m_lineOpacity = 100;
	}

	if (m_areaStyle == "SolidColor")
	{
		try
		{
			StringUtils::fromString<PYXValue>(m_areaColour);
		}
		catch(...)
		{
			m_areaColour = "uint8_t[3] 127 127 255";
		}

		//force opacity between 0...100
		m_areaOpacity = std::max(0,std::min(100,m_areaOpacity));

		m_areaPalette.clear();
		m_areaPaletteScale = "Normalized";
		m_areaPaletteField.clear();
	}
	else if (m_areaStyle == "Palette")
	{
		try
		{
			/*
			Debug: Skip palette validation

			//validate the palette is currect
			PYXColorPalette palette(m_areaPalette);
			m_areaPalette = palette.getName();
			*/
		}
		catch(...)
		{
			m_areaPalette = PYXColorPalette().getName();
		}

		//force opacity between 0...100
		m_areaOpacity = std::max(0,std::min(100,m_areaOpacity));

		if (m_areaPaletteField.empty())
		{
			m_areaStyle == "NoFill";
			m_areaColour = "uint8_t[3] 127 127 255";
			m_areaOpacity = 100;
			m_areaPalette.clear();
			m_areaPaletteScale = "Normalized";
			m_areaPaletteField.clear();
		}
	}
	else
	{
		m_areaStyle == "NoFill";
		m_areaColour = "uint8_t[3] 127 127 255";
		m_areaOpacity = 100;
		m_areaPalette.clear();
		m_areaPaletteScale = "Normalized";
		m_areaPaletteField.clear();
	}
}

void StyledFeaturesSummaryProcess::updateStyle()
{
	PYXPointer<CSharpXMLDoc> styleDoc;
	if (m_spInputFG)
	{
		styleDoc = CSharpXMLDoc::create(m_spInputFG->getStyle());
	} 
	else 
	{
		styleDoc = CSharpXMLDoc::create("<style></style>");
	}

	if (!styleDoc->hasNode("/style"))
	{
		styleDoc->addChild("/","style");
	}

	if (!styleDoc->hasNode("/style/Icon"))
	{
		styleDoc->addChild("/style","Icon");
	}
	else 
	{
		styleDoc->setInnerXMLString("/style/Icon","");
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

		if (m_iconStyle == "SolidColor")
		{		
			if (!styleDoc->hasNode("/style/Icon/Colour"))
			{
				styleDoc->addChild("/style/Icon","Colour");
			}
			styleDoc->setNodeText("/style/Icon/Colour", m_iconColour );
		}
		else if (m_iconStyle == "Palette")
		{
			if (!styleDoc->hasNode("/style/Icon/ColourPalette"))
			{
				styleDoc->addChild("/style/Icon","ColourPalette");
			}
			styleDoc->setNodeText("/style/Icon/ColourPalette",m_iconColourPalette);

			if (!styleDoc->hasNode("/style/Icon/ColourPaletteField"))
			{
				styleDoc->addChild("/style/Icon","ColourPaletteField");
			}
			styleDoc->setNodeText("/style/Icon/ColourPaletteField",m_iconColourPaletteField);			
		}
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

		styleDoc->setNodeText("/style/Icon/TextAppearanceMode",m_textAppearanceMode);
	}

	if (!styleDoc->hasNode("/style/Line"))
	{
		styleDoc->addChild("/style","Line");
	}
	else
	{
		styleDoc->setInnerXMLString("/style/Line","");
	}

	if (m_borderStyle == "SolidColor")
	{
		if (!styleDoc->hasNode("/style/Line/Colour"))
		{
			styleDoc->addChild("/style/Line","Colour");
		}
		styleDoc->setNodeText("/style/Line/Colour", m_lineColour );

		if (!styleDoc->hasNode("/style/Line/Width"))
		{
			styleDoc->addChild("/style/Line","Width");
		}
		styleDoc->setNodeText("/style/Line/Width",StringUtils::toString(m_lineWidth));

		if (!styleDoc->hasNode("/style/Line/Opacity"))
		{
			styleDoc->addChild("/style/Line","Opacity");
		}
		styleDoc->setNodeText("/style/Line/Opacity",StringUtils::toString(m_lineOpacity));
	}

	if (!styleDoc->hasNode("/style/Area"))
	{
		styleDoc->addChild("/style","Area");
	}
	else 
	{
		styleDoc->setInnerXMLString("/style/Area","");
	}

	if (m_areaStyle == "Palette")
	{
		if (!styleDoc->hasNode("/style/Area/Palette"))
		{
			styleDoc->addChild("/style/Area","Palette");
		}
		styleDoc->setNodeText("/style/Area/Palette",m_areaPalette);

		if (!styleDoc->hasNode("/style/Area/PaletteField"))
		{
			styleDoc->addChild("/style/Area","PaletteField");
		}
		styleDoc->setNodeText("/style/Area/PaletteField",m_areaPaletteField);

		if (!styleDoc->hasNode("/style/Area/Opacity"))
		{
			styleDoc->addChild("/style/Area","Opacity");
		}
		styleDoc->setNodeText("/style/Area/Opacity",StringUtils::toString(m_areaOpacity));

	}
	else if (m_areaStyle == "SolidColor")
	{
		if (!styleDoc->hasNode("/style/Area/Colour"))
		{
			styleDoc->addChild("/style/Area","Colour");
		}
		styleDoc->setNodeText("/style/Area/Colour",m_areaColour );

		if (!styleDoc->hasNode("/style/Area/Opacity"))
		{
			styleDoc->addChild("/style/Area","Opacity");
		}
		styleDoc->setNodeText("/style/Area/Opacity",StringUtils::toString(m_areaOpacity));
	}

	m_strStyle = styleDoc->getXMLString();
}


std::string StyledFeaturesSummaryProcess::getIdentity() const
{
	if (m_bitmapPipelineDefinition.size() > 0)
	{
		assert(this->m_initState == knInitialized && "Process must be initialized");
	}

	//The main issue in StyledFeaturesSummaryProcess identity is that the BitmapPipeline parameter is a pipeline.
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

PYXPointer<FeatureIterator> StyledFeaturesSummaryProcess::getIterator() const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getIterator();
}

PYXPointer<FeatureIterator> StyledFeaturesSummaryProcess::getIterator(const PYXGeometry& geometry) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getIterator(geometry);
}


std::vector<FeatureStyle> StyledFeaturesSummaryProcess::getFeatureStyles() const
{
	return m_spInputFG->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> StyledFeaturesSummaryProcess::getFeature(const std::string& strFeatureID) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getFeature(strFeatureID);	
}

PYXPointer<const PYXTableDefinition> StyledFeaturesSummaryProcess::getFeatureDefinition() const
{
	return m_spInputFG->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> StyledFeaturesSummaryProcess::getFeatureDefinition()
{
	return m_spInputFG->getFeatureDefinition();
}

bool StyledFeaturesSummaryProcess::canRasterize() const
{
	return m_spInputFG->canRasterize();
}


///////////////////////////////////////////////////////////////////////////////
// IEmbeddedResourceHolder
///////////////////////////////////////////////////////////////////////////////

int StyledFeaturesSummaryProcess::getEmbeddedResourceCount() const
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

boost::intrusive_ptr<IProcess> StyledFeaturesSummaryProcess::getEmbeddedResource(int index) const
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


///////////////////////////////////////////////////////////////////////////////
// IFeatureGroup
///////////////////////////////////////////////////////////////////////////////

Range<int> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getFeaturesCount() const
{
	return m_spInputFG->getFeaturesCount();
}

bool STDMETHODCALLTYPE StyledFeaturesSummaryProcess::moreDetailsAvailable() const
{
	return m_spInputFG->moreDetailsAvailable();
}


PYXPointer<PYXHistogram> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getFieldHistogram(int fieldIndex) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getFieldHistogram(fieldIndex);
}

PYXPointer<PYXHistogram> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getFieldHistogram(geometry,fieldIndex);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getFeatureGroup(const std::string & groupId) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getFeatureGroup(groupId);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getFeatureGroupForFeature(const std::string & featureId) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getFeatureGroupForFeature(featureId);
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getGroupIterator() const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getGroupIterator();
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE StyledFeaturesSummaryProcess::getGroupIterator(const PYXGeometry& geometry) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_spInputFG->getGroupIterator(geometry);
}