/******************************************************************************
pyxilization_data_source.cpp

begin		: 2007-02-02
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxilization_data_source.h"
#include "pyx_windows.h"
#include "windows.h"
#include "plugin_dll.h"
#include "pyx_multi_geometry.h"
#include "pyx_icos_index.h"
#include "data_source_event.h"
#include "pyx_icos_math.h"
#include "pyx_data_source_manager.h"
#include "pyx_dynamic_global_grid_data_source.h"
#include "data_source_toggle_visibility_cmd.h"
#include "pyx_global.h"
#include "command.h"
#include "wgs84.h"
#include "snyder_projection.h"
#include <windows.h>
#include "ArmyColouringDS.h"
#include "tester.h"
#include "pyx_edge_iterator.h"
#include "pyx_dir_edge_iterator.h"
#include "pyx_icos_iterator.h"

//#include "pyxilization_event.h"
/* Scovil's Notes

	No notes at the moment

*/

int PyxilizationDataSource::m_nGameResolution = 5;
const int PyxilizationDataSource::m_nMaxPlayers = 4;
const int PyxilizationDataSource::m_nMaxSoldiersPerCell = 10;
const int PyxilizationDataSource::m_nSoldierMovesPerTurn = 3;
const int PyxilizationDataSource::m_nInvalidPlayerID = 255;
const int PyxilizationDataSource::m_nDiceRange = 6;
const int PyxilizationDataSource::m_nMaxSoldierPlane = 5;
const int PyxilizationDataSource::m_nMaxPlaneFuel = 4;
const int PyxilizationDataSource::m_nSoldierBuildTime = 1;
const int PyxilizationDataSource::m_nTankBuildTime = 2;

const int PyxilizationDataSource::m_nUnitCost = 10;
const int PyxilizationDataSource::m_nTankCost = 20;

const double PyxilizationDataSource::m_nFarthestFlight = 4000000;

const int PyxilizationDataSource::m_rnPlayerColours[m_nMaxPlayers][3] = {{255,0,0},{0,255,0},{0,0,255},{80,80,80}};

const std::string PyxilizationDataSource::m_strAlphaURI = "c:\\pyxis_data\\pyxilizationalpha.tr2";

TesterUnit<PyxilizationDataSource> pyxilizationTester;

/*!
This function tests everything.
*/
void PyxilizationDataSource::test()
{
	PyxilizationDataSource pDS;
	std::stringstream strmCom;

	//start game
	strmCom.str("startgame 2\n");
	pDS.command(strmCom, 0);

	//test making new units
	boost::shared_ptr<PYXCell> spNewUnitCell(new PYXCell(PYXIcosIndex("A-010101")));
	pDS.newUnit(spNewUnitCell, PyxilizationFeature::eTypeUnit, 1, 5, 3);

	boost::shared_ptr<PyxilizationFeature> spFeature = pDS.findFeature(*spNewUnitCell, PyxilizationFeature::eTypeUnit);
	TEST_ASSERT(spFeature);
	TEST_ASSERT(spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() == 5);

	//test building factory
	boost::shared_ptr<PyxilizationFeature> spBuilding = pDS.newBuilding("E-020304", PyxilizationFeature::eTypeFactory);
	TEST_ASSERT(spBuilding);
	TEST_ASSERT(spBuilding->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == PyxilizationFeature::eTypeFactory);

	//test setting factory build type
	PYXCell factoryCell(PYXIcosIndex("E-020304"));
	pDS.setCellOwner(factoryCell,0);
	strmCom.str("settype " + spBuilding->getID() + " 4\n");
	pDS.command(strmCom, 0);
	TEST_ASSERT(spBuilding->getFieldValue(PyxilizationFeature::eFieldBuildType).getUInt8() == PyxilizationFeature::eTypeTank);

	//make sure a tank gets built
	strmCom.str("endturn\n");
	pDS.command(strmCom, 0);
	strmCom.str("endturn\n");
	pDS.command(strmCom, 0);
	strmCom.str("endturn\n");
	pDS.command(strmCom, 0);
	strmCom.str("endturn\n");
	pDS.command(strmCom, 0);
	TEST_ASSERT(pDS.findFeature(factoryCell, PyxilizationFeature::eTypeTank) != 0);

	//attempt to buy a unit
	pDS.setPoints(0, m_nTankCost);
	strmCom.str("buy " + spBuilding->getID() + " 4\n");
	pDS.command(strmCom, 0);
	TEST_ASSERT(pDS.findFeature(factoryCell, PyxilizationFeature::eTypeTank));

	//test loading/unloading from planes
	PYXIcosIndex index("D-0102030");
	boost::shared_ptr<PYXCell> spCell(new PYXCell(index));
	boost::shared_ptr<PyxilizationFeature> spUnit = pDS.newUnit(spCell, PyxilizationFeature::eTypeUnit, 0, 5, 3);
	boost::shared_ptr<PyxilizationFeature> spPlane = pDS.newUnit(spCell, PyxilizationFeature::eTypePlane, 0, 1, 1);

	//load...
	strmCom.str("load D-0102030 5\n");
	pDS.command(strmCom, 0);
	TEST_ASSERT(spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8() == 5);

	//move...
	std::string strMoveCom;
	strMoveCom = "move 1 " + spPlane->getID() + " C-0102030\n";
	strmCom.str(strMoveCom);
	pDS.command(strmCom, 0);
	PYXCell cell(PYXIcosIndex("C-0102030"));
	boost::shared_ptr<PyxilizationFeature> spPlane2 = pDS.findFeature(cell, PyxilizationFeature::eTypePlane);

	TEST_ASSERT(spPlane2);

	//unload...
	strmCom.str("unload C-0102030 5\n");
	pDS.command(strmCom, 0);
	TEST_ASSERT(spPlane2->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8() == 0);
}

/*!
This finds all the features that reside in a particular geography and
returns them in a vector.

\param geo		The geography the features must be in.
\return a vector to the features
*/
std::vector<boost::shared_ptr<PyxilizationFeature> > PyxilizationDataSource::findFeatures(PYXGeometry &geo)
{
	boost::intrusive_ptr<PyxilizationDataSourceIterator> it = boost::dynamic_pointer_cast<PyxilizationDataSourceIterator, PYXFeatureIterator>(getFeatureIterator(geo));
	std::vector<boost::shared_ptr<PyxilizationFeature> > vecFeatures;

	while(!it->end())
	{
		vecFeatures.push_back(boost::dynamic_pointer_cast<PyxilizationFeature, PYXFeature>(it->getFeature()));
		it->next();
	}
	return vecFeatures;
}

/*!
This finds all the features that reside in a particular geography and
have the TYPE specified, and returns them in a vector.

\param geo		The geography the features must be in.
\param nType	The type the feature must have
\return a vector to the features
*/
boost::shared_ptr<PyxilizationFeature> PyxilizationDataSource::findFeature(PYXGeometry &geo, int nType)
{
	boost::intrusive_ptr<PyxilizationDataSourceIterator> it = boost::dynamic_pointer_cast<PyxilizationDataSourceIterator, PYXFeatureIterator>(getFeatureIterator(geo));
	boost::shared_ptr<PYXFeature> featTemp;

	while(!it->end())
	{
		featTemp = it->getFeature();

		if(featTemp->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == nType)
		{
			return boost::dynamic_pointer_cast<PyxilizationFeature, PYXFeature>(featTemp);
		}
		it->next();
	}
	return boost::shared_ptr<PyxilizationFeature>();
}

