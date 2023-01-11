/******************************************************************************
hillshade_process.cpp

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "hillshade_process.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// standard includes
#include <cassert>

// {D8BFF3F3-4FFB-4676-A93D-2E6161580052}
PYXCOM_DEFINE_CLSID(HillShader, 
0xd8bff3f3, 0x4ffb, 0x4676, 0xa9, 0x3d, 0x2e, 0x61, 0x61, 0x58, 0x0, 0x52);
PYXCOM_CLASS_INTERFACES(HillShader, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(HillShader, "Shaded Relief", "A coverage that returns a shaded image based on the slope of its input coverages.", "Development/Old",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The input coverage to be shaded.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<HillShader> gTester;

}

HillShader::HillShader()
{
	// default to a greenish grey shade.
	const unsigned char nValue[3] = {128, 144, 128};
	PYXValue pyxValue(nValue, 3);
	m_colourValue = pyxValue;
}

HillShader::~HillShader()
{
}

void HillShader::test()
{
	// TODO: test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string HillShader::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"colour\">"
			"<xs:restriction base=\"xs:string\">"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"HillShaderProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"shade_colour\" type=\"colour\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Colour to Shade</friendlyName>"
					"<description>The colour to use as a base for the shading.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> HillShader::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;	
	std::ostringstream ost;
	ost << m_colourValue; 
	mapAttr["shade_colour"] = ost.str();
	return mapAttr;
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE HillShader::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it = mapAttr.find("shade_colour");
	if (it != mapAttr.end())
	{
		StringUtils::fromString(it->second, &m_colourValue);
	}
}

IProcess::eInitStatus HillShader::initImpl()
{
	m_strID = "Hill Shader: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	
	m_spCovDefn = PYXTableDefinition::create();

	m_spCovDefn->addFieldDefinition(
		"RGB", PYXFieldDefinition::knContextRGB, m_colourValue.getArrayType() ,m_colourValue.getArraySize());		

	// check that the input has a numerical definition and one channel.

	boost::intrusive_ptr<ICoverage> spCov;
	assert(getParameter(0)->getValue(0));
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &spCov);
	assert(spCov);

	m_spCov = spCov;

	PYXPointer<PYXTableDefinition> currentCov = 
		spCov->getCoverageDefinition();

	assert(0 < currentCov->getFieldCount());

	if (currentCov->getFieldCount() != 1)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Field count for input must be one.");
		return knFailedToInit;
	}

	if (!currentCov->getFieldDefinition(0).isNumeric())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Non-numeric input to the hill shader detected.");
		return knFailedToInit;
	}

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue HillShader::getCoverageValue(	const PYXIcosIndex& index,
											int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// Return a null value if we are outside our geometry.
	if (!getGeometry()->intersects(PYXCell(index)))
	{
		return PYXValue();
	}

	// TODO: support getting data one cell at a time.  possible by doing a whole tile, and then
	// picking the one piece of data out of it.

	return PYXValue();
}

void CalculatePartialTile (int firstIndex, int lastIndex, 
						   PYXPointer<const PYXGeometry> spGeometry,
						   PYXValueTile* spInputTile,
						   PYXValueTile* spValueTile,
						   PYXValue baseColourValue,
						   PYXMath::eHexDirection nHexDirection)
{
	// TODO: we should be able to be more efficient than this -- look for
	// or add support for setting the index in the iterator to part way along.
	PYXPointer<PYXIterator> it = spValueTile->getIterator();
	for (int nCell = 0; nCell < firstIndex; ++nCell)
	{
		it->next();
	}

	PYXValue currentValue = spInputTile->getTypeCompatibleValue(0);
	PYXValue offsetValue = spInputTile->getTypeCompatibleValue(0);
	PYXValue outputValue = baseColourValue;

	PYXIcosIndex inputTileIndex = spValueTile->getTile().getRootIndex();
	PYXMath::eHexDirection nOppositeHexDirection = PYXMath::negateDir(nHexDirection);
	double cellDistance = PYXMath::calcInterCellDistance(spValueTile->getTile().getCellResolution()) * 4434026.26;
	const unsigned char nBlack[3] = {0, 0, 0};
	PYXValue black(nBlack, 3);
	const unsigned char nWhite[3] = {255, 255, 255};
	PYXValue white(nWhite, 3);
	PYXValue shade(nWhite, 3);

	for (int nCell = firstIndex; nCell <= lastIndex; ++nCell)
	{
		PYXIcosIndex currentIndex = it->getIndex();
		bool bIsPartOfThisDataSet = spGeometry->intersects(PYXCell(currentIndex));
		it->next();
		if (bIsPartOfThisDataSet)
		{
			// get the value at the current position
			if (spInputTile->getValue(nCell, 0, &currentValue))
			{
				double slopeMultiplier = 0.0;
				
				// move in the direction that we need to go from the current index
				PYXIcosIndex offsetIndex = 
					PYXIcosMath::move(currentIndex, nHexDirection);

				// get the value at the offset position
				if (offsetIndex.isDescendantOf(inputTileIndex))
				{
					if (spInputTile->getValue(offsetIndex, 0, &offsetValue))
					{
						slopeMultiplier = 1.0;
					}
				}
				else
				{
					offsetIndex = 
						PYXIcosMath::move(currentIndex, nOppositeHexDirection);
					if (offsetIndex.isDescendantOf(inputTileIndex))
					{
						if (spInputTile->getValue(offsetIndex, 0, &offsetValue))
						{
							slopeMultiplier = -1.0;
						}
					}
				}
			
				// get the value, subtract the values and divide by the run.
				double slope = (currentValue.getDouble() - offsetValue.getDouble()) * slopeMultiplier;
				slope = slope / cellDistance;

				if (slope == 0.0)
				{
					spValueTile->setValue(nCell, 0, baseColourValue);
				}
				else
				{
					if (slope > 1.0)
					{
						// use the slope to scale between the baseColourValue and 255,255,255 if greater than 0.
						if (slope > 1.0)
						{
							spValueTile->setValue(nCell, 0, white);
						}
						else
						{
							for (int index = 0; index < 3; index += 1)
							{
								double shadeval = ((white.getDouble(index) - baseColourValue.getDouble(index)) * slope / 1.0)
									+ baseColourValue.getDouble(index);
								shade.setDouble(index, shadeval);
							}
							spValueTile->setValue(nCell, 0, shade);
						}
					}
					else
					{
						// use the slope to scale between the baseColourValue and 0,0,0 if greater less 0.
						if (slope < -1.0)
						{
							spValueTile->setValue(nCell, 0, black);
						}
						else
						{
							for (int index = 0; index < 3; index += 1)
							{
								double shadeval = baseColourValue.getDouble(index) -
									((baseColourValue.getDouble(index) - black.getDouble(index)) * slope / -1.0);
								shade.setDouble(index, shadeval);
							}
							spValueTile->setValue(nCell, 0, shade);
						}
					}
				}
			}
		}
	}
}

PYXPointer<PYXValueTile> HillShader::getFieldTile(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// If we don't have data for any of this tile return a null tile.
	PYXPointer<const PYXGeometry> spGeometry = getGeometry();
	if (!spGeometry->intersects(PYXTile(index, nRes)))
	{
		return PYXPointer<PYXValueTile>();
	}

	// our goal tile, which we are constructing	
	const PYXFieldDefinition& fieldDefn = getCoverageDefinition()->getFieldDefinition(nFieldIndex);
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, getCoverageDefinition());

	// get data from our source.
	PYXPointer<PYXValueTile> spInputTile = m_spCov->getFieldTile(index, nRes, nFieldIndex);

	// TODO:  find the direction that we want to use for the hill shading in this tile.
	// suggest something like one diredtion counter clockwise from north.  This will give 
	// a fairly consistent shade from tile to tile.

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
		//CalculatePartialTile(
		//		firstIndex, lastIndex,
		//		spGeometry, spInputTile.get(), 
		//		spValueTile.get(),
		//		m_colourValue,
		//		PYXMath::eHexDirection::knDirectionOne);

		threads.create_thread(
			boost::bind (&CalculatePartialTile,
				firstIndex, lastIndex,
				spGeometry, spInputTile.get(), 
				spValueTile.get(),
				m_colourValue,
				PYXMath::knDirectionOne));
		firstIndex += cellsPerThread;
	}

	// wait for all the threads to finish.
	threads.join_all();

	return spValueTile;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void HillShader::createGeometry() const
{
	m_spGeom = m_spCov->getGeometry()->clone();
}
