/******************************************************************************
ows_context_pipe_formater.cpp

begin      : 15/11/2012 2:31:48 PM
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

#include "ows_context_pipe_formater.h"
#include "ows_reference.h"
#include "exceptions.h"

#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/srs.h"
#include "pyxis/utility/file_utils.h"

#include "ogr_feature.h"
#include "ogrsf_frmts.h"


// {83E8A395-0A4F-4676-B5A6-CE27B9376254}
PYXCOM_DEFINE_CLSID(OwsContextPipeFormater, 
0x83e8a395, 0xa4f, 0x4676, 0xb5, 0xa6, 0xce, 0x27, 0xb9, 0x37, 0x62, 0x54);


PYXCOM_CLASS_INTERFACES(OwsContextPipeFormater, IPipeFormater::iid, PYXCOM_IUnknown::iid);

OwsContextPipeFormater::OwsContextPipeFormater()
{
}

bool OwsContextPipeFormater::canFormatPipeline(boost::intrusive_ptr<IProcess> pipeline) const
{
	return true;
}

std::string OwsContextPipeFormater::formatPipeline(boost::intrusive_ptr<IProcess> pipeline) const
{
	std::stringstream stream;

	stream << 
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<feed xmlns=\"http://www.w3.org/2005/Atom\" "
			  "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:georss=\"http://www.georss.org/georss\" "
			  "xmlns:gml=\"http://www.opengis.net/gml\" "
			  "xmlns:owc=\"http://www.opengis.net/owc/1.0\" "
			  "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
			  "xsi:schemaLocation=\"http://www.w3.org/2005/Atom ../atom/2005/atom.xsd http://purl.org/dc/elements/1.1/ ../../../csw/2.0.2/rec-dcmes.xsd http://www.georss.org/georss ../georss/1.1/georss.xsd http://www.opengis.net/gml ../georss/1.1/gmlgeorss311.xsd http://www.opengis.net/owc/1.0 ../OWSContextCore.xsd\" "
			  "xml:lang=\"en\">";

	stream << "<id>" << ProcRef(pipeline) << "</id>";
	stream << "<title>" << XMLUtils::toSafeXMLText(pipeline->getProcName()) << "</title>";
	stream << "<subtitle>" << XMLUtils::toSafeXMLText(pipeline->getProcDescription()) << "</subtitle>";
	stream << "<updated>" << XMLUtils::toSafeXMLText(pipeline->getProcName()) << "</updated>";

	std::set<ProcRef> visitedProcs;
	visitedProcs.insert(ProcRef(pipeline));

	addEnteyForPipeline(stream,pipeline,visitedProcs);

	stream << "<feed>" << XMLUtils::toSafeXMLText(pipeline->getProcName()) << "</feed>";

	return stream.str();
}

void OwsContextPipeFormater::addEnteyForPipeline(std::stringstream & stream,const boost::intrusive_ptr<IProcess> & pipeline, std::set<ProcRef> & visitedProcs) const
{
	if (pipeline->getInitState() == IProcess::knInitialized)
	{
		boost::intrusive_ptr<IOWSReference> owsReference = pipeline->getOutput()->QueryInterface<IOWSReference>();

		if (owsReference)
		{
			std::string reference = owsReference->getOWSReference(*owsReference->getDefaultOutputFormat());

			stream << "<entry>";
			stream << "<title>" << XMLUtils::toSafeXMLText(pipeline->getProcName()) << "</title>";
			stream << "<subtitle>" << XMLUtils::toSafeXMLText(pipeline->getProcDescription()) << "</subtitle>";
			stream << "</entry>";
			return;
		}
	}

	for(int p=0;p<pipeline->getParameterCount();p++)
	{
		PYXPointer<Parameter> paramter = pipeline->getParameter(p);

		for(int v=0;v<paramter->getValueCount();v++)
		{
			boost::intrusive_ptr<IProcess> process = paramter->getValue(v);

			if (visitedProcs.find(ProcRef(process)) == visitedProcs.end())
			{
				//mark this process as visited...
				visitedProcs.insert(ProcRef(process));

				//visit process
				addEnteyForPipeline(stream,pipeline,visitedProcs);
			}
		}
	}
}