/*!
Find a feature with a particular ID (given as an integer)
\param nID	The ID of the feature we want to find.
\return A shared pointer to the feature
*/
boost::shared_ptr<PyxilizationFeature> PyxilizationDataSource::findFeatureByID(int nID)
{
	std::vector<boost::shared_ptr<PyxilizationFeature> >::iterator it;

	it = m_vecUnits.begin();
	for(; it!=m_vecUnits.end(); it++)
	{
		//correct id?
		if((*it)->getIDInt() == nID)
		{
			return *it;
		}
	}
	return boost::shared_ptr<PyxilizationFeature>();
}


/*!
Remove a feature from the Data source
\param nID	The ID of the feature to remove
\return Returns true if the feature was found and removed, false if not.
*/

bool PyxilizationDataSource::removeFeature(int nID)
{
	std::vector<boost::shared_ptr<PyxilizationFeature> >::iterator it;
	boost::shared_ptr<PYXFeature> featTemp;

	it = m_vecUnits.begin();
	for(; it!=m_vecUnits.end(); it++)
	{
		if((*it)->getIDInt() == nID)
		{
			m_vecUnits.erase(it);
			return true;
		}
	}
	return false;
}

/*! 
This is a static function to check if two PYXCells are next to
each other.  It uses the PYXIcosMath::getNeighbours function to compare
the first cell's neighbours to the other cell.
*/
bool PyxilizationDataSource::areNeighbours(boost::shared_ptr<PYXCell> spCell1, boost::shared_ptr<PYXCell> spCell2)
{
	std::vector<PYXIcosIndex> vecIndices;
	std::vector<PYXIcosIndex>::iterator itIndices;

	//get all the neighbours then check to see if equal to the other
	PYXIcosMath::getNeighbours(spCell1->getIndex(), &vecIndices);

	itIndices = vecIndices.begin();
	for(; itIndices != vecIndices.end(); itIndices++)
	{
		if((*itIndices) == spCell2->getIndex())
		{
			return true;
		}
	}
	return false;
}

/*!
isWater searches all the available raster data sources and the first
it finds that is not the Pyxilization player colour coverage, it will
test the given index's colour to see if it is water.  Water is defined
as a cell where the blue is more than 5 values above both green and 
red, but still below the value of 40.

\param index	the index to test
\return			is it water or not?
*/
bool PyxilizationDataSource::isWater(const PYXIcosIndex &index)
{
	std::vector<std::string> vecDSs;
	std::vector<std::string>::iterator it;

	//get a list of all the loaded data sources then iterate through them
	PYXDataSourceManager::getInstance()->getOpenDataSources(&vecDSs);

	it = vecDSs.begin();
	while(it != vecDSs.end())
	{
		PYXDataItem::SPtr spItem;
		//if it's not our alpha blending one we can use it
		if(*it != m_strAlphaURI)
		{
			if(PYXDataSourceManager::getInstance()->getDSType(*it) == PYXDataItem::knRaster)
			{
				if(PYXDataSourceManager::getInstance()->getDataItem(*it, &spItem))
				{
					boost::shared_ptr<PYXCoverage> spCoverage = spItem->getDSAsCoverage();
					PYXValue valColour = spCoverage->getCoverageValue(index);
					PYXValue::eType type = valColour.getArrayType();
					std::string str = valColour.getTypeAsString(type);
					PYXValue valRed = valColour.getValue(0);
					PYXValue valGreen = valColour.getValue(1);
					PYXValue valBlue = valColour.getValue(2);

					uint8_t rgb[3];
					rgb[0] = valRed.getUInt8();
					rgb[1] = valGreen.getUInt8();
					rgb[2] = valBlue.getUInt8();

					if((rgb[2] > rgb[0] + 5) && (rgb[2] > rgb[1] + 5))
					{
						//blue is the most dominant
						if(rgb[0] < 40)
						{
							//it's still pretty dark -- good chance it's water
							return true;
						}
					}
				}
			}
		}
		it++;
	}

	return false;
}

/*!
newBuilding creates a new building feature at a given cell, adds it to
the PyxilizatoinDataSource's internal feature vector, and then returns
it.

\param strLocation	the index where to building should be, in string form
\param type			what type of building the building is
\return				returns a shared ptr to the new feature
*/
boost::shared_ptr<PyxilizationFeature> PyxilizationDataSource::newBuilding(std::string strLocation, PyxilizationFeature::Types type)
{
	PYXIcosIndex factoryIndex(strLocation);
	factoryIndex.setResolution(getGameResolution());
	boost::shared_ptr<PyxilizationFeature> spFeature(new PyxilizationFeature(boost::shared_ptr<PYXGeometry>(new PYXCell(factoryIndex))));

	spFeature->setFieldValue(PYXValue((uint8_t)type), PyxilizationFeature::eFieldType);
	//owned by nobody - we only create buildings at start of game
	spFeature->setFieldValue(PYXValue((uint8_t)m_nInvalidPlayerID), PyxilizationFeature::eFieldPlayerID);
	//start the countdown at the soldier build time
	spFeature->setFieldValue(PYXValue((uint8_t)m_nSoldierBuildTime), PyxilizationFeature::eFieldMovesLeft);
	//default build units
	spFeature->setFieldValue(PYXValue((uint8_t)PyxilizationFeature::eTypeUnit), PyxilizationFeature::eFieldBuildType);
	m_vecUnits.push_back(spFeature);
	return spFeature;
}

/*!
newUnit creates a new unit feature at a given cell, adds it to
the PyxilizatoinDataSource's internal feature vector, and then returns
it.

\param spGeo		A PYXCell where the new feature will be
\param type			what type of feature this will be
\param iPlayerID	Player id of who owns this
\param nHealth		What health should they start with?
\param nMovesLeft	How many moves left should this start with?
\return				returns a shared ptr to the new feature
*/
boost::shared_ptr<PyxilizationFeature> PyxilizationDataSource::newUnit(boost::shared_ptr<PYXGeometry> spGeo, PyxilizationFeature::Types type,  int iPlayerID, int nHealth, int nMovesLeft)
{
	spGeo->setCellResolution(getGameResolution());
	boost::shared_ptr<PyxilizationFeature> spNewUnit(new PyxilizationFeature(spGeo));
	spNewUnit->setFieldValue(PYXValue((uint8_t)iPlayerID), PyxilizationFeature::eFieldPlayerID);
	spNewUnit->setFieldValue(PYXValue((uint8_t)type), PyxilizationFeature::eFieldType);
	spNewUnit->setFieldValue(PYXValue((uint8_t)nHealth), PyxilizationFeature::eFieldHealth);
	spNewUnit->setFieldValue(PYXValue((uint8_t)nMovesLeft), PyxilizationFeature::eFieldMovesLeft);
	//make sure a plane doesn't start with units in it
	spNewUnit->setFieldValue(PYXValue((uint8_t)0), PyxilizationFeature::eFieldLoaded);
	
	//if it's a plane, give it some fuel (+1 because at end of turn 1 is subtracted from fuel of all planes
	if(type == PyxilizationFeature::eTypePlane)
	{
		spNewUnit->setFieldValue(PYXValue((uint8_t)m_nMaxPlaneFuel + 1), PyxilizationFeature::eFieldFuel);
	}
	m_vecUnits.push_back(spNewUnit);
	return spNewUnit;
}

