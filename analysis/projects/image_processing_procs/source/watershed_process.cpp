/******************************************************************************
watershed_process.cpp

begin		: 2015-09-07
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "watershed_process.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/iterator_linq.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/region/multi_curve_region.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// standard includes
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
// Watershed Algorithm
////////////////////////////////////////////////////////////////////////////////
//
// Watershed algorithm is a weighted variation on the floodfill algorithm.
// In the fill flood algorithm, we move from a cell to it's neighboring cells.
//
// On the watershed algorithm, we move from a cell to it's neighbor if the neighbor
// cell has higher elevation. 
//
// Because elevation dataset have different way of been generated (average value,
// max value, lowest value, creek simulation etc) - we need to "fix" the elevation
// model so our algorithm will run as expected.
//
// "Fixing" elevation means resolving "local-minimum" cells. a "local-minimum" is 
// a cell that has elevation lower than all its neighbors. those cells are caused
// from "lower resolution" estimation of elevation. but in reality, there would be
// a creek that will allow the water to flow or it will be a lake. we would like
// to fix the elevation model to find where water in this cell would probably 
// flow. 
//
// The "Fix" is to dig a tunnel from a "local-minimum" cell to "nearest" cell with 
// lower elevation (could be few cells away). the "nearest" cell is not calculated
// by distance, but it calculated by the amount of "dirt" we will need to dig to
// create a tunnel between those 2 cells.
//
// Once we found the best "tunnel" to to fix that local-minimum", we create
// a temporary and modified elevation model with that tunnel. And continue to fix
// the next local-minimum cell in the area. 
//
// The implementation heavily uses: PYXIteratorLinq and boost::bind
//
////////////////////////////////////////////////////////////////////////////////

PYXCoord3DDouble toCoord3D(const PYXIcosIndex & index)
{
	PYXCoord3DDouble coord;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &coord);

	return coord;	
}


/*
isLowerElevation - return if the elevation (taken from elevationCoverage) at target position is lower the given value (sourceElevation)

boost::bind will be used to inject elevationCoverage and sourceElevation
*/
bool isLowerElevation(const PYXIcosIndex & target,const boost::intrusive_ptr<ICoverage> & elevationCoverage,const PYXValue & sourceElevation)
{
	PYXValue value = elevationCoverage->getCoverageValue(target,0);
	if (value.isNull())
	{
		return true;
	}
	return value.getDouble() < sourceElevation.getDouble();
}

/*
elevationDiffCost - return the watershed cost of moving between two positions (from - to)

the cost is equal to amount of "dirt" we will need to dig. aka - the difference between elevation (taken form elevationCoverage) at "to" location and the
starting elevation (sourceElevation)

boost::bind will be used to inject elevationCoverage and sourceElevation
*/
double elevationDiffCost(const PYXIcosIndex & from,const PYXIcosIndex & to,const boost::intrusive_ptr<ICoverage> & elevationCoverage,const PYXValue & sourceElevation)
{
	return elevationCoverage->getCoverageValue(to,0).getDouble() - sourceElevation.getDouble();
}

/*
maxCost - aggregate the cost. in our case is the highest cost between the totalCost and moveCost.
*/
double maxCost(double totalCost,double moveCost)
{
	return 0.0001 + std::max(totalCost,moveCost);
}

/*
findPathNearestLowerPlace - find the tunnel from location (index) to another cell that is lower.

See "watershed algorithm" details above.
*/
PYXPointer<PYXIterator> findPathNearestLowerPlace(const PYXIcosIndex & index,const boost::intrusive_ptr<ICoverage> & elevationCoverage)
{
	PYXValue sourceElevation = elevationCoverage->getCoverageValue(index,0);
	return PYXIteratorLinq::findShortestPath(index,
		boost::bind(isLowerElevation,_1,elevationCoverage,sourceElevation),
		boost::bind(elevationDiffCost,_1,_2,elevationCoverage,sourceElevation),
		boost::bind(maxCost,_1,_2));
}

