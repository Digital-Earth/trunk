/******************************************************************************
pyxilization_driver.cpp

begin		: 2007-01-26
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Nick Lipson, Dale Offord
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxilization_driver.h"

// local includes
#include "pyx_const_coverage.h"
#include "file_utils.h"
#include "gdal_data_source.h"
#include "pyx_exception.h"
#include "pyx_histogram_filter.h"
#include "pyx_iterator.h"
#include "pyx_neighbourhood_filter.h"
#include "pyx_sampler_factory.h"
#include "pyx_subtraction_filter.h" 
#include "pyx_zoom_out_filter.h"
#include "pyx_zoom_in_filter.h"
#include "pyx_type_modification_filter.h"
#include "pyx_histogram_filter.h"
#include "pyx_data_classifier.h"
#include "pyx_suitability_filter.h"
#include "pyx_data_source_manager.h"
#include "pyxilization_data_source.h"
//#include "pyx_default_coverage_cache_filter.h"

#include "tester.h"

//! The name of the driver
static const std::string kstrDriverName = "Pyxilization";

//! Tester class
TesterUnit<PYXILIZATIONDriver> gTester;

//! Test method
void PYXILIZATIONDriver::test()
{
}

//! Constructor.
PYXILIZATIONDriver::PYXILIZATIONDriver()
{
}

//! Destructor
PYXILIZATIONDriver::~PYXILIZATIONDriver()
{
}

/*!
Get the name of the driver.

\return	The name of the driver.
*/
const std::string& PYXILIZATIONDriver::getName() const
{
	return kstrDriverName;
}

boost::shared_ptr<PYXDataSource>
PYXILIZATIONDriver::openForRead(const std::string& strDataSourceName) const
{
	//if it's the one we want, make the DS
	if(strDataSourceName == "c:\\pyxis_data\\pyxilization.tr1")
	{
		boost::shared_ptr<PyxilizationDataSource> spTRDS(new PyxilizationDataSource());
		return spTRDS;
	}
	return boost::shared_ptr<PYXDataSource>();
}

bool PYXILIZATIONDriver::getFileInfo(std::string& strURI, PYXDriver::FileInfo* pFileInfo, bool * pbExit)
{
	//the magic word
	std::string magic("c:\\pyxis_data\\pyxilization.tr1");
	if(strURI == magic)
	{
		//just let the system know what we are
		pFileInfo->m_strDriverName = getName();
		pFileInfo->m_nFileType = PYXDriver::eFileType::PYXIS;
		pFileInfo->m_nLayerType = PYXDriver::eLayerType::knFeature;
		pFileInfo->m_nRasterSize.setX(-1);
		pFileInfo->m_nRasterSize.setY(-1);
		pFileInfo->m_strDriverName = kstrDriverName;
		pFileInfo->m_nFileInterpretation = PYXDriver::eFileInterpretation::knVector;
		return 1;
	}
	return 0;
}