/*!
endRound() actually manages the transfer of gameplay from one round to
the next.  It iterates through the buildings, creates or refreshes units,
then iterates through units and destroys or refreshes them, as the rules
define.  It then calculates the owner of each resolution 1 face and gives
points to each player accordingly.  During this time it counts up all the
units each player has, and returns the statistics to the plug-in provided
through its addCmdResponse function.

\param pPlugin		the plugin that gave us this command
*/
void PyxilizationDataSource::endRound(PlugInDLL *pPlugin)
{
	//number of players BY number of types - we count all the units for every
	//player since we're already iterating through everything anyway
	int nCounts[m_nMaxPlayers][5] = {0};

	//first go through buildings and do updates
	//we split up doing updates like this because we want to make sure
	//the refueling and new unit creation is resolved first before
	//plane crashing and other logic is checked

	PyxilizationDataSourceIterator it(this);
	while(!it.end())
	{
		boost::shared_ptr<PyxilizationFeature> spFeature = boost::dynamic_pointer_cast<PyxilizationFeature, PYXFeature>(it.getFeature());
		if(spFeature!=0)
		{
			PYXValue val = spFeature->getFieldValue(PyxilizationFeature::eFieldType);
			switch(val.getUInt8())
			{
				//do factory stuff
				case PyxilizationFeature::eTypeFactory:
					if(spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() != m_nInvalidPlayerID)
					{
						boost::shared_ptr<PYXCell> cell(new PYXCell());
						*cell = *(boost::dynamic_pointer_cast<const PYXCell, const PYXGeometry>(spFeature->getGeometry()));

						int nTurnsLeft = spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8();
						if((nTurnsLeft-1) == 0)
						{
							PyxilizationFeature::Types type = (PyxilizationFeature::Types)spFeature->getFieldValue(PyxilizationFeature::eFieldBuildType).getUInt8();
							//set the correct number of turns until next is created
							switch(type)
							{
								case PyxilizationFeature::eTypeUnit:
									spFeature->setFieldValue(PYXValue((uint8_t)m_nSoldierBuildTime), PyxilizationFeature::eFieldMovesLeft);
								break;
								case PyxilizationFeature::eTypeTank:
									spFeature->setFieldValue(PYXValue((uint8_t)m_nTankBuildTime), PyxilizationFeature::eFieldMovesLeft);
								break;
							}

							//look to see if this type of feature already exists in the cell
							boost::shared_ptr<PyxilizationFeature> spUnitFeature = findFeature(*cell, type);
							if(spUnitFeature!=0) //feature already exists here, add +1 to health
							{
								val = spUnitFeature->getFieldValue(PyxilizationFeature::eFieldHealth);
								if(val.getUInt8() < m_nMaxSoldiersPerCell)
								{
									spUnitFeature->setFieldValue(PYXValue((uint8_t) val.getUInt8() + 1), PyxilizationFeature::eFieldHealth);
								}
							}
							else	//there's no feature - create one
							{
								newUnit(cell, type ,spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8(), 1, m_nSoldierMovesPerTurn);
							}
						}
						else
						{
							spFeature->setFieldValue(PYXValue((uint8_t)nTurnsLeft-1), PyxilizationFeature::eFieldMovesLeft);
						}
					}
				break;
				//airport stuff
				case PyxilizationFeature::eTypeAirport:
					if(spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() != m_nInvalidPlayerID)
					{
						boost::shared_ptr<PYXCell> cell(new PYXCell());
						*cell = *(boost::dynamic_pointer_cast<const PYXCell, const PYXGeometry>(spFeature->getGeometry()));
						boost::shared_ptr<PyxilizationFeature> spUnitFeature = findFeature(*cell, PyxilizationFeature::eTypePlane);

						if(spUnitFeature!=0) //plane already exists here, add to health for fuel (+1 because it will be decremented in the unit updates)
						{
							spUnitFeature->setFieldValue(PYXValue((uint8_t)m_nMaxPlaneFuel + 1), PyxilizationFeature::eFieldFuel);
						}
						else	//there's no plane - create one
						{
							newUnit(cell, PyxilizationFeature::eTypePlane, spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8(), 1, 1);
						}
					}
				break;
			}
		}
		it.next();
	}

	//update units
	PyxilizationDataSourceIterator it2(this);
	while(!it2.end())
	{
		boost::shared_ptr<PyxilizationFeature> spFeature = boost::dynamic_pointer_cast<PyxilizationFeature, PYXFeature>(it2.getFeature());
		if(spFeature!=0)
		{
			//add the number of that unit to their stats (nCounts)
			PYXValue val = spFeature->getFieldValue(PyxilizationFeature::eFieldType);
			if((val.getUInt8() == PyxilizationFeature::eTypeUnit) || (val.getUInt8() == PyxilizationFeature::eTypeTank))
			{
				nCounts[ spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() ][ spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() ] += spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8();
			}
			else
			{
				nCounts[ spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() ][ spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() ] ++;
				if(val.getUInt8() == PyxilizationFeature::eTypePlane)
				{
					nCounts[ spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() ][ PyxilizationFeature::eTypeUnit ] += spFeature->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8();
				}
			}

			switch(val.getUInt8())
			{
				case PyxilizationFeature::eTypeTank:
				case PyxilizationFeature::eTypeUnit:
					spFeature->setFieldValue(PYXValue((uint8_t) m_nSoldierMovesPerTurn), PyxilizationFeature::eFieldMovesLeft);
				break;
				case PyxilizationFeature::eTypePlane:
					spFeature->setFieldValue(PYXValue((uint8_t) 1), PyxilizationFeature::eFieldMovesLeft);
					spFeature->setFieldValue(PYXValue((uint8_t) spFeature->getFieldValue(PyxilizationFeature::eFieldFuel).getUInt8() - 1) , PyxilizationFeature::eFieldFuel);

					//if they've run out of fuel
					if(spFeature->getFieldValue(PyxilizationFeature::eFieldFuel).getUInt8() == 0)
					{
						removeFeature(spFeature->getIDInt());
					}
				break;
			}
		}
		it2.next();
	}

	//who owns which tile?
	//give points to whoever owns the most cells (or nobody if tied)
	bool bResult;
 	PYXDataItem::SPtr spItem;
	if(PYXDataSourceManager::getInstance()->isInitialized() == true)
	{
		//we use the alpha data source to find who owns a cell
		PYXDataSourceManager::getInstance()->isOpen(m_strAlphaURI, &bResult);
		if(bResult==true)
		{
			if(PYXDataSourceManager::getInstance()->getDataItem(m_strAlphaURI, &spItem))
			{
				boost::shared_ptr<PYXCoverage> spCoverage = spItem->getDSAsCoverage();
				if(spCoverage!=0)
				{
					PYXIcosIterator faceIt(1);	//best iterator name ever
					while(!faceIt.end())
					{
						int scores[m_nMaxPlayers] = {0};
						int highest=0;

						//second best iterator name ever
						PYXExhaustiveIterator exIt(faceIt.getIndex(), m_nGameResolution);
						while(!exIt.end())
						{
							PYXValue val = spCoverage->getCoverageValue(exIt.getIndex());
							//check which player owns the cell
							//this has to use hard coded values because of the way the alpha one is set up
							for(int n=0;n<m_nMaxPlayers;n++)
							{
								if((val.getUInt8(0) == m_rnPlayerColours[n][0]) && (val.getUInt8(1) == m_rnPlayerColours[n][1]) && (val.getUInt8(2) == m_rnPlayerColours[n][2]))
								{
									scores[n]++;
									if(scores[n] > scores[highest])
									{
										highest = n;
									}
								}
							}
							exIt.next();
						}
						//check for a tie
						int winner = highest;
						for(int n=0;n<m_nMaxPlayers;n++)
						{
							if((scores[n] == scores[highest]) && (n != highest))
							{
								winner=-1;
								break;
							}
						}
						//do we have a winner?
						if(winner!=-1)
						{
							//we have a winner!
							m_rnPoints[winner] += 1;
						}
						faceIt.next();
					}
				}
			}
		}
	}

	//we now give the stats to the plug-in
	//if while in unit test (no plugin given) we TRACE_INFO the data instead
	char buff[256];
	if(pPlugin != 0)
	{
		pPlugin->addCmdResponse("pyxilization");
	}
	for(int iPlayer=0; iPlayer < this->m_nPlayers; iPlayer++)
	{
		sprintf(buff, "pyxilization playerstats %d\n", iPlayer);
		if(pPlugin != 0)
		{
			pPlugin->addCmdResponse(buff);
		}
		else
		{
			TRACE_INFO(buff);
		}

		for(int iType=0; iType < 5; iType++)
		{
			sprintf(buff, "pyxilization %d %d\n", iType, nCounts[iPlayer][iType]);

			if(pPlugin!=0)
			{
				pPlugin->addCmdResponse(buff);
			}
			else
			{
				TRACE_INFO(buff);
			}
		}
	}


	//rerender the whole earth
	boost::shared_ptr<DataSourceEvent> spEvent(new DataSourceEvent());
	spEvent->setEventType(DataSourceEvent::knDataInvalid);
	this->notify(spEvent);
}

