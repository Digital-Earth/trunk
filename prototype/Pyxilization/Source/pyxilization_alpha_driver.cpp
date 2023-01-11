/******************************************************************************
pyxilization_driver.cpp

begin		: 2007-01-26
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Nick Lipson, Dale Offord
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxilization_alpha_driver.h"

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
#include "armycolouringds.h"
//#include "pyx_default_coverage_cache_filter.h"

#include "tester.h"

//! The name of the driver
static const std::string kstrDriverName = "PyxilizationAlpha";

//! Tester class
TesterUnit<PYXILIZATIONAlphaDriver> gTester;

//! Test method
void PYXILIZATIONAlphaDriver::test()
{
}

//! Constructor.
PYXILIZATIONAlphaDriver::PYXILIZATIONAlphaDriver()
{
}

//! Destructor
PYXILIZATIONAlphaDriver::~PYXILIZATIONAlphaDriver()
{
}

/*!
Get the name of the driver.

\return	The name of the driver.
*/
const std::string& PYXILIZATIONAlphaDriver::getName() const
{
	return kstrDriverName;
}

boost::shared_ptr<PYXDataSource>
PYXILIZATIONAlphaDriver::openForRead(const std::string& strDataSourceName) const
{ 
	if(strDataSourceName == "c:\\pyxis_data\\pyxilizationalpha.tr2")
	{
		boost::shared_ptr<ArmyColouringDS> spTRDS(new ArmyColouringDS());
		return spTRDS;
	}
	return boost::shared_ptr<PYXDataSource>();
}

bool PYXILIZATIONAlphaDriver::getFileInfo(std::string& strURI, PYXDriver::FileInfo* pFileInfo, bool * pbExit)
{
	std::string boo(strURI);
	std::string magic("c:\\pyxis_data\\pyxilizationalpha.tr2");
	if(boo == magic)
	{
		pFileInfo->m_strDriverName = getName();
		pFileInfo->m_nFileType = PYXDriver::eFileType::PYXIS;
		pFileInfo->m_nLayerType = PYXDriver::eLayerType::knCoverage;
		pFileInfo->m_nRasterSize.setX(-1);
		pFileInfo->m_nRasterSize.setY(-1);
		pFileInfo->m_strDriverName = kstrDriverName;
		pFileInfo->m_nFileInterpretation = PYXDriver::eFileInterpretation::knRaster;
		return 1;
	}
	return 0;
}