/*
fixElevation - fix elevation for a single location (source) - "local-minimum" cell.

Note: this function modifies elevationCoverage by digging a tunnel from the source location to the nearest lower elevation cell.

See "watershed algorithm" details above.
*/
void fixElevation(const PYXIcosIndex & source,const boost::intrusive_ptr<ICoverage> & elevationCoverage)
{
	std::list<PYXIcosIndex> way;
	PYXIteratorLinq(findPathNearestLowerPlace(source,elevationCoverage)).pushBackToList(way);

	double startElevation = elevationCoverage->getCoverageValue(*way.begin(),0).getDouble();
	double endElevation = elevationCoverage->getCoverageValue(*way.rbegin(),0).getDouble();
	int length = (int)way.size();

	double delta = endElevation-startElevation;

	if (startElevation<=endElevation)
	{
		//we have nothing to fix, this can happen when shortest path reach a null value.
		return;
	}

	int i=0;
	PYXValue val = elevationCoverage->getCoverageDefinition()->getFieldDefinition(0).getTypeCompatibleValue();
	for(std::list<PYXIcosIndex>::iterator it = way.begin(); it != way.end();++it,++i)
	{
		val.setDouble(startElevation + (delta*i)/(length-1));
		assert(startElevation + (delta*i)/(length-1) >= endElevation-0.0001);
		elevationCoverage->setCoverageValue(val,*it,0);
	}
}

/*
applyFixArea - fix elevation for a large area

Note: this function modifies elevationCoverage by digging a tunnels from all local minimum cells in the given area

See "watershed algorithm" details above.
*/
void applyFixArea(const PYXIcosIndex & rootIndex,const boost::intrusive_ptr<ICoverage> & elevationCoverage,const PYXPointer<PYXTileCollection> fixedArea)
{
	if (!fixedArea->isEmpty() && fixedArea->intersects(rootIndex))
	{
		//we already fixed this area...
		return;
	}
	PYXIcosIndex tileIndex = rootIndex;
	tileIndex.setResolution(rootIndex.getResolution()-9);
	PYXIteratorLinq::fromGeometry(PYXVectorGeometry2::create(PYXCircleRegion::create(toCoord3D(rootIndex),PYXIcosMath::UnitSphere::calcTileCircumRadius(tileIndex)),rootIndex.getResolution()))
		.filter(_value.localMinimumOf(elevationCoverage,0))
		.orderBy(_value.of(elevationCoverage))
		.foreach(boost::bind(fixElevation,_1,elevationCoverage));

	PYXPointer<PYXTileCollection> safeArea = PYXTileCollection::create();
	PYXVectorGeometry2::create(PYXCircleRegion::create(toCoord3D(rootIndex),PYXIcosMath::UnitSphere::calcTileCircumRadius(tileIndex)/3),rootIndex.getResolution())->copyTo(safeArea.get());

	fixedArea->addGeometry(*safeArea);
}

/*
watershedFlow - check the water will flow from cell (to) into cell (from).

I think the name are kind of confusing. but this function is used by "calculateWatershedFlow" function to visualize the watershed flow.

the building of that flow is building from the destination to the source of the watershed. Therefore, the from and to are used to mark
how the algorithm is tracing back the watershed, which is in reverse how the water is flowing. 
*/
bool watershedFlow(const PYXIcosIndex & from,const PYXIcosIndex & to,const boost::intrusive_ptr<ICoverage> & elevationCoverage)
{
	PYXValue fromValue = elevationCoverage->getCoverageValue(from,0);
	PYXValue toValue = elevationCoverage->getCoverageValue(to,0);
	return fromValue < toValue && PYXIteratorLinq::fromNeighboursWithoutSelf(to).select(elevationCoverage).minValueIndex() == from;
}

/*
showElevationFix - debug function to highlight the elevation fixes.

this function return a geometry of all cells that their elevation has been modified.
*/
PYXPointer<PYXGeometry> showElevationFix(const boost::intrusive_ptr<IProcess> & elevationProc,const PYXIcosIndex & rootIndex)
{
	PYXIcosIndex tileIndex = rootIndex;
	tileIndex.setResolution(tileIndex.getResolution()-11);

	boost::intrusive_ptr<ICoverage> elevationCoverage = elevationProc->getOutput()->QueryInterface<ICoverage>();

	boost::intrusive_ptr<ICoverage> modifiedElevation = new PYXTemporaryCoverage(elevationCoverage);

	//return PYXIteratorLinq::fromGeometry(PYXTile(tileIndex,rootIndex.getResolution()))
	return PYXIteratorLinq::fromGeometry(PYXVectorGeometry2::create(PYXCircleRegion::create(toCoord3D(rootIndex),PYXIcosMath::UnitSphere::calcTileCircumRadius(tileIndex)),rootIndex.getResolution()))
		.filter(_value.localMinimumOf(modifiedElevation,0))
		.selectMany(boost::bind(findPathNearestLowerPlace,_1,modifiedElevation))
		.toGeometry();
}