/*!
endTurn ends the current turn, and calls endRound if the last player's
turn is done.

\param pPlugin		The plugin that called the 'endturn' command.  This is sent to endRound if we call it.
*/
void PyxilizationDataSource::endTurn(PlugInDLL *pPlugin)
{
	//check if they're the only player left
	bool won = true;
	std::vector<boost::shared_ptr<PyxilizationFeature> >::iterator it;
	it = m_vecUnits.begin();
	while(it!=m_vecUnits.end())
	{
		int iPlayerID = (*it)->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8();
		if((iPlayerID != m_iCurPlayer) && (iPlayerID != m_nInvalidPlayerID))
		{
			won = false;
			break;
		}
		it++;
	}
	if(won == true)
	{
		//they've won!  Do something about it!
		char buff[256];
		sprintf(buff, "A winnar is player %d!", m_iCurPlayer);
		MessageBox(NULL, buff, "Game ovar", MB_OK);
	}
	else
	{
		//next turn
		m_iCurPlayer ++;
		if(m_iCurPlayer >= m_nPlayers)
		{
			m_iCurPlayer = 0;
			endRound(pPlugin);
		}
	}
}

/*!
startGame clears all the internal structures and whatnot and then
creates all the factories, airports and the starting units for the
number of players specified.  This gets called through the 'startgame'
command.

\param nPlayers		The number of players this game will have
*/
void PyxilizationDataSource::startGame(int nPlayers)
{
	m_nPlayers = nPlayers;
	m_iCurPlayer = 0;
	m_rnPoints[0] = 0;
	m_rnPoints[1] = 0;
	m_rnPoints[2] = 0;
	m_rnPoints[3] = 0;

	m_vecUnits.erase(m_vecUnits.begin(), m_vecUnits.end());

	//starting unit stuff
	boost::shared_ptr<PYXGeometry> geo4(new PYXCell(PYXIcosIndex("E-03040")));
	boost::shared_ptr<PYXGeometry> geo3(new PYXCell(PYXIcosIndex("5-40402")));
	boost::shared_ptr<PYXGeometry> geo2(new PYXCell(PYXIcosIndex("4-20205")));
	//boost::shared_ptr<PYXGeometry> geo2(new PYXCell(PYXIcosIndex("G-00040")));
	boost::shared_ptr<PYXGeometry> geo1(new PYXCell(PYXIcosIndex("G-00030")));
	switch(nPlayers)
	{
		case 4:
			newUnit(geo4, PyxilizationFeature::eTypeUnit,3, 5, m_nSoldierMovesPerTurn);
			setCellOwner(*geo4, 3);
		case 3:
			newUnit(geo3, PyxilizationFeature::eTypeUnit,2, 5, m_nSoldierMovesPerTurn);
			setCellOwner(*geo3, 2);
		case 2:
			newUnit(geo2, PyxilizationFeature::eTypeUnit,1, 5, m_nSoldierMovesPerTurn);
			setCellOwner(*geo2, 1);
	}
	newUnit(geo1, PyxilizationFeature::eTypeUnit, 0, 5, m_nSoldierMovesPerTurn);
	setCellOwner(*geo1, 0);

	//tank test
	boost::shared_ptr<PYXGeometry> tankcell(new PYXCell(PYXIcosIndex("G-00100")));
	newUnit(tankcell, PyxilizationFeature::eTypeTank, 0, 5, m_nSoldierMovesPerTurn);
	setCellOwner(*tankcell, 0);

	//default factory positions
	newBuilding("4-20105", PyxilizationFeature::eTypeFactory);
	newBuilding("1-04010", PyxilizationFeature::eTypeFactory);
	newBuilding("5-40302", PyxilizationFeature::eTypeFactory);
	newBuilding("D-05040", PyxilizationFeature::eTypeFactory);
	newBuilding("E-03050", PyxilizationFeature::eTypeFactory);
	newBuilding("S-01001", PyxilizationFeature::eTypeFactory);
	newBuilding("2-60601", PyxilizationFeature::eTypeFactory);
	newBuilding("3-30605", PyxilizationFeature::eTypeFactory);
	newBuilding("G-00020", PyxilizationFeature::eTypeFactory);
	newBuilding("8-06010", PyxilizationFeature::eTypeFactory);
	newBuilding("6-00500", PyxilizationFeature::eTypeFactory);
 
	//default airports positions
	newBuilding("4-40205", PyxilizationFeature::eTypeAirport);
	newBuilding("2-60200", PyxilizationFeature::eTypeAirport);
	newBuilding("6-06020", PyxilizationFeature::eTypeAirport);
	newBuilding("C-03003", PyxilizationFeature::eTypeAirport);
	newBuilding("1-06030", PyxilizationFeature::eTypeAirport);
	newBuilding("10-60602", PyxilizationFeature::eTypeAirport);
	newBuilding("5-03040", PyxilizationFeature::eTypeAirport);
	//newBuilding("G-00010", PyxilizationFeature::eTypeAirport);

	//rerender the whole earth
	boost::shared_ptr<DataSourceEvent> spEvent(new DataSourceEvent());
	spEvent->setEventType(DataSourceEvent::knDataInvalid);
	this->notify(spEvent);
}

