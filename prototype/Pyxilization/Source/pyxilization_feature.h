#ifndef PYXILIZATION_FEATURE_H
#define PYXILIZATION_FEATURE_H

/******************************************************************************
pyxilization_feature.h

begin		: 2007-02-11
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/


// local includes
#include "pyx_geometry.h"
#include "pyx_feature.h"
#include "string_utils.h"
#include "pyx_cell.h"

// boost includes
#include <boost/shared_ptr.hpp>

// standard includes
#include <cassert>
#include <map>
#include <vector>

class PyxilizationFeature: public PYXFeature
{
public:
	//!this is what type of object the feature is
	enum Types
	{
		eTypeUnit = 0,
		eTypePlane,
		eTypeFactory,
		eTypeAirport,
		eTypeTank
	};
	enum FieldTypes
	{
		eFieldType = 0, //what type of unit it is
		eFieldHealth,   //how much health the unit has (number of units, really)
		eFieldMovesLeft,//how many moves the unit has left
		eFieldPlayerID, //id of the owning player
		eFieldLoaded,	//number of units loaded on a plane
		eFieldFuel,		//fuel left in plane
		eFieldBuildType	//what a factory is building
	};

	//!create a PyxilizationFeature - should be a PYXCell given
	PyxilizationFeature(boost::shared_ptr<PYXGeometry> spGeometry);

	//!set a particular field's value in the feature
	void setFieldValue(PYXValue value, int nFieldIndex);

	std::string getName() const {
		return "PyxilizationFeature";
	}

	virtual const std::string& getID() const {
		return m_strID;
	}

	//! since all IDs are integers, return this feature's id as an int
	int getIDInt(){
		return m_nID;
	}

	PYXValue getFieldValue(int nFieldIndex) const;

	boost::shared_ptr<const PYXGeometry> getGeometry() const{
		return m_spGeometry;
	}

	//! return whether the feature is able to be moved by the player
	bool isMovable(void)
	{
		if ((m_nType == eTypeUnit) || (m_nType == eTypePlane) || (m_nType == eTypeTank) )
		{
			return true;
		}
		return false;
	}

	//!set the resolution that the feature's cell is at - needed since the TVT will only render features of the exactly correct resolution
	void setResolution(int nRes)
	{
		m_spGeometry->setCellResolution(nRes);
	}

	void static test();

private:
	std::string m_strID;
	boost::shared_ptr<PYXGeometry> m_spGeometry;

	int m_nID;
	int m_nPlayerID;
	int m_nType;
	int m_nHealth;
	int m_nMovesLeft;
	int m_nLoaded;
	int m_nFuel;
	int m_nBuildType;
};

#endif