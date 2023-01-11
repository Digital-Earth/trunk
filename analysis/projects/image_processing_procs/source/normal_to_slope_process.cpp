/******************************************************************************
normal_to_rgb_process.cpp

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "normal_to_slope_process.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/math_utils.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// standard includes
#include <cassert>


// {A916F98B-E4A2-4563-BB14-5CA04699CAA3}
PYXCOM_DEFINE_CLSID(NormalToSlopeProcess, 
0xa916f98b, 0xe4a2, 0x4563, 0xbb, 0x14, 0x5c, 0xa0, 0x46, 0x99, 0xca, 0xa3);

PYXCOM_CLASS_INTERFACES(NormalToSlopeProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(NormalToSlopeProcess, "Normal To Slope", "A coverage that takes normal values and converts them to slope and aspect values.", "Analysis/Elevations",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Elevation Coverage", "The surface normal input coverage.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<NormalToSlopeProcess> gTester;

}

NormalToSlopeProcess::NormalToSlopeProcess()
{	
	m_outputType = knSlope;
}

NormalToSlopeProcess::~NormalToSlopeProcess()
{
}

void NormalToSlopeProcess::test()
{
	//TODO [shatzi]: add some unit testing!
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus NormalToSlopeProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Normal to Slope " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the input coverage.
	m_spCov = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &m_spCov	);
	
	if (!m_spCov)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage not set");
		return knFailedToInit;
	}

	// Get the input coverage definition.
	PYXPointer<PYXTableDefinition> spCovDefn = m_spCov->getCoverageDefinition();

	// Create a new field definition, in which the greyscale fields are verified and converted to rgb.
	m_spCovDefn = PYXTableDefinition::create();
	if (spCovDefn->getFieldCount() == 0)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage should always have at least one field");
		return knFailedToInit;
	}
	else
	{
		for (int nOffset = 0; nOffset < spCovDefn->getFieldCount(); ++nOffset)
		{
			// For each greyscale UInt8[1] field, add a 3 RGB UInt8[3] field.
			const PYXFieldDefinition& field = spCovDefn->getFieldDefinition(nOffset);
			if (field.getContext() == PYXFieldDefinition::knContextNormal)
			{				
				switch(m_outputType)
				{
				case knSlope:
					m_spCovDefn->addFieldDefinition("Slope", PYXFieldDefinition::knContextNone, PYXValue::knDouble, 1 );				
					break;
				case knAspect:
					m_spCovDefn->addFieldDefinition("Aspect", PYXFieldDefinition::knContextNone, PYXValue::knDouble, 1 );				
					break;
				}
			}
			else
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Expected a field '" + StringUtils::toString(nOffset) + "' to be normal.");
				return knFailedToInit;
			}
		}
	}
	return knInitialized;
}

//! Get the attributes in this process.
std::map<std::string, std::string> STDMETHODCALLTYPE NormalToSlopeProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	switch (m_outputType)
	{
	case knSlope:
		mapAttr["Output"] = "Slope";
		break;
	case knAspect:
		mapAttr["Output"] = "Aspect";
		break;
	}

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE NormalToSlopeProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	std::string output;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Output",output);
	

	if (output == "Aspect")
	{
		m_outputType = knAspect;
	} 
	else 
	{
		m_outputType = knSlope;
	}
}


std::string STDMETHODCALLTYPE NormalToSlopeProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"OutputType\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"Slope\" />"
				"<xs:enumeration value=\"Aspect\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"NormalToSlopeProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Output\" type=\"OutputType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Output</friendlyName>"
					"<description>Define if output is slope or aspect.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue NormalToSlopeProcess::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	assert(m_spCov);
	return convert(index, m_spCov->getCoverageValue(index, nFieldIndex / 3), nFieldIndex % 3);
}

PYXPointer<PYXValueTile> NormalToSlopeProcess::getFieldTile(
	const PYXIcosIndex& index,
	int nRes,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	assert(m_spCov);

	// get the tile from the input coverage
	PYXPointer<PYXValueTile> spInputValueTile = m_spCov->getFieldTile(index, nRes, nFieldIndex / 3);

	// Return null tiles right away.
	if (!spInputValueTile)
	{
		return spInputValueTile;
	}

	// create the tile to return to the caller
	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

	int nCellCount = spValueTile->getNumberOfCells();
	int cellsPerThread = (nCellCount + N_THREADS - 1) / N_THREADS;
	int firstIndex = 0;
	boost::thread_group threads;
	for (int count = 0; count < N_THREADS; ++count)
	{
		int lastIndex = firstIndex + cellsPerThread - 1;
		if (lastIndex >= nCellCount)
		{
			lastIndex = nCellCount - 1;
		}

		// just call it directly if you want single threaded... and comment out the create_thread below
		//calculatePartialTile(firstIndex,lastIndex,index,spInputValueTile.get(),spValueTile.get(),nFieldIndex,nRes);

		threads.create_thread(
			boost::bind (&NormalToSlopeProcess::calculatePartialTile,this,
				firstIndex, lastIndex, index, spInputValueTile.get(), spValueTile.get(), nFieldIndex, nRes));
				
		firstIndex += cellsPerThread;
	}

	// wait for all the threads to finish.
	threads.join_all();
	
	return spValueTile;
}

void NormalToSlopeProcess::calculatePartialTile (int firstIndex, int lastIndex,
							   const PYXIcosIndex & index,
							   PYXValueTile* spInputTile,
						       PYXValueTile* spOutputTile,
							   int nFieldIndex,int nRes) const
{
	// convert all of the values from the input coverage tile
	PYXValue inVal = m_spCov->getCoverageDefinition()->getFieldDefinition(nFieldIndex / 3).getTypeCompatibleValue();
	PYXValue outVal = getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	
	// TODO: we should be able to be more efficient than this -- look for
	// or add support for setting the index in the iterator to part way along.
	PYXPointer<PYXIterator> it = spOutputTile->getIterator();

	for (int n = 0; n < firstIndex; ++n)
	{
		it->next();
	}	
	
	for (int n = firstIndex; n <= lastIndex; ++n)
	{
		PYXIcosIndex curIndex(it->getIndex());
		it->next();
	
		if (spInputTile->getValue(n, 0, &inVal))
		{			
			spOutputTile->setValue(n, 0, convert(curIndex,inVal, nFieldIndex % 3));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void NormalToSlopeProcess::createGeometry() const
{
	assert(m_spCov);
	PYXPointer<PYXGeometry> spGeometry = m_spCov->getGeometry();
	if (!spGeometry)
	{
		m_spGeom = PYXEmptyGeometry::create();
	}
	else
	{
		m_spGeom = spGeometry->clone();
	}
}

void NormalToSlopeProcess::extractSlopeAndDirectionAndLightDirection(
		const PYXCoord3DDouble & xyz,
		const PYXCoord3DDouble & normal,
		double * slope,
		double * direction) const
{
	PYXCoord3DDouble north(0,0,1);

	PYXCoord3DDouble left = north.cross(xyz);
	left.normalize();

	PYXCoord3DDouble up   = xyz.cross(left);
	up.normalize();

	double x = normal.dot(left);
	double y = normal.dot(up);
	
	*slope = xyz.dot(normal);
	*direction = atan2(-x,-y); //we want +Y to be zero (which is north)
}

//! Convert a greyscale value to an RGB value.
PYXValue NormalToSlopeProcess::convert(const PYXIcosIndex & index, const PYXValue& valIn, int nFieldIndex) const
{
	if (!valIn.isNull())
	{
		CoordLatLon latLon;

		SnyderProjection::getInstance()->pyxisToNative(index,&latLon);
		PYXCoord3DDouble xyz,normal(valIn.getDouble(0),valIn.getDouble(1),valIn.getDouble(2));
		SphereMath::llxyz(latLon,&xyz);

		double slope;
		double direction;

		extractSlopeAndDirectionAndLightDirection(xyz,normal,&slope,&direction);

		//normalize direction
		direction = (direction/MathUtils::kf180Rad+1)/2;
		
		//calculate angle
		double slopeAngle = acos(slope);

		switch (m_outputType)
		{		
		case knSlope:
			return PYXValue(100*atan(slopeAngle));
		case knAspect:
			return PYXValue(direction*360);
		}
	}
	return valIn;
}