/*!
This function transfers ownership of a cell to a given player.  This
includes giving them any planes or buildings on that cell, and changing
the colour of the player colouring coverage.

\param geo			A PYXCell that specifies the cell
\param iPlayerID	The player taking over the cell
*/
void PyxilizationDataSource::setCellOwner(PYXGeometry &geo, int iPlayerID)
{
	//change the colour
	PYXDataItem::SPtr spItem;
	bool bResult;

	if(PYXDataSourceManager::getInstance()->isInitialized() == true)
	{
		PYXDataSourceManager::getInstance()->isOpen(m_strAlphaURI, &bResult);
		if(bResult == true)
		{
			//get the alpha ds
			if(PYXDataSourceManager::getInstance()->getDataItem(m_strAlphaURI, &spItem))
			{
				boost::shared_ptr<PYXCoverage> spCoverage = spItem->getDSAsCoverage();
				PYXCell *cell;

				cell = dynamic_cast<PYXCell*>(&geo);
				if(cell!=0)
				{
					PYXValue val((uint8_t) iPlayerID);
					//set the cell's value to the player id specified
					spCoverage->setCoverageValue(val,cell->getIndex(),0);
				}
			}
		}
	}

	//set any factories or non-battle units to us
	std::vector<boost::shared_ptr<PyxilizationFeature> > vecFeatures = findFeatures(geo);
	std::vector<boost::shared_ptr<PyxilizationFeature> >::iterator it;

	it = vecFeatures.begin();
	while(it != vecFeatures.end())
	{
		(*it)->setFieldValue(PYXValue((uint8_t)iPlayerID), PyxilizationFeature::eFieldPlayerID);
		it++;
	}
}

//find which number of this array of ints is highest, returning the index
int highestIndex(int rgNumbers[], int nLen)
{
	int nHighest = 0;
	for(int n=1;n<nLen;n++)
	{
		if(rgNumbers[nHighest] < rgNumbers[n])
		{
			nHighest=n;
		}
	}
	return nHighest;
}

/*!
This resolves a battle between two units.  'Dice' are rolled to decide
how much health each will lose and then each loses the appropriate
amount of health.

\param		spAttacker	the unit attacking
\param		spDefender  the unit defending
\param		nAttackers  how many units the attacker is attacking with
\return		returns the health of the attacking unit
*/
int PyxilizationDataSource::battle(boost::shared_ptr<PyxilizationFeature> spAttacker, boost::shared_ptr<PyxilizationFeature> spDefender, int nAttackers)
{
	int nDefenders = spDefender->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8();

	int typeAttacker = spAttacker->getFieldValue(PyxilizationFeature::eFieldType).getUInt8();
	int typeDefender = spDefender->getFieldValue(PyxilizationFeature::eFieldType).getUInt8();
	int nAttackLoss = 0;
	int nDefendLoss = 0;

	if(typeAttacker != PyxilizationFeature::eTypePlane)
	{
		int rgAttackDice[m_nMaxSoldiersPerCell] = {0};
		int rgDefendDice[m_nMaxSoldiersPerCell] = {0};

		//roll the dice for each
		for(int n=0;n<nAttackers;n++)
		{
			rgAttackDice[n] = rand() % m_nDiceRange + 1;
		}
		for(int n=0;n<nDefenders;n++)
		{
			rgDefendDice[n] = rand() % m_nDiceRange + 1;
		}

		//decide how many rounds we're going to check
		//avoided using ? operator because the coding standards said it's a good idea
		int nRounds;
		if(nDefenders < nAttackers)
		{
			nRounds = nDefenders;
		}
		else
		{
			nRounds = nAttackers;
		}

		//let's do the rounds!
		for(int n=0;n<nRounds;n++)
		{
			int nAttack = highestIndex(rgAttackDice, nAttackers);
			int nDefend = highestIndex(rgDefendDice, nDefenders);

			if(rgAttackDice[nAttack] < rgDefendDice[nDefend])
			{
				nAttackLoss ++;
			}
			if(rgDefendDice[nDefend] < rgAttackDice[nAttack])
			{
				nDefendLoss ++;
			}
			if(rgDefendDice[nDefend] == rgAttackDice[nAttack])
			{
				//tanks don't lose a unit on a tie
				if(typeAttacker != PyxilizationFeature::eTypeTank)
				{
					nAttackLoss++;
				}
				if(typeDefender != PyxilizationFeature::eTypeTank)
				{
					nDefendLoss++;
				}
			}

			//make sure the dice we used are discarded
			rgAttackDice[nAttack] = -1;
			rgDefendDice[nDefend] = -1;
		}

		int nAttackHealth = spAttacker->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nAttackLoss;
		int nDefendHealth = spDefender->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nDefendLoss;

		spAttacker->setFieldValue(PYXValue((uint8_t)nAttackHealth), PyxilizationFeature::eFieldHealth);
		spDefender->setFieldValue(PYXValue((uint8_t)nDefendHealth), PyxilizationFeature::eFieldHealth);

		return nAttackHealth;
	}
	else
	{
		return spAttacker->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8();
	}
}

/*!
Find the strongest unit (for defense) in a vector of feature shared
pointers.  This is used to figure which unit should battle when a cell
is under attack.

\param vecFeatures	a vector of features to search through
\return the index value of the strongest unit in the vector
*/
int PyxilizationDataSource::getStrongest(std::vector<boost::shared_ptr<PyxilizationFeature> > vecFeatures)
{
	int nPos = -1;

	for(unsigned int n=0;n<vecFeatures.size();n++)
	{
		if(vecFeatures[n]->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == PyxilizationFeature::eTypeUnit)
		{
			if((nPos == -1) || (vecFeatures[nPos]->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() != PyxilizationFeature::eTypeTank))
			{
				nPos = n;
			}
		}
		else if(vecFeatures[n]->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == PyxilizationFeature::eTypeTank)
		{
			nPos = n;
		}
	}
	return nPos;
}