/*
findResolutionToPerformWatershedAnalysis - find a good resolution to perform watershed analysis
*/
PYXIcosIndex findResolutionToPerformWatershedAnalysis(const boost::intrusive_ptr<ICoverage> & elevationCoverage,const PYXIcosIndex & rootIndex) 
{
	PYXIcosIndex root = rootIndex;

	//Watershed analysis only provides value when performed at a resolution less than the elevation pipeline.
	//therefore, there is no need to perform watershed analysis on resolutions larger than the native resolution
	auto maxResolution = elevationCoverage->getGeometry()->getCellResolution() - 1;
	if (root.getResolution() > maxResolution)
	{
		root.setResolution(maxResolution);
	}

	//if we are given a root with resolution lower the knDefaultTileDepth we align it to that	
	if (root.getResolution() < PYXTile::knDefaultTileDepth) 
	{
		root.setResolution(PYXTile::knDefaultTileDepth);
	}
	return root;
}

/*
calculateWatershed - calculate a watershed for a given elevation and a destination cell
*/
PYXPointer<PYXGeometry> calculateWatershed(const boost::intrusive_ptr<IProcess> & elevationProc,const PYXIcosIndex & rootIndex)
{
	//create watershed creation state
	boost::intrusive_ptr<ICoverage> elevationCoverage = elevationProc->getOutput()->QueryInterface<ICoverage>();
	PYXIcosIndex root = findResolutionToPerformWatershedAnalysis(elevationCoverage, rootIndex);
	boost::intrusive_ptr<ICoverage> modifiedElevation = new PYXTemporaryCoverage(elevationCoverage);	
	PYXPointer<PYXTileCollection> fixedArea = PYXTileCollection::create();

	//fix DEM around the root
	applyFixArea(root,modifiedElevation,fixedArea);

	//find min elevation neighbor (2xcell width) - move root to nearest place - min elevation means for larger watershed
	PYXIcosIndex betterRoot = PYXIteratorLinq::fromSelfAndNeighbours(root).select(modifiedElevation).minValueIndex();
	betterRoot = PYXIteratorLinq::fromSelfAndNeighbours(betterRoot).select(modifiedElevation).minValueIndex();

	return PYXIteratorLinq::fromIndex(betterRoot)
		.floodFill(boost::bind(watershedFlow,_1,_2,modifiedElevation))
		.beforeEachItem(boost::bind(applyFixArea,_1,modifiedElevation,fixedArea))
		.toGeometry();
}

/*
addFlowCurve - add a single geometry curve to the result geometry that mark how the water flow in the watershed.
*/
void addFlowCurve(std::vector<PYXPointer<PYXCurveRegion>> & curves,const PYXIcosIndex & index,const boost::intrusive_ptr<ICoverage> & elevationCoverage)
{
	auto dest = PYXIteratorLinq::fromNeighboursWithoutSelf(index).select(elevationCoverage).minValueIndex();
	if (elevationCoverage->getCoverageValue(dest,0) < elevationCoverage->getCoverageValue(index,0))
	{
		curves.push_back(PYXCurveRegion::create(toCoord3D(index),toCoord3D(dest)));
	}
}

/*
calculateWatershedFlow - calculate a watershed and generate a watershed flow geometry

watershed flow geometry is a multi-curve geometry that shows how the water flow between cells.

it uses "watershedFlow" function to find the connective between all cells in the watershed.

*/
PYXPointer<PYXGeometry> calculateWatershedFlow(const boost::intrusive_ptr<IProcess> & elevationProc,const PYXIcosIndex & rootIndex)
{
	//create watershed creation state
	boost::intrusive_ptr<ICoverage> elevationCoverage = elevationProc->getOutput()->QueryInterface<ICoverage>();
	PYXIcosIndex root = findResolutionToPerformWatershedAnalysis(elevationCoverage, rootIndex);
	boost::intrusive_ptr<ICoverage> modifiedElevation = new PYXTemporaryCoverage(elevationCoverage);
	PYXPointer<PYXTileCollection> fixedArea = PYXTileCollection::create();

	//fix DEM around the root
	applyFixArea(root,modifiedElevation,fixedArea);

	//find min elevation neighbor (2xcell width) - move rootIndex to nearest place - min elevation means for larger watershed
	PYXIcosIndex betterRoot = PYXIteratorLinq::fromSelfAndNeighbours(root).select(modifiedElevation).minValueIndex();
	betterRoot = PYXIteratorLinq::fromSelfAndNeighbours(betterRoot).select(modifiedElevation).minValueIndex();

	auto watershed = PYXIteratorLinq::fromIndex(betterRoot)
		.floodFill(boost::bind(watershedFlow,_1,_2,modifiedElevation))
		.beforeEachItem(boost::bind(applyFixArea,_1,modifiedElevation,fixedArea))
		.toGeometry();

	std::vector<PYXPointer<PYXCurveRegion>> curves;
	PYXIteratorLinq::fromGeometry(watershed).foreach(boost::bind(addFlowCurve,boost::ref(curves),_1,modifiedElevation));

	auto geometry = PYXVectorGeometry2::create(PYXMultiCurveRegion::create(curves),root.getResolution());

	geometry->getInnerTileIterator(PYXInnerTile(root,root.getResolution()+1));

	return geometry;
}

