/******************************************************************************
library_feature_collection_process.cpp

begin      : 11/12/2007 9:52:49 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define LIBRARY_SOURCE

#include "library_feature_collection_process.h"

// local includes
#include "exceptions.h"
#include "library_feature.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/multi_cell.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/string.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/string_utils.h"

// boost includes
#include <boost/filesystem/path.hpp>

// standard includes
#include <algorithm>
#include <cassert>

// {0120071D-1D18-4f81-84F9-63083E6A2402}
PYXCOM_DEFINE_CLSID(LibraryFeatureCollectionProcess, 
0x120071d, 0x1d18, 0x4f81, 0x84, 0xf9, 0x63, 0x8, 0x3e, 0x6a, 0x24, 0x2);
PYXCOM_CLASS_INTERFACES(LibraryFeatureCollectionProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(LibraryFeatureCollectionProcess, "Library Outline Process", "Outlines of the geometries of pipelines that exist in the library.", "Push",
					IFeatureCollection::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IString::iid, 1, -1, "Library Process", "The unique process reference of an entry in the library.");
IPROCESS_SPEC_END

//! The unit test class
Tester<LibraryFeatureCollectionProcess> gTester;

/*!
The unit test method for the class.
*/
void LibraryFeatureCollectionProcess::test()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE LibraryFeatureCollectionProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["Resolution"] = StringUtils::toString(m_nResolution);
	return mapAttr;
}

void STDMETHODCALLTYPE LibraryFeatureCollectionProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it;

	// resolution
	{
		it = mapAttr.find("Resolution");
		if (it == mapAttr.end())
		{
			PYXTHROW(PYXDataException, "No resolution specified for LibraryFeatureCollectionProcess");
		}
		m_nResolution = atoi(it->second.c_str());
	}
}

std::string STDMETHODCALLTYPE LibraryFeatureCollectionProcess::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"LibraryFeatureCollectionProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"resolution\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";      
}

IProcess::eInitStatus LibraryFeatureCollectionProcess::initImpl()
{
	// create a vector of the input procrefs
	m_vecProcref.clear();

	// cycle through all of the input procref parameters
	for (int n = 0; n < getParameter(0)->getValueCount(); ++n)
	{
		boost::intrusive_ptr<IString> spString; 
		getParameter(0)->getValue(n)->QueryInterface(IString::iid, (void**) &spString);
		assert(spString);
		ProcRef procref = strToProcRef(spString->str());

		if (procref.getProcID() == GUID_NULL)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError(
				"String parameter does not resolve to a valid Procref: " +
				spString->str());
			return knFailedToInit;
		}
		
		if (!Library::existsItem(procref))
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Process " + procRefToStr(procref) + " does not exist in the library.");
			return knFailedToInit;
		}

		// add to the stored list of procref
		m_vecProcref.push_back(procref);
	}

	if (m_vecProcref.size() == 0)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("No library items added to the outline process.");
		return knFailedToInit;
	}

	initMetaData();

	// IFeature variables
	m_strID = "Library Outline Process: " + procRefToStr(ProcRef(this));
	m_spGeom = createGeometry();

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

PYXPointer<IFeature> STDMETHODCALLTYPE LibraryFeatureCollectionProcess::getFeature(
	const std::string& strFeatureID) const 
{ 
	// verify the procref exists in the collection
	ProcRef procRef = strToProcRef(strFeatureID);
	std::vector<ProcRef>::const_iterator it =
		std::find(m_vecProcref.begin(), m_vecProcref.end(), procRef);
	if (it == m_vecProcref.end())
	{
		PYXTHROW(PYXDataException, "Feature does not exist in Library Outline Process: " << strFeatureID);
	}

	PYXPointer<IFeature> spFeature = new LibraryFeature(strToProcRef(strFeatureID), m_nResolution);
	return spFeature;
} 

PYXPointer<FeatureIterator> STDMETHODCALLTYPE LibraryFeatureCollectionProcess::getIterator() const 
{ 
	// pass an empty iterator so that all features in the collection pass
	return createIterator(PYXGlobalGeometry::create(m_nResolution));
} 

PYXPointer<FeatureIterator> STDMETHODCALLTYPE LibraryFeatureCollectionProcess::getIterator(
	const PYXGeometry& geometry) const 
{ 
	return createIterator(geometry.clone());
} 

//! Create an iterator for the selected geometry.
PYXPointer<FeatureIterator> LibraryFeatureCollectionProcess::createIterator(PYXPointer<PYXGeometry> spGeom) const
{
	assert(spGeom && "iterator needs a valid geometry");
	assert(0 <= spGeom->getCellResolution() && "Invalid geometry resolution.");
	return LibraryOutlineIterator::create(spGeom, m_vecProcref);
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

//! Get the current PYXIS feature.
boost::intrusive_ptr<IFeature> LibraryFeatureCollectionProcess::LibraryOutlineIterator::getFeature() const
{
	boost::intrusive_ptr<IFeature> spFeature;
	if (!end())
	{
		assert(m_spGeom);
		if (boost::dynamic_pointer_cast<PYXGlobalGeometry>(m_spGeom))
		{	
			// any feature with a valid geometry passes this filter
			spFeature = new LibraryFeature(m_vecProcref[m_nCurrent], m_spGeom->getCellResolution());
			if (!spFeature->getGeometry())
			{
				// no valid geometry
				spFeature = 0;
			}
		}
		else
		{
			// non global geometry, only intersecting processes pass
			while (!end())
			{
				spFeature = new LibraryFeature(m_vecProcref[m_nCurrent], m_spGeom->getCellResolution());
				if (spFeature->getGeometry() && spFeature->getGeometry()->intersects(*m_spGeom))
				{
					break;
				}
				else
				{
					++m_nCurrent;
					spFeature = 0;
				}
			}
		}
	}
	return spFeature;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

LibraryFeatureCollectionProcess::LibraryFeatureCollectionProcess() :
	m_nResolution(20)
{
}

/*!
Create the table definition for the loaded excel file. If the table can not be
properly defined an exception would be thrown.
*/
void LibraryFeatureCollectionProcess::initMetaData()
{
	// IFeature definition for the overall data source
	{
		m_spDefn = PYXTableDefinition::create();

		addField(
			"Name",
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(std::string("Outline Process: " + getProcName())));
	}

	// IFeatureCollection feature definiton
	m_spFeatureDefn = LibraryFeature::createFieldDefn();
}

/*!
The library outline process has a global geometry.
*/
PYXPointer<PYXGeometry> LibraryFeatureCollectionProcess::createGeometry()
{
	return PYXGlobalGeometry::create(m_nResolution);
}
