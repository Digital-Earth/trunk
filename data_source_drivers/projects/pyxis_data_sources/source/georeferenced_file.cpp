/******************************************************************************
file_geo_locator_proc.cpp

begin		: 2007-10-18
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE

// local includes
#include "georeferenced_file.h"
#include "exceptions.h"

//pyxlib includes
#include "pyxis/procs/url.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/app_services.h"

// boost includes
#include <boost/algorithm/string.hpp>

// {54F44709-AD41-4197-B91E-08860C25F9FE}
PYXCOM_DEFINE_CLSID(GeoRerefencedFileProcess, 
0x54f44709, 0xad41, 0x4197, 0xb9, 0x1e, 0x8, 0x86, 0xc, 0x25, 0xf9, 0xfe);

PYXCOM_CLASS_INTERFACES(GeoRerefencedFileProcess, IProcess::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GeoRerefencedFileProcess, "Georeferenced File", "Geo references a file on the PYXIS Grid", "Reader",
					IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IUrl::iid, 1, 1, "File to georeference", "File to georeference on the PYXIS Grid");
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Located Feature", "Feature and geometry to associate the file with")
IPROCESS_SPEC_END

const std::string GeoRerefencedFileProcess::kstrDoc = "doc";
const std::string GeoRerefencedFileProcess::kstrPdf = "pdf";
const std::string GeoRerefencedFileProcess::kstrDocx = "docx";

IProcess::eInitStatus GeoRerefencedFileProcess::initImpl()
{
	m_strFeatureID = procRefToStr(ProcRef(this));
	getParameter(1)->getValue(0)->getOutput()->QueryInterface(IFeature::iid, (void**) &m_spAssociatedFeature);
	assert(m_spAssociatedFeature);

	m_strFeatureID = "File Geo Locator: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	if (!m_spDefn)
	{
		boost::intrusive_ptr<IUrl> spUrl;
		getParameter(0)->getValue(0)->getOutput()->QueryInterface(IUrl::iid, (void**) &spUrl);
		std::string strFilePath = spUrl->getUrl();
		size_t lastSlash = strFilePath.rfind("\\");
		if (lastSlash == std::string::npos)
		{
			lastSlash = strFilePath.rfind("/");
		}
		std::string strFileName = strFilePath.substr(static_cast<int> (lastSlash) + 1, strFilePath.length() - static_cast<int> (lastSlash));
		m_spDefn = PYXTableDefinition::create();
		m_spDefn->addFieldDefinition("DocumentName", PYXFieldDefinition::knContextNone, PYXValue::knString);
		m_spDefn->addFieldDefinition("Path", PYXFieldDefinition::knContextNone, PYXValue::knString);
		setFieldValue(PYXValue(strFileName), 0);
		setFieldValue(PYXValue(strFilePath), 1);
		
		// for testing showing a pipeline on hyperlink click
		/*setFieldValue(PYXValue(
			AppServices::getPyxisProtocol() + 
			"{DB1174EB-0170-4831-B5DE-35BA1D694839}+1"), 1);*/
	}
	
	return knInitialized;
}

/*!
Determines if this feature can be written to or not. Since this process supports the IWriteableFeature
interface we default to true.

\return bool indiciating whether this feature can be written to or not.
*/
bool GeoRerefencedFileProcess::isWritable() const
{
	return true; 
}

/*!
Get the geometry of this feature. In this particular case the geometry is the same geometry of the 
associated feature, therefore we return the associated feature's geometry.

\return A PYXPointer to a Pyxis Geometry.
*/
PYXPointer<const PYXGeometry> GeoRerefencedFileProcess::getGeometry() const 
{
	return m_spAssociatedFeature->getGeometry();
}

/*!
Get the geometry of this feature. In this particular case the geometry is the same geometry of the 
associated feature, therefore we return the associated feature's geometry.

\return A PYXPointer to a Pyxis Geometry.
*/
PYXPointer<PYXGeometry> GeoRerefencedFileProcess::getGeometry()
{
	return m_spAssociatedFeature->getGeometry();
}

/*!
Get the style that determines how to display this feature. 

\return The icon ID encoded within a string to of the icon to display with this feature.
*/
std::string GeoRerefencedFileProcess::getStyle() const
{
	boost::intrusive_ptr<IUrl> spUrl;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IUrl::iid, (void**) &spUrl);
	if (spUrl)
	{
		return "<style><Icon><Bitmap>1</Bitmap></Icon></style>";
	}
	else
	{
		boost::intrusive_ptr<IUrl> spUrl;
		getParameter(0)->getValue(0)->getOutput()->QueryInterface(IUrl::iid, (void**) &spUrl);

		std::string strExtension = FileUtils::getExtensionNoDot(spUrl->getUrl());

		if (boost::iequals(strExtension, kstrDoc) || boost::iequals(strExtension, kstrDocx))
		{
			return "<style><Icon><Bitmap>10</Bitmap></Icon></style>";
		}
		else if (boost::iequals(strExtension, kstrPdf))
		{
			return "<style><Icon><Bitmap>13</Bitmap></Icon></style>";
		}
		else
		{
			return "<style><Icon><Bitmap>7</Bitmap></Icon></style>";
		}
	}
	return "";
}

std::string GeoRerefencedFileProcess::getStyle(const std::string& strStyleToGet) const
{
	PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(getStyle());
	return styleDoc->getNodeText("/style/" + strStyleToGet);	
}

/*!
Get the ID of this feature.

\return std::string containing the ID of this feature.
*/
const std::string& GeoRerefencedFileProcess::getID() const
{
	return m_strFeatureID;
}



		