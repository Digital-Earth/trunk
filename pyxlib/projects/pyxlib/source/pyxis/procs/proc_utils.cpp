/******************************************************************************
proc_utils.cpp

begin		: 2007-04-13
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "proc_utils.h"

#include "pyxis/procs/bad_coverage.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/procs/default_feature.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/srs.h"
#include "pyxis/procs/string.h"
#include "pyxis/procs/url.h"
#include "pyxis/procs/named_geometry_proc.h"
#include "pyxis/procs/process_collection_proc.h"
#include "pyxis/procs/xy_bounds_region_proc.h"
#include "pyxis/procs/tool_box_provider.h"
#include "pyxis/procs/user_credentials_provider.h"
#include "pyxis/procs/feature_collection_index_proc.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/derm/coord_converter.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BadCoverage),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ConstCoverage),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(NamedGeometryProc),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PathProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ProcessCollectionProc),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(SRSProc),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(StringProc),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(UrlProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(XYBoundsRegionProc),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ToolBoxProviderProc),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(UserCredentialsProviderProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PYXAxisFlipCoordConverter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureCollectionIndexProcess),
PYXCOM_END_CLASS_OBJECT_TABLE_EXPORT_AS(pyxis__procs__proc_utils__GCO)

void ProcUtils::initialize()
{
	PYXCOMRegister(pyxis__procs__proc_utils__GCO);
}

void ProcUtils::uninitialize()
{
}

//! Tester class
Tester<ProcUtils> gTester;

void ProcUtils::test()
{
	std::vector<GUID> vecClasses;
	PYXCOMGetClassIDs(IProcess::iid, &vecClasses);
	std::vector<GUID>::iterator it;

	for(it = vecClasses.begin(); it != vecClasses.end(); ++it)
	{
		boost::intrusive_ptr<IProcess> spProc;
		HRESULT hr = 
			PYXCOMCreateInstance(*it, 0, IProcess::iid, (void**) &spProc);

		if(FAILED(hr))
		{
			TRACE_ERROR("PYXCOM was unable to create an instance of " + 
				guidToStr(*it) + ".");
			TEST_ASSERT(false);
		}
		else
		{
			try
			{
				TRACE_INFO("Getting and setting default attributes for '" + 
					spProc->getProcName() + "'.");
				std::map<std::string, std::string> mapAttr = 
					spProc->getAttributes();
				spProc->setAttributes(mapAttr);
			}
			catch (PYXException& ex)
			{
				TRACE_ERROR("The process '" + spProc->getProcName() + 
					"' threw the following "
					"exception while trying to get and set its default "
					"attributes: " + ex.getFullErrorString());
				TEST_ASSERT(false);
			}
			catch (...)
			{
				TRACE_ERROR("The process '" + spProc->getProcName() + 
					" threw an exception while trying to get and set its "
					"default attributes.");
				TEST_ASSERT(false);
			}
		}
	}
}