/*!
unloadCommand takes a text command to transfer units from a plane to a
cell.

\param commandLine	the command line
*/
void PyxilizationDataSource::unloadCommand(std::stringstream &commandLine)
{
	std::string strCell;
	std::string strNum;

	commandLine >> strCell;
	commandLine >> strNum;
	int nToUnLoad = atoi(strNum.c_str());

	if(nToUnLoad > 0)
	{
		boost::shared_ptr<PYXCell> cell(new PYXCell(PYXIcosIndex(strCell)));
		boost::shared_ptr<PyxilizationFeature> spPlane = findFeature(*cell, PyxilizationFeature::eTypePlane);
		boost::shared_ptr<PyxilizationFeature> spUnits = findFeature(*cell, PyxilizationFeature::eTypeUnit);

		if(spPlane!=0)
		{
			if(isWater(cell->getIndex()) == false)
			{
				if(spPlane->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() == m_iCurPlayer)
				{
					//make sure they're not unloading more than in plane
					if(nToUnLoad > spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8())
					{
						nToUnLoad = spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8();
					}

					//if we have units there already, join them
					if(spUnits!=0)
					{
						if(spUnits->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() + nToUnLoad > m_nMaxSoldiersPerCell)
						{
							nToUnLoad = m_nMaxSoldiersPerCell - spUnits->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8();
						}
						if(nToUnLoad > spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8())
						{
							nToUnLoad = spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8();
						}
						spUnits->setFieldValue(PYXValue((uint8_t)spUnits->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() + nToUnLoad), PyxilizationFeature::eFieldHealth);
						spPlane->setFieldValue(PYXValue((uint8_t)spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8() - nToUnLoad), PyxilizationFeature::eFieldLoaded);
					}
					else	//create new feature
					{
						if(nToUnLoad > m_nMaxSoldiersPerCell)
						{
							nToUnLoad = m_nMaxSoldiersPerCell;
						}
						boost::shared_ptr<PYXCell> spCell(new PYXCell(PYXIcosIndex(strCell)));
						boost::shared_ptr<PyxilizationFeature> spNewUnit = newUnit(spCell, PyxilizationFeature::eTypeUnit, m_iCurPlayer, nToUnLoad, 0);
						spPlane->setFieldValue(PYXValue((uint8_t)spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8() - nToUnLoad), PyxilizationFeature::eFieldLoaded);
						setCellOwner(*cell, m_iCurPlayer);
					}

					boost::shared_ptr<DataSourceEvent> spEvent(new DataSourceEvent());
					spEvent->setEventType(DataSourceEvent::knGeometryInvalid);
					boost::shared_ptr<PYXCell> spGeo(new PYXCell(cell->getIndex()));
					spEvent->setGeometry((spGeo));
					this->notify(spEvent);
				}
			}
		}
	}
}

/*!
loadCommand takes a text command to transfer units from a cell to a
plane.

\param commandLine	the command line
*/
void PyxilizationDataSource::loadCommand(std::stringstream &commandLine)
{
	std::string strCell;
	std::string strNum;

	commandLine >> strCell;
	commandLine >> strNum;
	int nToLoad = atoi(strNum.c_str());

	if(nToLoad > 0)
	{
		boost::shared_ptr<PYXCell> cell(new PYXCell(PYXIcosIndex(strCell)));
		boost::shared_ptr<PyxilizationFeature> spPlane = findFeature(*cell, PyxilizationFeature::eTypePlane);
		boost::shared_ptr<PyxilizationFeature> spUnits = findFeature(*cell, PyxilizationFeature::eTypeUnit);

		if((spPlane!=0)&&(spUnits!=0))
		{
			if(spUnits->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() == m_iCurPlayer)
			{
				if(nToLoad > spUnits->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8())
				{
					nToLoad = spUnits->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8();
				}
				if(nToLoad + spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8() > m_nMaxSoldierPlane)
				{
					nToLoad = m_nMaxSoldierPlane - spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8();
				}

				//set the values for the unit/plane
				spPlane->setFieldValue(PYXValue((uint8_t)spPlane->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8() + nToLoad), PyxilizationFeature::eFieldLoaded);
				spUnits->setFieldValue(PYXValue((uint8_t)spUnits->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nToLoad), PyxilizationFeature::eFieldHealth);

				//if we took them all, destroy the feature
				if(spUnits->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() == 0)
				{
					removeFeature(spUnits->getIDInt());
				}

				boost::shared_ptr<DataSourceEvent> spEvent(new DataSourceEvent());
				spEvent->setEventType(DataSourceEvent::knGeometryInvalid);
				boost::shared_ptr<PYXCell> spGeo(new PYXCell(cell->getIndex()));
				spEvent->setGeometry((spGeo));
				this->notify(spEvent);
			}
		}
	}
}

