/******************************************************************************
first_not_null_geometry_provider.cpp

begin		: 2013-6-17
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "stdafx.h"
#include "first_not_null_geometry_provider.h"
#include "pyxis/pipe/process.h"

// {0FD362D5-501E-4CD7-ADB0-9EE1C99B64BE}
PYXCOM_DEFINE_CLSID(FirstNotNullGeometryProvider, 
0xfd362d5, 0x501e, 0x4cd7, 0xad, 0xb0, 0x9e, 0xe1, 0xc9, 0x9b, 0x64, 0xbe);


PYXCOM_CLASS_INTERFACES(FirstNotNullGeometryProvider, IGeometryProvider::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FirstNotNullGeometryProvider, "First Not Null Geometry Provider", "Provides a geometry for a given IRecord based on the first available input geometry provider.", "Analysis/Features/Geotagging",
					IGeometryProvider::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IGeometryProvider::iid, 1, -1, "Input geometry providers", "A collection of geometry providers, based on which a geometry is inducted for a given record.")

IPROCESS_SPEC_END

IProcess::eInitStatus FirstNotNullGeometryProvider::initImpl()
{
	m_geometryProviders = std::vector<boost::intrusive_ptr<IGeometryProvider>>();
	for(int i = 0 ; i < getParameter(0)->getValueCount(); ++i)
	{
		boost::intrusive_ptr<IGeometryProvider> gp = getParameter(0)->getValue(i)->getOutput()->QueryInterface<IGeometryProvider>();
		if(gp)
		{
			m_geometryProviders.push_back(gp);
		}
		else
		{
			m_spInitError->setError("Invalid input connected to First not null geometry provider process. input is not a Geometry Provider");
			return knFailedToInit;
		}
	}
	if(m_geometryProviders.size() == 0)
	{
		m_spInitError->setError("No inputs provided to first not null geometry provider process");
		return knFailedToInit;
	}
	return knInitialized;
}

PYXPointer<PYXGeometry> STDMETHODCALLTYPE FirstNotNullGeometryProvider::getGeometry(boost::intrusive_ptr<IRecord> & record) const
{
	for(auto it : m_geometryProviders)
	{
		auto geometry = (it)->getGeometry(record);
		if(geometry)
		{
			return geometry;
		}
	}
	TRACE_DEBUG("No Geometry found for the record: " << record);
	return NULL;
}