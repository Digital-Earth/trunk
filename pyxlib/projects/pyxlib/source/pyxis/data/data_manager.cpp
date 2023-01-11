/******************************************************************************
data_manager.cpp

begin		: 2007-01-22
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "pyxis/data/data_manager.h"

#include "pyxis/data/data_driver.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/trace.h"

// standard includes
#include <vector>

namespace
{

//! The data driver objects available for use.
std::vector<boost::intrusive_ptr<IDataDriver> > vecspDataDriver;

}

void DataManager::initialize()
{
	TRACE_INFO("DataManager::initialize");

	HRESULT hr;

	std::vector<CLSID> vecClsid;
	PYXCOMGetClassIDs(IDataDriver::iid, &vecClsid);

	vecspDataDriver.clear();

	while (!vecClsid.empty())
	{
		boost::intrusive_ptr<IDataDriver> spDataDriver;

		hr = PYXCOMCreateInstance(vecClsid.back(), 0, IDataDriver::iid, (void**) &spDataDriver);
		if (FAILED(hr))
		{
			TRACE_ERROR("couldn't get IDataDriver interface (programming error?)");
		}
		else
		{
			vecspDataDriver.push_back(spDataDriver);
		}

		vecClsid.pop_back();
	}
}

void DataManager::uninitialize()
{
	TRACE_INFO("DataManager::uninitialize");

	vecspDataDriver.clear();
}

PYXPointer<PYXDataSource> DataManager::open(const std::string& strURI)
{
	TRACE_INFO("DataManager::open '" << strURI << '\'');

	// TODO implement by trying each registered DataDriver in turn

	PYXPointer<PYXDataSource> spDS;

	for (int n = 0; n != vecspDataDriver.size(); ++n)
	{
		spDS = vecspDataDriver[n]->openForRead(strURI);
		if (spDS)
		{
			break;
		}
	}

	return spDS;
}