/*!
A very important function, moveCommand controls all action of units when
a move is attempted.  This means it manages movement, cell takeover, and
battles.  After it is complete, it tells the globe to rerender.

\param commandLine	the text command given
*/
void PyxilizationDataSource::moveCommand(std::stringstream &commandLine)
{
	std::string strCmd;
	std::string strID;
	std::string strTo;

	commandLine >> strCmd;
	int nToMove = atoi(strCmd.c_str());
	if(nToMove > 0)
	{
		commandLine >> strID;
		commandLine >> strTo;

		int nID = atoi(strID.c_str());

		//get the feature
		boost::shared_ptr<PyxilizationFeature> spFeature = findFeatureByID(nID);
		if(spFeature!=0)
		{
			//it is the right player's, right?
			if((spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() == m_iCurPlayer) && (spFeature->isMovable()))
			{
				if(spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8() > 0)
				{
					boost::shared_ptr<PYXCell> spToCell(new PYXCell(PYXIcosIndex(strTo)));
					boost::shared_ptr<PYXCell> spFromCell(new PYXCell(boost::dynamic_pointer_cast<const PYXCell, const PYXGeometry>(spFeature->getGeometry())->getIndex().toString()));

					if((spToCell) && (spFromCell))
					{
						spToCell->setCellResolution(getGameResolution());
						spFromCell->setCellResolution(getGameResolution());

						//check if the move is valid
						bool isValid = false;

						//plane stuff - goes by distance instead of neighbour-ness
						if(spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == PyxilizationFeature::eTypePlane)
						{
							CoordLatLon coordTo;
							CoordLatLon coordFrom;
							SnyderProjection::getInstance()->pyxisToNative(spToCell->getIndex(), &coordTo);
							SnyderProjection::getInstance()->pyxisToNative(spFromCell->getIndex(), &coordFrom);

							double dDistance = WGS84::getInstance()->calcDistance(coordTo, coordFrom);
							if(dDistance < m_nFarthestFlight)
							{
								isValid = true;
							}
						}
						else //neighbour checking
						{
							if(areNeighbours(spFromCell, spToCell) == true)
							{
								if(!isWater(spToCell->getIndex()))
								{
									isValid = true;
								}
							}
						}

						if(isValid == true)		//valid move location
						{
							if(nToMove > spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8())
							{
								nToMove = spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8();
							}

							//ok, let's find out what is on the other cell
							std::vector<boost::shared_ptr<PyxilizationFeature> > vecFeatures = findFeatures(*spToCell);

							// if there's something there
							if(vecFeatures.size() > 0)
							{
								int iToPlayerID = vecFeatures[0]->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8();

								//if we already own this cell
								if(iToPlayerID == m_iCurPlayer)
								{
									int iFeaturePos = -1;
									//do we have one of this unit already?
									for(unsigned int n=0;n<vecFeatures.size();n++)
									{
										if(vecFeatures[n]->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8())
										{
											iFeaturePos = n;
										}
									}
									if(iFeaturePos == -1)
									{
										//no such unit, create one
										if(spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() != PyxilizationFeature::eTypePlane)
										{
											setCellOwner(*spToCell, m_iCurPlayer);
										}
										boost::shared_ptr<PyxilizationFeature> spNewUnit = newUnit(spToCell, (PyxilizationFeature::Types)spFeature->getFieldValueByName("type").getUInt8(), m_iCurPlayer, nToMove, spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8() - 1);
										spNewUnit->setFieldValue(PYXValue((uint8_t) spFeature->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8()), PyxilizationFeature::eFieldLoaded);
										spNewUnit->setFieldValue(PYXValue((uint8_t) spFeature->getFieldValue(PyxilizationFeature::eFieldFuel).getUInt8()), PyxilizationFeature::eFieldFuel);
										spFeature->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nToMove), PyxilizationFeature::eFieldHealth);
									}
									else
									{
										//make sure they're not trying to put in too many
										if(vecFeatures[iFeaturePos]->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() + nToMove > m_nMaxSoldiersPerCell)
										{
											nToMove = m_nMaxSoldiersPerCell - vecFeatures[iFeaturePos]->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8();
										}

										spFeature->setFieldValue(PYXValue((uint8_t) spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nToMove), PyxilizationFeature::eFieldHealth);
										vecFeatures[iFeaturePos]->setFieldValue(PYXValue((uint8_t) vecFeatures[iFeaturePos]->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() + nToMove), PyxilizationFeature::eFieldHealth);
									}
								}
								else //if someone else owns the cell
								{
									int iFeaturePos = -1;
									iFeaturePos = getStrongest(vecFeatures);
									//boost::shared_ptr<PyxilizationFeature> spNewUnit();
									//if they don't own any battle units, just take it over
									if(iFeaturePos == -1)
									{
										if(spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() != PyxilizationFeature::eTypePlane)
										{
											setCellOwner(*spToCell, m_iCurPlayer);
										}
										
										boost::shared_ptr<PyxilizationFeature> spNewUnit =  newUnit(spToCell, (PyxilizationFeature::Types)spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8(), m_iCurPlayer, nToMove, spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8() - 1);
										spNewUnit->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8()), PyxilizationFeature::eFieldLoaded);
										spNewUnit->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldFuel).getUInt8()), PyxilizationFeature::eFieldFuel);
										spFeature->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nToMove), PyxilizationFeature::eFieldHealth);
									}
									else
									{
										//do the battle
										int nResult = battle(spFeature,vecFeatures[iFeaturePos], nToMove);
										spFeature->setFieldValue(PYXValue((uint8_t)0), PyxilizationFeature::eFieldMovesLeft);

										//if the attacker defeated all the defenders...
										if(vecFeatures[iFeaturePos]->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() == 0)
										{
											removeFeature(vecFeatures[iFeaturePos]->getIDInt());
											if(nResult > 0) //if they have at least one attacker left
											{
												setCellOwner(*spToCell, m_iCurPlayer);
												newUnit(spToCell, (PyxilizationFeature::Types)spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8(),m_iCurPlayer, nResult, spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8() - 1);
												spFeature->setFieldValue(PYXValue((uint8_t)(spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nResult)), PyxilizationFeature::eFieldHealth);
												spFeature->setFieldValue(PYXValue((uint8_t)0), PyxilizationFeature::eFieldMovesLeft);
											}
										}
									}
								}
							}
							else	//nobody owns this cell
							{
								if(spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() != PyxilizationFeature::eTypePlane)
								{
									setCellOwner(*spToCell, m_iCurPlayer);
								}
								boost::shared_ptr<PyxilizationFeature> spNewUnit = newUnit(spToCell, (PyxilizationFeature::Types)spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8(), m_iCurPlayer, nToMove, spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8() - 1);
								spNewUnit->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldLoaded).getUInt8()), PyxilizationFeature::eFieldLoaded);
								spNewUnit->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldFuel).getUInt8()), PyxilizationFeature::eFieldFuel);
								spFeature->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() - nToMove), PyxilizationFeature::eFieldHealth);
							}

							if(spFeature->getFieldValue(PyxilizationFeature::eFieldHealth).getUInt8() == 0)
							{
								removeFeature(spFeature->getIDInt());
							}

							//less one movesleft
							spFeature->setFieldValue(PYXValue((uint8_t)spFeature->getFieldValue(PyxilizationFeature::eFieldMovesLeft).getUInt8()-1), PyxilizationFeature::eFieldMovesLeft);

							//redraw notifications (to and from)
							TRACE_INFO("Redrawing...");
							boost::shared_ptr<DataSourceEvent> spEvent(new DataSourceEvent());
							spEvent->setEventType(DataSourceEvent::knGeometryInvalid);
							boost::shared_ptr<PYXCell> spGeoTo(new PYXCell(spToCell->getIndex()));
							spEvent->setGeometry((spGeoTo));
							this->notify(spEvent);

							boost::shared_ptr<DataSourceEvent> spEvent2(new DataSourceEvent());
							spEvent2->setEventType(DataSourceEvent::knGeometryInvalid);
							boost::shared_ptr<PYXCell> spGeoFrom(new PYXCell(spFromCell->getIndex()));
							spEvent2->setGeometry(spGeoFrom);
							this->notify(spEvent2);
						}
					}
				}
			}
		}
	}
}

/*!
This command is given to change a factory's production unit.

\param commandLine	the command line
*/
void PyxilizationDataSource::setTypeCommand(std::stringstream &commandLine)
{
	int nID;
	int nType;

	commandLine >> nID;
	commandLine >> nType;

	boost::shared_ptr<PyxilizationFeature> spFeature = findFeatureByID(nID);
	if(spFeature!=0)
	{
		if(spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == PyxilizationFeature::eTypeFactory)
		{
			if(spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() == m_iCurPlayer)
			{
				if((nType == PyxilizationFeature::eTypeUnit)||(nType == PyxilizationFeature::eTypeTank))
				{
					if(spFeature->getFieldValue(PyxilizationFeature::eFieldBuildType).getUInt8() != nType)
					{
						//ok, valid factory and new type - let's set it
						spFeature->setFieldValue(PYXValue((uint8_t)nType), PyxilizationFeature::eFieldBuildType);
						//reset the build time
						if(nType == PyxilizationFeature::eTypeUnit)
						{
							spFeature->setFieldValue(PYXValue((uint8_t)m_nSoldierBuildTime), PyxilizationFeature::eFieldMovesLeft);
						}
						else
						{
							spFeature->setFieldValue(PYXValue((uint8_t)m_nTankBuildTime), PyxilizationFeature::eFieldMovesLeft);
						}
					}
				}
			}
		}
	}
}

