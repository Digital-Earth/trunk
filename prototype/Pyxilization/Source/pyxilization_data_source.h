#ifndef PYXILIZATION_DATA_SOURCE_H
#define PYXILIZATION_DATA_SOURCE_H
/******************************************************************************
pyxilization_data_source.h

begin		: 2007-02-02
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/
 
// local includes
#include "pyx_feature_data_source.h"
#include "pyx_feature_iterator.h"
#include "pyx_geometry.h"
#include "string_utils.h"
#include "style_mapper.h"
#include "pyx_cell.h"
#include "pyxilization_data_source_iterator.h"
#include "pyxilization_feature.h"
#include "pyxilization_style_mapper.h"
#include "pyx_windows.h"
#include "windows.h"
#include "plugin_dll.h"

// driver includes
#include "pyx_driver.h"


// boost includes
#include <boost/shared_ptr.hpp>

// standard includes
#include <cassert>
#include <map>
#include <vector>

/*!

This class it the main class in the Pyxilization game.  It controls
all the game logic and actions.  It can be communicated with
through its 'command' function which takes text commands and plays
them out on the earth.

*/
class PyxilizationDataSource : public PYXFeatureDataSource
{
public:

	//! receive text command from the command interpreter
	void command(std::stringstream &commandLine, PlugInDLL *pPlugin);

	//! Destructor
	virtual ~PyxilizationDataSource(){}

	//! Sets the resolution
	void setResolution(int nRes);

	//! Gets the name...
	virtual std::string getName(void) const
	{
		return "pyxilization";
	}

	//! Return the class name of this observer class.
	virtual std::string getObserverDescription() const 
	{
		return "PyxilizationDataSource " + getName();
	}

	//! Return the name of the notification class.
	virtual std::string getNotifierDescription() const
	{
		return getObserverDescription();
	}

	//! Get the geometry of the data source.  Note: must be non-null.
	virtual boost::shared_ptr<const PYXGeometry> getGeometry() const
	{
		return m_spGeomDS;
	}

	//! Get an iterator to the features.
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator() const;

	//! Get an iterator to the features.
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator(const PYXGeometry& geometry	) const;

	//! Get an iterator to the features. 
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator(
		const std::string& strWhere	) const {return getFeatureIterator();}

	//! Get an iterator to the features.
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator(
		const PYXGeometry& geometry,
		const std::string& strWhere	) const {return getFeatureIterator();}

	//!	Get a specific feature based on its ID.
	boost::shared_ptr<const PYXFeature> getFeature(const std::string& strFeatureID) const;

	//! Get the style mapper for the data source
	virtual boost::shared_ptr<StyleMapper> getStyleMapper()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (!m_spMapper)
		{
			m_spMapper.reset(new PyxilizationStyleMapper());
		}
		return m_spMapper;
	}

	//! Open the data source.
	bool open(const std::string& strFileName);

	//! Constructor
	PyxilizationDataSource();

	//! Let's just make sure that we don't go below our lowest resolution
	int getMinimumAvailableResolution() { return 5; };

	//! Wouldn't work without this - not entirely sure why
	int getMaximumAvailableResolution() { return 30; };

	//!check two cells to see if they are beside each other
	bool static areNeighbours(boost::shared_ptr<PYXCell> spCell1, boost::shared_ptr<PYXCell> spCell2);

	//!get the game's resolution
	int static getGameResolution(void)
	{
		return m_nGameResolution;
	}

	//!set the game resolution
	void static setGameResolution(int nRes)
	{
		m_nGameResolution = nRes;
	}

	//! Unit test function
	void static test();

	//!get a pointer to all the features - done this way to avoid consty-ness problems (guh)
	const std::vector<boost::shared_ptr<PyxilizationFeature> > *getUnits() const
	{
		return &m_vecUnits;
	}

	//!set the number of points a player has
	void setPoints(int iPlayer, int nPoints)
	{
		m_rnPoints[iPlayer] = nPoints;
	}

