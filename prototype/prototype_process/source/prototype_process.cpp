/******************************************************************************
prototype_process.cpp

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PROTOTYPE_PROCESS_SOURCE
#include "prototype_process.h"

#include "pyxis/data/value_tile.h"

// {49172563-CD7C-4086-BF2A-1B74DC917CD3}
PYXCOM_DEFINE_CLSID(ProcA, 
0x49172563, 0xcd7c, 0x4086, 0xbf, 0x2a, 0x1b, 0x74, 0xdc, 0x91, 0x7c, 0xd3);
PYXCOM_CLASS_INTERFACES(ProcA, IProcess::iid, IUnknown::iid);

// {3C99E13B-04B6-4e4d-B6C1-7DEDA98522F2}
PYXCOM_DEFINE_CLSID(ProcB, 
0x3c99e13b, 0x4b6, 0x4e4d, 0xb6, 0xc1, 0x7d, 0xed, 0xa9, 0x85, 0x22, 0xf2);
PYXCOM_CLASS_INTERFACES(ProcB, IProcess::iid, IUnknown::iid);

// {80A2A331-E9AF-49f4-81C3-A831F1065674}
PYXCOM_DEFINE_CLSID(ProcC, 
0x80a2a331, 0xe9af, 0x49f4, 0x81, 0xc3, 0xa8, 0x31, 0xf1, 0x6, 0x56, 0x74);
PYXCOM_CLASS_INTERFACES(ProcC, IProcess::iid, IUnknown::iid);

#if 0
// {984CE653-C206-4fda-A3A4-D0B1C6C9BE32}
PYXCOM_DEFINE_CLSID(SoccerBallProc, 
0x984ce653, 0xc206, 0x4fda, 0xa3, 0xa4, 0xd0, 0xb1, 0xc6, 0xc9, 0xbe, 0x32);
PYXCOM_CLASS_INTERFACES(SoccerBallProc, IProcess::iid, IUnknown::iid);

// {81A67477-BAEB-4a93-B438-26F4C600C7F9}
PYXCOM_DEFINE_CLSID(SoccerBallCoverage, 
0x81a67477, 0xbaeb, 0x4a93, 0xb4, 0x38, 0x26, 0xf4, 0xc6, 0x0, 0xc7, 0xf9);
PYXCOM_CLASS_INTERFACES(SoccerBallCoverage, ICoverage::iid, IFeatureCollection::iid, IFeature::iid, IUnknown::iid);

// {3E896D4D-8C2D-4bdd-B009-4E3E7DB26A98}
PYXCOM_DEFINE_CLSID(SoccerBall, 
0x3e896d4d, 0x8c2d, 0x4bdd, 0xb0, 0x9, 0x4e, 0x3e, 0x7d, 0xb2, 0x6a, 0x98);
PYXCOM_CLASS_INTERFACES(SoccerBall, IProcess::iid, ICoverage::iid, IFeatureCollection::iid, IFeature::iid, IUnknown::iid);
#endif

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ProcA),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ProcB),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ProcC),
	//PYXCOM_CLASS_OBJECT_TABLE_ENTRY(SoccerBallProc),
	//PYXCOM_CLASS_OBJECT_TABLE_ENTRY(SoccerBallCoverage),
	//PYXCOM_CLASS_OBJECT_TABLE_ENTRY(SoccerBall),
PYXCOM_END_CLASS_OBJECT_TABLE

#if 0
IPROCESS_SPEC_BEGIN(ProcA, IClassFactory::iid, "procspec A name", "procspec A desc")
	IPROCESS_SPEC_PARAMETER(IProcess::iid, 1, 1, "required", "r desc")
	IPROCESS_SPEC_PARAMETER(IProcess::iid, 0, -1, "multivalued", "mv desc")
IPROCESS_SPEC_END

IPROCESS_SPEC_BEGIN(ProcB, IClassFactory::iid, "procspec B name", "procspec B desc")
IPROCESS_SPEC_END

IPROCESS_SPEC_BEGIN(ProcC, IClassFactory::iid, "procspec C name", "procspec C desc")
	IPROCESS_SPEC_PARAMETER(IClassInfo::iid, 1, 1, "required", "r desc")
	IPROCESS_SPEC_PARAMETER(IClassInfo::iid, 0, -1, "multivalued", "mv desc")
IPROCESS_SPEC_END

IPROCESS_SPEC_BEGIN(SoccerBallProc, ICoverage::iid, "SoccerBallProc", "SoccerBallProc")
IPROCESS_SPEC_END

PYXPointer<FeatureIterator> SoccerBallCoverage::getIterator() const
{
	return PYXPointer<FeatureIterator>();
}

PYXValue SoccerBallCoverage::getCoverageValue(const PYXIcosIndex& index) const
{
	return PYXValue(index.isFace() ? 1 : 0);
}

PYXPointer<PYXValueTile> SoccerBallCoverage::getCoverageTile(const PYXTile& tile) const
{
	return PYXPointer<PYXValueTile>();
}

#if 1
PYXPointer<FeatureIterator> SoccerBall::getIterator() const
{
	return PYXPointer<FeatureIterator>();
}

PYXValue SoccerBall::getCoverageValue(const PYXIcosIndex& index) const
{
	return PYXValue(index.isFace() ? 1 : 0);
}

PYXPointer<PYXValueTile> SoccerBall::getCoverageTile(const PYXTile& tile) const
{
	return PYXPointer<PYXValueTile>();
}
#endif
#endif