/*!
buyCommand receives a text command to give a player a new unit instantly
and deduct the cost from their points.

\param commandLine		the command line given
*/
void PyxilizationDataSource::buyCommand(std::stringstream &commandLine)
{
	int nID;
	int nType;

	commandLine >> nID;
	commandLine >> nType;

	boost::shared_ptr<PyxilizationFeature> spFeature = findFeatureByID(nID);
	if(spFeature!=0)
	{
		if(spFeature->getFieldValue(PyxilizationFeature::eFieldType).getUInt8() == PyxilizationFeature::eTypeFactory)
		{
			if(spFeature->getFieldValue(PyxilizationFeature::eFieldPlayerID).getUInt8() == m_iCurPlayer)
			{
				switch(nType)
				{
					//if they want to build a soldier...
					case PyxilizationFeature::eTypeUnit:
						if(m_rnPoints[m_iCurPlayer] >= m_nUnitCost)
						{
							m_rnPoints[m_iCurPlayer] -= m_nUnitCost;
							boost::shared_ptr<PYXCell> spCell(new PYXCell());
							*spCell = *(boost::dynamic_pointer_cast<const PYXCell, const PYXGeometry>(spFeature->getGeometry()));
							boost::shared_ptr<PyxilizationFeature> spUnitFeature = findFeature(*spCell, PyxilizationFeature::eTypeUnit);
							if(spUnitFeature!=0) //feature already exists here, add +1 to health
							{
								PYXValue val = spUnitFeature->getFieldValue(PyxilizationFeature::eFieldHealth);
								if(val.getUInt8() < m_nMaxSoldiersPerCell)
								{
									spUnitFeature->setFieldValue(PYXValue((uint8_t) val.getUInt8() + 1), PyxilizationFeature::eFieldHealth);
								}
							}
							else	//there's no feature - create one
							{
								newUnit(spCell, PyxilizationFeature::eTypeUnit ,m_iCurPlayer, 1, m_nSoldierMovesPerTurn);
							}
						}
					break;
					//if they want to build a tank
					case PyxilizationFeature::eTypeTank:
						if(m_rnPoints[m_iCurPlayer] >= m_nTankCost)
						{
							m_rnPoints[m_iCurPlayer] -= m_nTankCost;
							boost::shared_ptr<PYXCell> spCell(new PYXCell());
							*spCell = *(boost::dynamic_pointer_cast<const PYXCell, const PYXGeometry>(spFeature->getGeometry()));
							boost::shared_ptr<PyxilizationFeature> spUnitFeature = findFeature(*spCell, PyxilizationFeature::eTypeTank);
							if(spUnitFeature!=0) //feature already exists here, add +1 to health
							{
								PYXValue val = spUnitFeature->getFieldValue(PyxilizationFeature::eFieldHealth);
								if(val.getUInt8() < m_nMaxSoldiersPerCell)
								{
									spUnitFeature->setFieldValue(PYXValue((uint8_t) val.getUInt8() + 1), PyxilizationFeature::eFieldHealth);
								}
							}
							else	//there's no feature - create one
							{
								newUnit(spCell, PyxilizationFeature::eTypeTank ,m_iCurPlayer, 1, m_nSoldierMovesPerTurn);
							}
						}
					break;
				}
			}
		}
	}
}

/*!
command is the fundamental function of communication reception from the
plug-in.  This is how players interact with the data inside the
data source.  This function checks what type of command is being given
and then directs it to the appropriate function for further parsing.

\param commandLine		the actual text command line
\param pPlugin			a pointer to the plug-in calling this (used only with the 'endturn' command)
*/
void PyxilizationDataSource::command(std::stringstream &commandLine, PlugInDLL *pPlugin)
{
	std::string strCmd;

	commandLine >> strCmd;

	if(strCmd == "move")
	{
		moveCommand(commandLine);
	}
	else if(strCmd == "endturn")
	{
		endTurn(pPlugin);
	}
	else if(strCmd == "load")
	{
		loadCommand(commandLine);
	}
	else if(strCmd == "unload")
	{
		unloadCommand(commandLine);
	}
	else if(strCmd == "startgame")
	{
		std::string strPlayers;
		commandLine >> strPlayers;
		startGame(atoi(strPlayers.c_str()));
	}
	else if(strCmd == "settype")
	{
		setTypeCommand(commandLine);
	}
	else if(strCmd == "buy")
	{
		buyCommand(commandLine);
	}
	else if(strCmd == "getpoints")
	{
		//simple command to return number of points a player has
		int iID;
		commandLine >> iID;

		if(iID < m_nPlayers)
		{
			char buff[256];
			sprintf(buff, "pyxilization points %d\n", m_rnPoints[iID]);
			//send to plugin
			//if no plugin, send through TRACE_INFO
			if(pPlugin!=0)
			{
				pPlugin->addCmdResponse(buff);
			}
			else
			{
				TRACE_INFO(buff);
			}
		}
	}
}

/*!
The constructor sets up all internal fields and structures, and
attempts to make the resolution 1 pyxis grid visible.
*/
PyxilizationDataSource::PyxilizationDataSource()
{
	srand( GetTickCount() );
	setResolution(5);
	setGameResolution(5);
	setType(knVector);

	m_iCurPlayer = 0;
	m_nPlayers = 0;

	//set up DS definition - don't really use it
	PYXFieldDefinition field("my awesome field", PYXFieldDefinition::knContextNone, PYXValue::knUInt8,1);
	PYXTableDefinition::SPtr table = PYXTableDefinition::create();
	table->addFieldDefinition(field);
	setFeatureDefinition(table);

	//make grid visible
	if(PYXDataSourceManager::getInstance() != 0)
	{
		if(PYXDataSourceManager::getInstance()->isInitialized() == true)
		{
			std::string strURI(PYXDynamicGlobalGridDataSource::kstrDynamicGlobalGrid);

			//set grid type
			boost::shared_ptr<PYXDataSource> spDS;
			PYXDataSourceManager::getInstance()->getDataSource(strURI, &spDS);
			boost::shared_ptr<PYXDynamicGlobalGridDataSource> spGrid = boost::dynamic_pointer_cast<PYXDynamicGlobalGridDataSource, PYXDataSource>(spDS);

			//set it to pyxis type grid
			spGrid->setDatum(Geotrans::knPYXIS);

			//enable grid
			boost::shared_ptr< DataSourceToggleVisibilityCmd > spCmd(
				DataSourceToggleVisibilityCmd::create(strURI)	);
			PYXGlobal::executeCommand(spCmd);
		}
	}

//	startGame(2);
}

/*!
Does just what you'd expect.

\param nRes		What resolution to set the DS to be to the TVT
*/
void PyxilizationDataSource::setResolution(int nRes)
{
	m_spGeomDS.reset(new PYXGlobalGeometry(nRes));
}

/*!
Get an iterator to all the features inside the Data Source.

\return An intrusive pointer to the iterator
*/
PYXPointer<PYXFeatureIterator> PyxilizationDataSource::getFeatureIterator() const
{
	return boost::dynamic_pointer_cast<PYXFeatureIterator, PyxilizationDataSourceIterator>(PyxilizationDataSourceIterator::create(this));
}

/*!
Get an iterator to the features inside the Data Source that intersect a
particular geography.

\param geography	The geography the feature must reside in.
\return An intrusive pointer to the iterator
*/
PYXPointer<PYXFeatureIterator> PyxilizationDataSource::getFeatureIterator(const PYXGeometry& geometry) const
{
	PyxilizationStyleMapper::setDisplayResolution(geometry.getCellResolution());
 	return boost::dynamic_pointer_cast<PYXFeatureIterator, PyxilizationDataSourceIterator>(PyxilizationDataSourceIterator::create(this, geometry));
}

/*!
Finds a feature in the Data Source that matches the ID given.

\param strFeatureID		The ID to find
\return A shared pointer to the feature
*/
boost::shared_ptr<const PYXFeature> PyxilizationDataSource::getFeature(const std::string& strFeatureID) const
{
	//where's waldo?
	std::vector<boost::shared_ptr<PyxilizationFeature> >::const_iterator it = m_vecUnits.begin();
	while(it != m_vecUnits.end())
	{
		if((*it)->getID() == strFeatureID)
			return *it;
		it++;
	}
	return boost::shared_ptr<const PYXFeature>();
}

 