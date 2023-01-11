/******************************************************************************
pyxilization_style_mapper.cpp

begin		: 2007-02-11
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxilization_style_mapper.h"
#include "pyxilization_feature.h"
#include "pyx_rgb.h"

//this gets changed as need be to reflect the resolution currently
//being displayed - we need to change a feature's resolution
//to match this so it displays at all
int PyxilizationStyleMapper::m_nDisplayResolution = 6;

FeatureStyle::FeatureStyleConstPtr
PyxilizationStyleMapper::mapFeature(boost::shared_ptr<const PYXFeature> spFeature) const
{
	boost::shared_ptr<FeatureStyle> sp(new FeatureStyle);
	sp->shouldRenderGeometry(true);
	
	PYXValue nUnits;

	switch(spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8())
	{
		//factory is always the same
		case PyxilizationFeature::eTypeFactory:
			sp->setValue(FeatureStyle::knSymbol, "factory");
		break;

		//units are shown differently based on health
		case PyxilizationFeature::eTypeUnit:
			nUnits = spFeature->getFieldValue(PyxilizationFeature::eFieldHealth);
			switch(nUnits.getUInt8())
			{
				case 1:sp->setValue(FeatureStyle::knSymbol, "soldier1"); break;
				case 2:sp->setValue(FeatureStyle::knSymbol, "soldier2"); break;
				case 3:sp->setValue(FeatureStyle::knSymbol, "soldier3"); break;
				case 4:sp->setValue(FeatureStyle::knSymbol, "soldier4"); break;
				case 5:sp->setValue(FeatureStyle::knSymbol, "soldier5"); break;
				case 6:sp->setValue(FeatureStyle::knSymbol, "soldier6"); break;
				case 7:sp->setValue(FeatureStyle::knSymbol, "soldier7"); break;
				case 8:sp->setValue(FeatureStyle::knSymbol, "soldier8"); break;
				case 9:sp->setValue(FeatureStyle::knSymbol, "soldier9"); break;
				case 10:sp->setValue(FeatureStyle::knSymbol, "soldier10"); break;
			}
		break;

		//airpost is always the same
		case PyxilizationFeature::eTypeAirport:
			sp->setValue(FeatureStyle::knSymbol, "airport");
		break;

		//plane is always the same
		case PyxilizationFeature::eTypePlane:
			sp->setValue(FeatureStyle::knSymbol, "plane");
		break;

		//tanks are like units
		case PyxilizationFeature::eTypeTank:
			nUnits = spFeature->getFieldValue(PyxilizationFeature::eFieldHealth);
			switch(nUnits.getUInt8())
			{
				case 1:sp->setValue(FeatureStyle::knSymbol, "tank1"); break;
				case 2:sp->setValue(FeatureStyle::knSymbol, "tank2"); break;
				case 3:sp->setValue(FeatureStyle::knSymbol, "tank3"); break;
				case 4:sp->setValue(FeatureStyle::knSymbol, "tank4"); break;
				case 5:sp->setValue(FeatureStyle::knSymbol, "tank5"); break;
				case 6:sp->setValue(FeatureStyle::knSymbol, "tank6"); break;
				case 7:sp->setValue(FeatureStyle::knSymbol, "tank7"); break;
				case 8:sp->setValue(FeatureStyle::knSymbol, "tank8"); break;
				case 9:sp->setValue(FeatureStyle::knSymbol, "tank9"); break;
				case 10:sp->setValue(FeatureStyle::knSymbol, "tank10"); break;
			}
			break;
/*		case 117:
				PYXRGB selectedColorRGB(255, 0, 255, 0);
				std::ostringstream outSelected;
				outSelected << selectedColorRGB;
				sp->setValue(FeatureStyle::knVectorColour, outSelected.str());
			break;*/
	}

	// determine the location for the symbol
	boost::shared_ptr<const PYXCell> spCell =
		boost::dynamic_pointer_cast<const PYXCell>(spFeature->getGeometry());
	if (spCell)
	{
		//adjust the resolution to the currently displayed resolution
		//otherwise it won't show up
		PYXIcosIndex indexAdjusted(spCell->getIndex());
		indexAdjusted.setResolution(getDisplayResolution());
		// store the index location for the symbol
		sp->setValue(FeatureStyle::knSymbolCell, indexAdjusted.toString());
	}

	return sp;
}