private:
	//! find a feature at a particular geometry with a particular type
	boost::shared_ptr<PyxilizationFeature> findFeature(PYXGeometry &geo, int nType);

	//! find all features at a particular geometry
	std::vector<boost::shared_ptr<PyxilizationFeature> > findFeatures(PYXGeometry &geo);

	//! find a feature based on its ID
	boost::shared_ptr<PyxilizationFeature> findFeatureByID(int nID);

	//!remove a feature from the datasource with id nID
	bool removeFeature(int nID);

	//! move a feature command
	void moveCommand(std::stringstream &commandLine);

	//! interpret a 'load' command (load units onto a plane)
	void loadCommand(std::stringstream &commandLine);

	//! interpret an 'unload' command (unload units from a plane)
	void unloadCommand(std::stringstream &commandLine);

	//! interpret a 'settype' command (change factory production)
	void setTypeCommand(std::stringstream &commandLine);

	//! interpret at 'buy' command (buy a new unit at a factory)
	void buyCommand(std::stringstream &commandLine);

	//! resolve a battle
	int battle(boost::shared_ptr<PyxilizationFeature> spAttacker, boost::shared_ptr<PyxilizationFeature> spDefender, int nAttackers);

	//! call to end turn
	void endTurn(PlugInDLL *pPlugin);

	//! call to end round of turns - creates new units, etc.
	void endRound(PlugInDLL *pPlugin);

	//! empties all the structures in the game and sets it up for a new game
	void startGame(int nPlayers);

	//! sets which player owns a cell
	void setCellOwner(PYXGeometry &geo, int iPlayerID);

	//! get the strongest feature from a vector of them
	int getStrongest(std::vector<boost::shared_ptr<PyxilizationFeature> > vecFeatures);

	//! creates a new soldier
	boost::shared_ptr<PyxilizationFeature> newUnit(boost::shared_ptr<PYXGeometry> spGeo, PyxilizationFeature::Types field, int iPlayerID, int nHealth, int nMovesLeft);
	//! creates a new factory
	boost::shared_ptr<PyxilizationFeature> newBuilding(std::string strLocation, PyxilizationFeature::Types field);

	//! Disable copy constructor
	PyxilizationDataSource(const PyxilizationDataSource&);

	//! Disable copy assignment
	void operator=(const PyxilizationDataSource&);

	//! The style mapper for the features.
	boost::shared_ptr<PyxilizationStyleMapper> m_spMapper;

	//! The geometry of the data source.
	mutable boost::shared_ptr<PYXGeometry> m_spGeomDS;

	//!vector of the features - each unit/factory/plane/etc. stores as an individual feature
	std::vector<boost::shared_ptr<PyxilizationFeature> > m_vecUnits;

	//!check a cell to see if it is water or not (requires a raster DS loaded or will always return false)
	bool isWater(const PYXIcosIndex &index);

	//game stuff

	//!number of players
	int m_nPlayers;

	//!what resolution game is being played at
	static int m_nGameResolution;

	//!whose turn it is right now
	int m_iCurPlayer;

	//!points for each player
	int m_rnPoints[4];

	//DEFINES
	//!max number of players
	static const int m_nMaxPlayers;
	//!max number of units on a cell
	static const int m_nMaxSoldiersPerCell;
	//!max number of moves for a unit
	static const int m_nSoldierMovesPerTurn;
	//!the owning player id given to features not owned
	static const int m_nInvalidPlayerID;
	//!the range of values the battle dice can be rolled to
	static const int m_nDiceRange;
	//!max number of soldiers on a plane
	static const int m_nMaxSoldierPlane;
	//!maximum fuel a plane can have
	static const int m_nMaxPlaneFuel;
	//!how many turns it takes a factory to build a soldier
	static const int m_nSoldierBuildTime;
	//!how many turns it takes a factory to build a tank
	static const int m_nTankBuildTime;
	//!how many points it costs to make a soldier
	static const int m_nUnitCost;
	//!how many points it costs to make a tank
	static const int m_nTankCost;
	//!how far a plane can fly
	static const double m_nFarthestFlight;
	//!what colour each player is
	static const int PyxilizationDataSource::m_rnPlayerColours[4][3];
	//!the URI of the pyxilization player colour coverage
	static const std::string m_strAlphaURI;

	//features used to draw border around icos faces
	//mutable std::vector<boost::shared_ptr<PyxilizationFeature>> m_vecEdges;
};




#endif