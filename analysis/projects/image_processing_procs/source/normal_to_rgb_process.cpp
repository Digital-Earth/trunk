/******************************************************************************
normal_to_rgb_process.cpp

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "normal_to_rgb_process.h"

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

#define Airborne_Imaging_Demo

// {76FEEB10-69F8-4af9-8A22-FFDEFB72F552}
PYXCOM_DEFINE_CLSID(NormalToRGBProcess, 
0x76feeb10, 0x69f8, 0x4af9, 0x8a, 0x22, 0xff, 0xde, 0xfb, 0x72, 0xf5, 0x52);

PYXCOM_CLASS_INTERFACES(NormalToRGBProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(NormalToRGBProcess, "Shaded Relief from Normal", "A coverage that takes normal values and converts them to RGB color.", "Analysis/Elevations",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Elevation Coverage", "The surface normal input coverage.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<NormalToRGBProcess> gTester;

}

NormalToRGBProcess::NormalToRGBProcess()
{	
	m_paletteName = "HSV";
	m_palette = HSVPalette::create();
	
	m_lightAngle = 0; //up
	m_lightAzimuth = 0; //north;	
	m_shadingMode = knShadeSlopeAndDirection;	

	cacluateLightFactors();	
}

NormalToRGBProcess::~NormalToRGBProcess()
{
}

void NormalToRGBProcess::test()
{
	//TODO [shatzi]: add some unit testing!
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus NormalToRGBProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Normal to RGB" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

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
				m_spCovDefn->addFieldDefinition(
					field.getName(), PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3	);

#ifdef Airborne_Imaging_Demo

				m_spCovDefn->addFieldDefinition(
					"Slope", PYXFieldDefinition::knContextNone, PYXValue::knDouble, 1);

				m_spCovDefn->addFieldDefinition(
					"Direction", PYXFieldDefinition::knContextNone, PYXValue::knDouble, 1);

#endif
				
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
std::map<std::string, std::string> STDMETHODCALLTYPE NormalToRGBProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["Palette"] = m_paletteName;	
	mapAttr["LightAngle"] = StringUtils::toString(m_lightAngle);
	mapAttr["LightAzimuth"] = StringUtils::toString(m_lightAzimuth);

	switch (m_shadingMode)
	{
	case knShadeSlope:
		mapAttr["Mode"] = "Slope Only";
		break;
	case knShadeDirection:
		mapAttr["Mode"] = "Direction Only";
		break;
	case knShadeSlopeAndDirection:
		mapAttr["Mode"] = "Slope And Direction";
		break;
	case knHillShade:
		mapAttr["Mode"] = "Shaded Relief";
		break;		
	}

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE NormalToRGBProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"LightAngle",int,m_lightAngle);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"LightAzimuth",int,m_lightAzimuth);
	cacluateLightFactors();

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Palette",m_paletteName);
	
	std::string mode;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Mode",mode);
	

	if (m_paletteName == "HSV")
	{
		m_palette = HSVPalette::create();
	} 
	else if (m_paletteName == "Green to Red")
	{
		m_palette = GreenToRedPalette::create();
	}
	else 
	{
		m_palette = GrayscalePalette::create();
	}

	if (mode == "Slope Only")
	{
		m_shadingMode = knShadeSlope;
	}
	else if (mode == "Direction Only")
	{
		m_shadingMode = knShadeDirection;
	}
	else if (mode == "Shaded Relief")
	{
		m_shadingMode = knHillShade;
	}	
	else
	{
		m_shadingMode = knShadeSlopeAndDirection;
	}
}

void NormalToRGBProcess::cacluateLightFactors()
{
	m_lightSourceEastFactor = sin(MathUtils::degreesToRadians(m_lightAzimuth));
	m_lightSourceNorthFactor = cos(MathUtils::degreesToRadians(m_lightAzimuth));
	m_lightSourceForwadFactor = sin(MathUtils::degreesToRadians(m_lightAngle));
	m_lightSourceUpFactor = cos(MathUtils::degreesToRadians(m_lightAngle));
}


std::string STDMETHODCALLTYPE NormalToRGBProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"PaletteType\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"Grayscale\" />"
				"<xs:enumeration value=\"HSV\" />"
				"<xs:enumeration value=\"Green to Red\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:simpleType name=\"ShadeMode\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"Slope Only\" />"
				"<xs:enumeration value=\"Direction Only\" />"
				"<xs:enumeration value=\"Slope And Direction\" />"
				"<xs:enumeration value=\"Shaded Relief\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"NormalToRGBProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Palette\" type=\"PaletteType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Slope Palette</friendlyName>"
					"<description>A color palette for slope visualization.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			  "<xs:element name=\"Mode\" type=\"ShadeMode\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Shading Mode</friendlyName>"
					"<description>The shading method to use for visualization.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>" 
			  "<xs:element name=\"LightAngle\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Light Source Angle</friendlyName>"
					"<description>A vertical angle for the light source (used only in Shaded Relief mode).</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>" 
			  "<xs:element name=\"LightAzimuth\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Light Source Azimuth</friendlyName>"
					"<description>An Azimuth of the light source (used only in Shaded Relief mode).</description>"
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

PYXValue NormalToRGBProcess::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	assert(m_spCov);
	return convert(index, m_spCov->getCoverageValue(index, nFieldIndex / 3), nFieldIndex % 3);
}

PYXPointer<PYXValueTile> NormalToRGBProcess::getFieldTile(
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
			boost::bind (&NormalToRGBProcess::calculatePartialTile,this,
				firstIndex, lastIndex, index, spInputValueTile.get(), spValueTile.get(), nFieldIndex, nRes));
				
		firstIndex += cellsPerThread;
	}

	// wait for all the threads to finish.
	threads.join_all();
	
	return spValueTile;
}

void NormalToRGBProcess::calculatePartialTile (int firstIndex, int lastIndex,
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

void NormalToRGBProcess::createGeometry() const
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

void NormalToRGBProcess::extractSlopeAndDirectionAndLightDirection(
		const PYXCoord3DDouble & xyz,
		const PYXCoord3DDouble & normal,
		double * slope,
		double * direction, 
		PYXCoord3DDouble * lightDirection) const
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

	//find light direction;
	left.scale(m_lightSourceEastFactor);
	up.scale(m_lightSourceNorthFactor);

	lightDirection->set(
		(left[0]+up[0])*m_lightSourceForwadFactor+xyz[0]*m_lightSourceUpFactor,
		(left[1]+up[1])*m_lightSourceForwadFactor+xyz[1]*m_lightSourceUpFactor,
		(left[2]+up[2])*m_lightSourceForwadFactor+xyz[2]*m_lightSourceUpFactor);
}

//! Convert a greyscale value to an RGB value.
PYXValue NormalToRGBProcess::convert(const PYXIcosIndex & index, const PYXValue& valIn, int nFieldIndex) const
{
	if (!valIn.isNull())
	{
		CoordLatLon latLon;

		SnyderProjection::getInstance()->pyxisToNative(index,&latLon);
		PYXCoord3DDouble xyz,normal(valIn.getDouble(0),valIn.getDouble(1),valIn.getDouble(2));
		SphereMath::llxyz(latLon,&xyz);

		double slope;
		double direction;
		PYXCoord3DDouble lightDirection;

		extractSlopeAndDirectionAndLightDirection(xyz,normal,&slope,&direction,&lightDirection);

		//normalize direction
		direction = (direction/MathUtils::kf180Rad+1)/2;
		
		//calculate angle
		double slopeAngle = acos(slope);

#ifdef Airborne_Imaging_Demo

		switch (nFieldIndex % 3)
		{		
		case 1:
			return PYXValue(MathUtils::radiansToDegrees(slopeAngle));
		case 2:
			return PYXValue(direction*360);
		}

#endif

		//convert it between 0 and 1.
		slopeAngle /= MathUtils::kf90Rad;
		
		//create some color
		uint8_t buf[3];

		switch (m_shadingMode)
		{
		case knShadeDirection:			
			m_palette->convert(direction,buf);
			break;
		case knShadeSlope:
			m_palette->convert(1-std::max(0.0,std::min(slopeAngle,1.0)),buf);			
			break;
		case knShadeSlopeAndDirection:
			m_palette->convert(std::max(0.0,std::min(direction,1.0)),buf);
			slopeAngle = std::max(0.0,std::min(slopeAngle,1.0));
			if (slopeAngle<0.2)
			{
				//fade to white
				slopeAngle /= 0.2;
				buf[0] = static_cast<uint8_t>(buf[0]*(slopeAngle) + 255*(1-slopeAngle));
				buf[1] = static_cast<uint8_t>(buf[1]*(slopeAngle) + 255*(1-slopeAngle));
				buf[2] = static_cast<uint8_t>(buf[2]*(slopeAngle) + 255*(1-slopeAngle));
			} 
			else
			{
				//fade to black
				slopeAngle = (slopeAngle-0.2)/(1-0.2);
				buf[0] = static_cast<uint8_t>(buf[0]*(1-slopeAngle));
				buf[1] = static_cast<uint8_t>(buf[1]*(1-slopeAngle));
				buf[2] = static_cast<uint8_t>(buf[2]*(1-slopeAngle));
			}
			break;
		case knHillShade:
			m_palette->convert(std::max(0.0,std::min(lightDirection.dot(normal),1.0)),buf);
			break;
		}
		
		return PYXValue(buf, 3);
	}
	return valIn;
}

void GrayscalePalette::convert(double position,uint8_t *rgb) 
{
	uint8_t level = (uint8_t)(255*position);

	rgb[0] = level;
	rgb[1] = level;
	rgb[2] = level;
}

void HSVPalette::convert(double position,uint8_t *rgb) 
{
	int section = (int)floor(position*6);
		
	uint8_t one  = 255;
	uint8_t zero = 0;
	uint8_t step = (uint8_t)((6*position-section)*255);
	
	//create some color
	
	switch (section)
	{
	case 0:
		rgb[0] = one;
		rgb[1] = step;
		rgb[2] = zero;
		break;
	case 1:
		rgb[0] = one-step;
		rgb[1] = one;
		rgb[2] = zero;
		break;
	case 2:
		rgb[0] = zero;
		rgb[1] = one;
		rgb[2] = step;
		break;
	case 3:
		rgb[0] = zero;
		rgb[1] = one-step;
		rgb[2] = one;
		break;
	case 4:
		rgb[0] = step;
		rgb[1] = zero;
		rgb[2] = one;
		break;
	case 5:
		rgb[0] = one;
		rgb[1] = zero;
		rgb[2] = one-step;
		break;
	case 6:
		rgb[0] = one;
		rgb[1] = zero;
		rgb[2] = zero;
		break;
	}
}

void GreenToRedPalette::convert(double position,uint8_t *rgb) 
{
	uint8_t level = (uint8_t)(255*position);
	rgb[0] = level;
	rgb[1] = 255-level;
	rgb[2] = 0;
}