////////////////////////////////////////////////////////////////////////////////
// WatershedProcess
////////////////////////////////////////////////////////////////////////////////

// {4D367363-2421-4DF2-AD97-529A4EDDE0A8}
PYXCOM_DEFINE_CLSID(WatershedProcess, 
0x4d367363, 0x2421, 0x4df2, 0xad, 0x97, 0x52, 0x9a, 0x4e, 0xdd, 0xe0, 0xa8);

PYXCOM_CLASS_INTERFACES(WatershedProcess, IProcess::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(WatershedProcess, "Watershed calculator", "Calculate a watershed to a given geometry based on elevation coverage.", "Analysis/Elevations",
					IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Elevation Coverage", "The surface elevation input coverage.")
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Destination Geometry", "The destination of the watershed geometry.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<WatershedProcess> gTester;

}

WatershedProcess::WatershedProcess()
{
}

WatershedProcess::~WatershedProcess()
{
}

void WatershedProcess::test()
{
	//TODO [shatzi]: add some unit testing!
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus WatershedProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Watershed-" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	
	auto coverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
	
	if (!coverage)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage not set");
		return knFailedToInit;
	}

	auto feature = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IFeature>();
	
	if (!feature)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input geometry not set");
		return knFailedToInit;
	}

	auto geometry = boost::dynamic_pointer_cast<PYXCell>(feature->getGeometry());

	if (!geometry) 
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input geometry must be a cell");
		return knFailedToInit;
	}

	m_spGeom = calculateWatershed(getParameter(0)->getValue(0),geometry->getIndex());

	return knInitialized;
}

//! Get the attributes in this process.
std::map<std::string, std::string> STDMETHODCALLTYPE WatershedProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;	

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE WatershedProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
}

std::string STDMETHODCALLTYPE WatershedProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"WatershedProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}


////////////////////////////////////////////////////////////////////////////////
// WatershedFlowProcess
////////////////////////////////////////////////////////////////////////////////

// {4BC01EC7-191B-4983-BB5C-3709B4F7AFC9}
PYXCOM_DEFINE_CLSID(WatershedFlowProcess, 
0x4bc01ec7, 0x191b, 0x4983, 0xbb, 0x5c, 0x37, 0x9, 0xb4, 0xf7, 0xaf, 0xc9);

PYXCOM_CLASS_INTERFACES(WatershedFlowProcess, IProcess::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(WatershedFlowProcess, "Watershed flow calculator", "Calculate a watershed flow to a given geometry based on elevation coverage.", "Analysis/Elevations",
					IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Elevation Coverage", "The surface elevation input coverage.")
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Destination Geometry", "The destination of the watershed geometry.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<WatershedFlowProcess> gFlowTester;

}

WatershedFlowProcess::WatershedFlowProcess()
{
}

WatershedFlowProcess::~WatershedFlowProcess()
{
}

void WatershedFlowProcess::test()
{
	//TODO [shatzi]: add some unit testing!
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus WatershedFlowProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Watershed-" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	
	auto coverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
	
	if (!coverage)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage not set");
		return knFailedToInit;
	}

	auto feature = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IFeature>();
	
	if (!feature)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input geometry not set");
		return knFailedToInit;
	}

	auto geometry = boost::dynamic_pointer_cast<PYXCell>(feature->getGeometry());

	if (!geometry) 
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input geometry must be a cell");
		return knFailedToInit;
	}

	m_spGeom = calculateWatershedFlow(getParameter(0)->getValue(0),geometry->getIndex());

	return knInitialized;
}

//! Get the attributes in this process.
std::map<std::string, std::string> STDMETHODCALLTYPE WatershedFlowProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;	

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE WatershedFlowProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
}

std::string STDMETHODCALLTYPE WatershedFlowProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"WatershedFlowProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}