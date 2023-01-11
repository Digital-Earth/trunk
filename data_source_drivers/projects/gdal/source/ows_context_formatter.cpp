/******************************************************************************
ows_context_formatter.cpp

begin      : 03/01/2013 9:57:18 AM
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "ows_context_formatter.h"


#include "boost/date_time/posix_time/posix_time.hpp"
#include "pyxis/derm/wgs84_coord_converter.h"

// {9EC79F0C-8B54-4091-8C56-96C843D24D8D}
PYXCOM_DEFINE_CLSID(OwsContextFormatter ,
0x9ec79f0c, 0x8b54, 0x4091, 0x8c, 0x56, 0x96, 0xc8, 0x43, 0xd2, 0x4d, 0x8d);

PYXCOM_CLASS_INTERFACES(OwsContextFormatter , IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(OwsContextFormatter , "OWS Context Formatter", "Format a pipeline into an OWS Context document", "Hidden",
					IPipeFormater::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END


std::string STDMETHODCALLTYPE OwsContextFormatter::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"OwsContextFormatter\">"
		  "<xs:complexType>"
			"<xs:sequence>"

			  "<xs:element name=\"author_name\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Author Name</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"author_email\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Author Email</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			   "<xs:element name=\"author_uri\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Author Website</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"location\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Location</friendlyName>"
					"<description>Gml geometry</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE OwsContextFormatter::setAttributes( const std::map<std::string, std::string>& mapAttr )
{
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"author_name",m_author_name);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"author_email",m_author_email);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"author_uri",m_author_uri);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"location",m_location);
}

std::map<std::string, std::string> STDMETHODCALLTYPE OwsContextFormatter::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["author_name"] = m_author_name;
	mapAttr["author_email"] = m_author_email;
	mapAttr["author_uri"] = m_author_uri;
	mapAttr["location"] = m_location;

	return mapAttr;
}

IProcess::eInitStatus OwsContextFormatter::initImpl()
{
	m_document_title = getProcName();
	m_document_subtitle = getProcDescription();
	return IProcess::knInitialized;
}

bool OwsContextFormatter::canFormatPipeline( boost::intrusive_ptr<IProcess> pipeline ) const
{
	return true;
}

std::string OwsContextFormatter::formatPipeline( boost::intrusive_ptr<IProcess> pipeline ) const
{
	PYXPointer<CSharpXMLDoc> doc = CSharpXMLDoc::create(
"<feed xmlns=\"http://www.w3.org/2005/Atom\" \
      xml:lang='en' >\
  <category term=\"http://www.opengis.net/spec/owc/1.0/req/atom\" scheme=\"http://www.opengis.net/spec/owc/specReference\" label=\"This file is compliant with version 1.0 of OGC Context\"/>\
  <generator uri=\"http://www.pyxisinnovation.com/\" version=\"1.0\">Pyxis Innovation WorldView</generator>\
</feed>");

	doc->addNamespace("atom","http://www.w3.org/2005/Atom");
	doc->addNamespace("dc","http://purl.org/dc/elements/1.1/");
	doc->addNamespace("georss","http://www.georss.org/georss");
	doc->addNamespace("gml","http://www.opengis.net/gml");
	doc->addNamespace("owc","http://www.opengis.net/owc/1.0");

	doc->addChildWithInnerText("/atom:feed","atom:id","pyxis://"+StringUtils::toString(ProcRef(pipeline)));

	if (m_document_title.empty())
	{
		//default title
		doc->addChildWithInnerText("/atom:feed","atom:title",pipeline->getProcName());	
	}
	else
	{
		doc->addChildWithInnerText("/atom:feed","atom:title",m_document_title);
	}

	if (m_document_subtitle.empty())
	{
		//default description
		doc->addChildWithInnerText("/atom:feed","atom:subtitle",pipeline->getProcDescription());
	}
	else
	{
		doc->addChildWithInnerText("/atom:feed","atom:subtitle",m_document_subtitle);
	}	

	std::string documentTime = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::universal_time())+"Z";
	//TODO: need to make sure this is the right format
	doc->addChildWithInnerText("/atom:feed","atom:updated",documentTime);

	if (!m_location.empty())
	{
		doc->addChild("/atom:feed","georss:where");
		doc->setInnerXMLString("/atom:feed/georss:where",m_location);
	}

	doc->addChild("/atom:feed","atom:author");

	if (!m_author_name.empty())
	{
		doc->addChildWithInnerText("/atom:feed/atom:author","atom:name",m_author_name);
	}

	if (!m_author_email.empty())
	{
		doc->addChildWithInnerText("/atom:feed/atom:author","atom:email",m_author_email);
	}

	if (!m_author_uri.empty())
	{
		doc->addChildWithInnerText("/atom:feed/atom:author","atom:uri",m_author_uri);
	}

	//add entries...
	examinPipeline(doc,pipeline);

	return doc->getXMLString();
}

void OwsContextFormatter::examinPipeline(const PYXPointer<CSharpXMLDoc> & doc, const boost::intrusive_ptr<IProcess> & pipeline) const
{
	if (pipeline->getInitState() != knInitialized)
	{
		return;
	}

	boost::intrusive_ptr<IOWSReference> reference = pipeline->getOutput()->QueryInterface<IOWSReference>();

	if (!reference)
	{
		//this is not a IOWS reference - therefore, we will examine all its inputs...
		for(int p=0;p<pipeline->getParameterCount();++p)
		{
			PYXPointer<Parameter> param = pipeline->getParameter(p);
			for(int v=0;v<param->getValueCount();v++)
			{
				examinPipeline(doc,param->getValue(v));
			}
		}

		return;
	}

	doc->addChild("/atom:feed","atom:entry");
	int nodeIndex = doc->getNodesCount("/atom:feed/atom:entry");

	std::string entryPath = "/atom:feed/atom:entry[" + StringUtils::toString(nodeIndex) + "]";

	//add a uniqe id
	doc->addChildWithInnerText(entryPath,"atom:id","pyxis://"+StringUtils::toString(ProcRef(pipeline)));
	//add a right name time
	doc->addChildWithInnerText(entryPath,"atom:updated",doc->getNodeText("/atom:feed/atom:updated"));

	doc->addChildWithInnerText(entryPath,"atom:title",pipeline->getProcName());
	doc->addChildWithInnerText(entryPath,"atom:content",pipeline->getProcDescription());
	doc->setAttributeValue(entryPath+"/atom:content","type","text");

	//Convert the pipeline geometry into gml:Envelope
	boost::intrusive_ptr<IFeature> feature = pipeline->getOutput()->QueryInterface<IFeature>();
	WGS84CoordConverter convreter;
	PYXRect2DDouble rect1,rect2;
	feature->getGeometry()->getBoundingRects(&convreter,&rect1,&rect2);

	if (!rect1.empty())
	{
		//<georss:where>
		// <gml:Envelope srsName="EPSG:4326" srsDimension="2">
		//  <gml:lowerCorner>18.548652 -72.3627630648135</gml:lowerCorner>
		//  <gml:upperCorner>18.592594 -72.2677392817456</gml:upperCorner>
		// </gml:Envelope>
		//</georss:where>
		doc->addChild(entryPath,"georss:where");
		doc->addChild(entryPath+"/georss:where","gml:Envelope");
		doc->addAttribute(entryPath+"/georss:where/gml:Envelope","srsName","urn:ogc:def:crs:EPSG::4326");
		doc->addAttribute(entryPath+"/georss:where/gml:Envelope","srsDimension","2");
		doc->addChildWithInnerText(entryPath+"/georss:where/gml:Envelope","gml:lowerCorner",StringUtils::toString(rect1.yMin()) + " " + StringUtils::toString(rect1.xMin()));
		doc->addChildWithInnerText(entryPath+"/georss:where/gml:Envelope","gml:upperCorner",StringUtils::toString(rect1.yMax()) + " " + StringUtils::toString(rect1.xMax()));
	}

	try
	{
		//append owc:offering at the end of the entery
		doc->setInnerXMLString(entryPath,
			doc->getInnerXMLString(entryPath) + 
			reference->getOWSReference(IOWSReference::OwsContextReference,*reference->getDefaultOutputFormat()));
	}
	catch (...)
	{
	}
}