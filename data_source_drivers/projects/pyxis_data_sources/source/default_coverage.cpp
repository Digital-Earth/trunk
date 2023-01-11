/******************************************************************************
default_coverage.cpp

begin		: 2006-05-30
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE

#include "default_coverage.h"

// local includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/geometry/geometry_serializer.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

//! The class name, for notifier/observer debugging
const std::string PYXDefaultCoverage::kstrScope = "PYXDefaultCoverage";

/*! 
The file extension for our data files will be .d## with the tile depth
filled in for the ## portion.
*/
const std::string PYXDefaultCoverage::kstrFileExtension = ".d";

namespace
{
	//! The name of the metadata file (contains the data source definition).
	const std::string kstrMetaDataFileName = "defn.txt";

	//! The name of the definition data file (contains data source definition values).
	const std::string kstrDataFileName = "defn_data.txt";

	//! The name of the coverage meta data file (contains data definition).
	const std::string kstrCoverageMetaDataFileName = "coverage_defn.txt";

	//! The name of the geometry file.
	const std::string kstrGeometryFileName = "geometry.pyx";

} //namespace

//! Tester class
Tester<PYXDefaultCoverage> gTester;

//! Test method
void PYXDefaultCoverage::test()
{
	// This class is partially tested through the cache class

	// TODO: Write unit tests.
}

/*!
Constructor sets data members to sensible null values.
*/
PYXDefaultCoverage::PYXDefaultCoverage() :
	m_bWritable(true),
	m_bPersistent(true),
	m_defaultTileDepth(-1),
	m_nCellResolution(-1),
	m_vecTypes(),
	m_vecCounts()
{
	m_spGeometry = PYXTileCollection::create();
	m_spDefn = PYXTableDefinition::create();
	m_spCovDefinition = PYXTableDefinition::create();
	m_spCovDefinition->addFieldDefinition("RGB", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
	m_tileCache.attach(this);
}

/*!
Destructor.
*/
PYXDefaultCoverage::~PYXDefaultCoverage()
{
	close();
	m_tileCache.detach(this);
}

void PYXDefaultCoverage::initTypeArrays()
{
	m_vecTypes.clear();
	m_vecCounts.clear();

	int nFieldCount = m_spCovDefinition->getFieldCount();
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		const PYXFieldDefinition& defn = 
			m_spCovDefinition->getFieldDefinition(nField);
		m_vecTypes.push_back(defn.getType());
		m_vecCounts.push_back(defn.getCount());
	}
}

/*!
Open an existing data set for reading.

\param	strDir				Name of directory containing the files.

\return true on success, false on any failure.
*/
bool PYXDefaultCoverage::openReadOnly(const std::string& strDir)
{
	TRACE_INFO("Opening PYXDefaultCoverage read/only in '" << strDir << "'.");

	// directory path argument should not be empty
	assert(!strDir.empty() && "Invalid argument.");

	// make sure it is a directory
	boost::filesystem::path directoryPath = FileUtils::stringToPath(strDir);
	if (!FileUtils::exists(directoryPath) || 
		!FileUtils::isDirectory(directoryPath))
	{
		TRACE_INFO("Invalid directory '" << strDir << "'.");
		return false;
	}

	// load the definitions
	try
	{
		if (!loadDefinitions(strDir))
		{
			return false;
		}
		loadGeometry(strDir);
		initTypeArrays();
	}
	catch (...)
	{
		return false;
	}

	// set the new name
	setName(strDir);

	return true;
}

/*!
Open an existing data set read/write.

\param	strDir				Name of directory containing the files.

\return true on success, false on any failure.
*/
bool PYXDefaultCoverage::openReadWrite(const std::string& strDir)
{
	TRACE_INFO("Opening PYXDefaultCoverage read/write in '" << strDir << "'.");

	if (openReadOnly(strDir))
	{
		m_bWritable = true;
		return true;
	}
	else
	{
		return false;
	}
}

/*!
Create a new data set and open it read/write, OR, if the folder strDir exists,
just open it read/write.

If the folder strDir does exist, it will be created.

Parameter nTileResolution should be less than nCellResolution, but if you don't
care, you can set nTileResolution to 0 or negative; the code will automatically
choose the tile resolution so the tiles are no deeper than knDefaultTileDepth.

\param	strDir				Name of directory to contain the files.
\param	defn				Describes the data source.
\param	vecValues			Vector of values for the definition.
\param	coverageDefn		Describes the coverage.
\param	nCellResolution		Resolution of the data cells.
\param	nTileDepth			Desired tile depth, or 0 to force auto-selection.

\return true on success, false on any failure.
*/
bool PYXDefaultCoverage::openReadWrite(
	const std::string& strDir,
	const PYXTableDefinition& defn,
	const std::vector<PYXValue>& vecValues,
	const PYXTableDefinition& coverageDefn,
	int nCellResolution,
	int nTileDepth	)
{
	if (openReadWrite(strDir))
	{
		// if the data set already exists and opened OK, we're done
		return true;
	}

	TRACE_INFO("Creating new PYXDefaultCoverage in '" << strDir << "'.");
	assert(coverageDefn.getFieldCount() != 0 && "Assert no fields in Meta data.");
	setName(strDir);
	setCellResolution(nCellResolution);
	setDefaultTileDepth(nTileDepth);
	m_bWritable = true;
	m_spCovDefinition = PYXTableDefinition::create(coverageDefn);
	try
	{
		// copy definition from input
		m_spDefn = defn.clone();
		if (!m_spCovDefinition)
		{
			// copy values from input
			setFieldValues(vecValues);
		}
		initTypeArrays();

		// create the empty directory
		boost::filesystem::path dirPath = FileUtils::stringToPath(strDir);
		if (!FileUtils::exists(dirPath))
		{
			boost::filesystem::create_directories(dirPath);
		}

		if (m_bPersistent)
		{
			saveDefinitions(getName());
		}
	}
	catch (...)
	{
		TRACE_INFO( "Unable to open PYXIS data source. Root directory: '" << strDir << "'. File extension: '.pyx'."	);
		return false;
	}

	return true;
}

/*!
Close a data set.
*/
void PYXDefaultCoverage::close()
{
	if (m_bWritable && m_bPersistent)
	{
		// save definitions
		saveDefinitions(getName());

		// save the geometry
		saveGeometry(getName());

		// save data and free memory
		m_tileCache.clearAllTiles();
	}
	else
	{
		// just free memory
		m_tileCache.detach(this);
		m_tileCache.clearAllTiles();
		m_tileCache.attach(this);
	}

	// Ensure that a subsequent close() call (e.g. via destructor) will
	// have no effect.
	m_bWritable = false;
}

PYXPointer<PYXValueTile> PYXDefaultCoverage::getTileFromMemory(const PYXTile& tile) const
{
	return m_tileCache.getTile(tile);
}

/*!
Look up a tile in the cache.  If it's not there, but its geometry is included in
m_tileCollection, recover it from disk.

\param tile		The tile geometry to look for.
\return			Shared pointer to the tile in the cache, or an empty shared
pointer if the tile is not part of this data source's geometry.
*/
PYXPointer<PYXValueTile> PYXDefaultCoverage::getTile(const PYXTile& tile) const
{
	{
		boost::recursive_mutex::scoped_lock lock(m_tileHintMutex);
		// check if the tile was the last one used.
		if (m_spTileHint)
		{
			if (m_spTileHint->getTile() == tile)
			{
				return m_spTileHint;
			}
		}
	}

	// look up the tile
	PYXPointer<PYXValueTile> spVT = m_tileCache.getTile(tile);
	if (!spVT)
	{
		// in geometry but not in cache: try and retrieve from disk now
		spVT = recoverTile(tile);

		// we did not find it on disk
		if (!spVT)
		{
			return spVT;
		}

		m_tileCache.add(spVT);
	}

	{		
		boost::recursive_mutex::scoped_lock lock(m_tileHintMutex);
		// tile is now definitely in the cache: return it
		m_spTileHint = spVT;
	}

	return spVT;
}

//! Calculate the tile resolution for a given cell index accounting for the minimum resolution.
int PYXDefaultCoverage::tileResolution (const PYXIcosIndex& index, int depth) const
{
	if (depth == -1)
	{
		depth = PYXTile::knDefaultTileDepth;
	}

	int tileResolution = index.getResolution() - depth;
	if (tileResolution < PYXIcosIndex::knMinSubRes)
	{
		tileResolution = PYXIcosIndex::knMinSubRes;
	}
	return tileResolution;
}

PYXTile PYXDefaultCoverage::getDefaultTile(const PYXIcosIndex& index) const
{
	PYXIcosIndex tileIndex(index);
	tileIndex.setResolution(tileResolution(index, m_defaultTileDepth));
	PYXTile tile(tileIndex, index.getResolution());
	return tile;
}

/*!
Helper function to locate a tile that contains the cell we are trying to find.
Because we support many depths of tile in the same storage mechanism, we have
many different tiles that could contain the cell.  First, we will look at the
last used tile depth, and see if it is in that tile, and then we will look at
the default tile depth to see if it is in there.  Otherwise, we must do an exhaustive
search for all tiles that could contain it.  With normal usage, the exhaustive search
mechanism will never come into play, as the normal usage would be for tiles all of the
same depth (like when it is driven through the visualization).
*/
PYXPointer<PYXValueTile> PYXDefaultCoverage::findTileForCell(const PYXIcosIndex& index) const
{
	int defaultDepth = -1;

	{
		boost::recursive_mutex::scoped_lock lock(m_tileHintMutex);
		// check to see if the last tile has the cell that we are interested in
		if (m_spTileHint)
		{
			if (m_spTileHint->getTile().hasIndex(index))
			{
				return m_spTileHint;
			}
		}
	}

	PYXIcosIndex testIndex(index);
	testIndex.setResolution(tileResolution(index, m_defaultTileDepth));
	PYXTile testTile(testIndex, index.getResolution());

	PYXPointer<PYXValueTile> spVT = getTile(testTile);
	if (spVT)
	{
		assert(spVT->getTile().getCellResolution() == index.getResolution());
		return spVT;
	}

	// note the resolution that we tried so we don't test it again
	defaultDepth = testTile.getDepth();

	// the exhaustive search...
	for (int res = PYXIcosIndex::knMinSubRes; res <= index.getResolution(); res++)
	{
		PYXIcosIndex testIndex(index);
		testIndex.setResolution(res);
		PYXTile testTile(testIndex, index.getResolution());
		if (testTile.getDepth() != defaultDepth)
		{
			PYXPointer<PYXValueTile> spVT = getTile(testTile);
			if (spVT)
			{
				assert(spVT->getTile().getCellResolution() == index.getResolution());
				return spVT;
			}
		}
	}

	return PYXPointer<PYXValueTile>();
}

/*!
Get the field value at the specified PYXIS index.  If pbInitialized is non-null,
the caller-supplied bool variable to which it points will be updated to indicate
whether or not the indexed field has ever been set.

\param	index			The PYXIS index
\param	nFieldIndex		The field index
\param	pbInitialized	Optional pointer to variable to receive empty/null distinction.

\return	The value.
*/
PYXValue PYXDefaultCoverage::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex,
	bool *pbInitialized	) const
{
	PYXCell cell(index);

	if (!m_spGeometry->intersects(cell))
	{
		// no: return null, with interpretation "yes, its a null!"
		if (pbInitialized != 0)
		{
			*pbInitialized = true;
		}
		return PYXValue();
	}

	PYXPointer<PYXValueTile> spVT = findTileForCell(index);

	if (spVT)
	{
		// yes: get requested cell value and return it
		return spVT->getValue(index, nFieldIndex, pbInitialized);
	}
	else
	{
		// no: return null, with interpretation "not yet initialized"
		if (pbInitialized != 0)
		{
			*pbInitialized = false;
		}
		return PYXValue();
	}
}

/*!
Get coverage values for an entire tile (all fields).

\param	tile	Defines requested tile (root index, depth).

\return Shared pointer to value tile.
*/
PYXPointer<PYXValueTile>
	PYXDefaultCoverage::getCoverageTile(const PYXTile& tile) const
{
	// return empty tile if requested tile is not in our geometry
	if (!m_spGeometry->intersects((tile)))
	{
		PYXPointer<PYXValueTile> spEmptyTile = PYXValueTile::create(tile, m_vecTypes, m_vecCounts);
		spEmptyTile->setIsComplete(true);
		return spEmptyTile;
	}

	return getTile(tile);
}

/*!
Set the coverage value at a specific PYXIS index.

\param	value		The value
\param	index		The PYXIS index
\param	nFieldIndex	The field index.
*/
void PYXDefaultCoverage::setCoverageValue(
	const PYXValue& value,	
	const PYXIcosIndex& index,
	int nFieldIndex	) 
{
	if (!m_bWritable)
	{
		assert(false && "attempt to write to non-writable Pyxis data set");
		return;
	}

	PYXPointer<PYXValueTile> spVT = findTileForCell(index);
	if (!spVT)
	{
		// we don't have that tile: create it now and add to the cache
		PYXTile tile = getDefaultTile(index);
		spVT = PYXValueTile::create(tile, m_vecTypes, m_vecCounts);
		m_tileCache.add(spVT);

		//PYXTileCollection* pTileCollection = dynamic_cast<PYXTileCollection*>(m_spGeometry.get());
		//if (pTileCollection != 0)
		//{
		//	pTileCollection->addTile(tile.getRootIndex(), index.getResolution());

		//// If this geometry is world-spanning, use a PYXGlobalGeometry
		//if (m_nTileResolution <= PYXMath::knMaxRelResolution)
		//{
		//	if (pTileCollection->getGeometryCount() == PYXIcosMath::getCellCount(m_nTileResolution))
		//	{
		//		m_spGeometry = PYXGlobalGeometry::create(m_nCellResolution);
		//	}
		//}
		//}
	}

	// set requested value
	spVT->setValue(index, nFieldIndex, value);
}

/*!
Set coverage values for an entire tile (all fields).

\param	spInputTile	Shared pointer to a PYXValueTile containing value data.
*/
void PYXDefaultCoverage::setCoverageTile(PYXPointer<PYXValueTile> spInputTile)
{
	PYXPointer<PYXValueTile> spExistingTile = m_tileCache.getTile(spInputTile->getTile());

	if (!spExistingTile)
	{
		m_tileCache.add(spInputTile);
		if (m_bPersistent)
		{
			persistTile(spInputTile.get());
		}
	}
	else
	{
		// Copy the new tile over the old tile.
		int nFieldCount = spInputTile->getNumberOfDataChannels();
		int nCellCount = spInputTile->getNumberOfCells();
		assert(nFieldCount == spExistingTile->getNumberOfDataChannels()  && "Attempt to write over a tile with a different number of fields.");
		assert(nCellCount == spExistingTile->getNumberOfCells()  && "Attempt to write over a tile with a different number of cells.");
		for (int nFieldIndex = 0; nFieldIndex < nFieldCount; ++nFieldIndex)
		{
			for (int nIndexOffset = 0; nIndexOffset < nCellCount; ++nIndexOffset)
			{
				spExistingTile->setValue(nIndexOffset, nFieldIndex, spInputTile->getValue(nIndexOffset, nFieldIndex));
			}
		}
		if (m_bPersistent)
		{
			persistTile(spExistingTile.get());
		}
	}
}

/*!
Handle notification events.

\param	spEvent	Pointer to event object.
*/
void PYXDefaultCoverage::updateObserverImpl(PYXPointer< NotifierEvent > spEvent)
{
	PYXTileCacheEvent<PYXValueTile>* pVTEvent =
		dynamic_cast<PYXTileCacheEvent<PYXValueTile>*>(spEvent.get());
	if (pVTEvent == 0)
	{
		return;
	}

	switch(pVTEvent->getEventType())
	{

	case PYXTileCacheEvent<PYXValueTile>::knDeletingTile:
		{
			if (m_bWritable)
			{
				if (pVTEvent->getTile())
				{
					// valid pointer: persist just one tile
					persistTile(pVTEvent->getTile().get());
				}
				else
				{
					TRACE_ERROR ("Deleting tile event with no tile specified.");
				}
			}
			break;
		}

	case PYXTileCacheEvent<PYXValueTile>::knDeleteAllTiles:
		{
			if (m_bWritable)
			{
				persistAllTiles();
			}
			break;
		}

	default:
		{
			PYXTHROW(
				PYXDataSourceException, 
				"Undefined event '" << pVTEvent->getEventType() << "'.");
		}
	}
}

/*!
Persist (serialize) a tile from our cache to disk.
*/
void PYXDefaultCoverage::persistTile(PYXValueTile *pTile)
{
	if (pTile->isDirty())
	{
		boost::filesystem::path tileFile = toFileName(pTile->getTile());
		boost::filesystem::path tilePath = tileFile.branch_path();

		// make sure the directory exists.
		if (!FileUtils::exists(tilePath))
		{
			boost::filesystem::create_directories(tilePath);
		}
		auto fileName = FileUtils::pathToString(tileFile);
		auto random = rand();
		auto tmpFileName = fileName + +"." + StringUtils::toString(random) + ".tmp";

		// making sure the file and tmp file do not exist
		if (FileUtils::exists(tmpFileName))
		{
			boost::filesystem::remove(tmpFileName);
		}

		// write to tmp file
		std::ofstream out(tmpFileName, std::ios_base::binary);
		pTile->serialize(out);
		out.close();

		//remove current file
		if (FileUtils::exists(fileName))
		{
			boost::filesystem::remove(fileName);
		}
		// rename the tmp file to actual file
		std::rename(tmpFileName.c_str(), fileName.c_str());
	}
}

/*!
Persist (serialize) all tiles in cache immediately to disk.
*/
void PYXDefaultCoverage::persistAllTiles()
{
	// persist all tiles in the cache now
	boost::recursive_mutex::scoped_lock lock(m_tileCache.getMutex());
	PYXTileCache<PYXValueTile>::TileMap::iterator itTiles = m_tileCache.getCache()->begin();
	for (; itTiles != m_tileCache.getCache()->end(); ++itTiles)
	{
		persistTile(itTiles->second.get());
	}
}

/*!
Convert a PYXTile into the file name that will be used to store the tile
in this cache.

\return    A fully qualified file name.
*/
const boost::filesystem::path PYXDefaultCoverage::toFileName(const PYXTile &tile) const
{
	boost::filesystem::path tileFile = FileUtils::stringToPath(getName());
	std::string strIndex = tile.getRootIndex().toString();

	// adjust for two character "first digit" (before the dash)
	// so that all of the same resolutions end up in the same directory.
	unsigned int trimLength = 4;
	if (strIndex.substr(1,1) != "-")
	{
		trimLength = 5;
	}

	while (strIndex.length() > trimLength)
	{
		tileFile /= strIndex.substr(0,trimLength);
		strIndex = strIndex.substr(trimLength,strIndex.length());
		trimLength = 4;
	}

	tileFile /= (strIndex + kstrFileExtension + intToString(tile.getDepth(), 2));
	return 	tileFile;
}

/*!
Recover a previously persisted tile from disk.

\return		Pointer to a new PYXValueTile from disk or a null PYXPointer if the
file does not exist.
*/
PYXPointer<PYXValueTile> PYXDefaultCoverage::recoverTile(const PYXTile &tile) const
{
	boost::filesystem::path filePath = toFileName(tile);
	try
	{
		if (FileUtils::exists(filePath))
		{
			return PYXValueTile::createFromFile(FileUtils::pathToString(filePath).c_str());			
		}
	}
	catch (PYXDataException&)
	{
		TRACE_INFO("Removing corrupted tile file '" << FileUtils::pathToString(filePath) << "'.");

		// The file was corrupted, delete the file
		boost::filesystem::remove(filePath);
	}
	catch (...)
	{
		// if anything at all goes wrong with reading from disk, then just say we didn't find it.
		TRACE_INFO("Unable to load a tile from '" << 
			FileUtils::pathToString(filePath) << "'. Possibly bad file format.");
	}
	return PYXPointer<PYXValueTile>();
}

/*!
Load the data source and coverage definitions from files.

\param	strDir	Directory where the files reside.
*/
bool PYXDefaultCoverage::loadDefinitions(const std::string& strDir)
{
	boost::filesystem::path dirPath = FileUtils::stringToPath(strDir);

	// load the data source definition
	{
		boost::filesystem::path defnPath = dirPath / kstrMetaDataFileName;
		if (!FileUtils::exists(defnPath))
		{
			TRACE_INFO("Missing definition file: " << FileUtils::pathToString(defnPath));
			return false;
		}
		std::ifstream in(FileUtils::pathToString(defnPath).c_str());
		in >> *(getDefinition().get());

		// update the write time so the file will not be deleted by cache cleaners
		FileUtils::touchFile(defnPath);
	}

	// load the data source values
	{
		boost::filesystem::path dataPath = dirPath / kstrDataFileName;
		if (!FileUtils::exists(dataPath))
		{
			TRACE_INFO("Missing values file: " << FileUtils::pathToString(dataPath));
			return false;
		}
		std::ifstream in(FileUtils::pathToString(dataPath).c_str());

		// check for valid header
		std::string strArg;
		in >> strArg;

		if (strArg != kstrScope)
		{
			TRACE_INFO("Invalid values file: " << FileUtils::pathToString(dataPath));
			return false;
		}

		// read version
		std::string strVersion;
		in >> strVersion;

		// read field count
		int nFieldCount;
		in >> nFieldCount;

		// read fields
		for (int nField = 0; nField < nFieldCount; ++nField)
		{
			PYXValue value;
			in >> value;

			setFieldValue(value, nField);
		}

		// update the write time so the file will not be deleted by cache cleaners
		FileUtils::touchFile(dataPath);
	}

	// load the coverage definition
	{
		boost::filesystem::path defnPath = dirPath / kstrCoverageMetaDataFileName;
		if (!FileUtils::exists(defnPath))
		{
			TRACE_INFO("Missing definition file: " << FileUtils::pathToString(defnPath));
			return false;
		}
		std::ifstream in(FileUtils::pathToString(defnPath).c_str());
		in >> *(m_spCovDefinition.get());

		// update the write time so the file will not be deleted by cache cleaners
		FileUtils::touchFile(defnPath);
	}

	return true;
}

/*!
Save the data source and coverage definitions to files.

\param	strDir	Directory where the files reside.
*/
void PYXDefaultCoverage::saveDefinitions(const std::string& strDir)
{
	boost::filesystem::path dirPath = FileUtils::stringToPath(strDir);

	if (!FileUtils::exists(dirPath))
	{
		boost::filesystem::create_directories(dirPath);
	}

	{
		assert(getDefinition() && "No data source definition.");

		// save the data source definition
		boost::filesystem::path defnPath = dirPath / kstrMetaDataFileName;
		std::ofstream out(FileUtils::pathToString(defnPath).c_str());

		out << *(getDefinition().get());
		out.close();
	}

	{
		// save the data source field values
		boost::filesystem::path dataPath = dirPath / kstrDataFileName;
		std::ofstream out(FileUtils::pathToString(dataPath).c_str());

		// write file header and version
		out << kstrScope << " " << "0.1" << std::endl;

		// write field count
		int nFieldCount = getDefinition()->getFieldCount();
		out << nFieldCount << std::endl;

		// write fields
		for (int nField = 0; nField < nFieldCount; ++nField)
		{
			out << getDefinition()->getFieldDefinition(nField) << std::endl;
		}
		out.close();
	}

	{
		assert(m_spCovDefinition && "No coverage definition.");

		// save the coverage definition
		boost::filesystem::path defnPath = dirPath / kstrCoverageMetaDataFileName;
		std::ofstream out(FileUtils::pathToString(defnPath).c_str());

		out << *(m_spCovDefinition.get());
		out.close();
	}
}

/*!
Add a file on disk into the cache and include it in the geometry of the cache.
This is called on opening a persisted cache form disk, and when a new tile has been
transferred into the cache from an external mechanism such as PyxNet.
*/
void PYXDefaultCoverage::addTileFile(const std::string& strFullFileName)
{
	// read in the tile
	std::ifstream in(strFullFileName.c_str(), std::ios_base::binary );
	PYXPointer<PYXValueTile> pDataTile = PYXValueTile::create(in);

	// might as well put this tile in our cache, since we've read it
	m_tileCache.add(pDataTile);

	// TODO:: we need to figure out if we are managing our own geometry, or
	// if the geometry has been supplied for us.

	//// add tile to our geometry
	//PYXTileCollection* pTileCollection = dynamic_cast<PYXTileCollection*>(m_spGeometry.get());
	//pTileCollection->addTile(rootIndex, m_nCellResolution);

	//// If this geometry is world-spanning, use a PYXGlobalGeometry
	//if (m_nTileResolution <= PYXMath::knMaxRelResolution)
	//{
	//	if (pTileCollection->getGeometryCount() == PYXIcosMath::getCellCount(m_nTileResolution))
	//	{
	//		m_spGeometry = PYXGlobalGeometry::create(m_nCellResolution);
	//	}
	//}
}

/*!
Load the geometry from the saved geometry file.

\param	strDir	Directory of data files.
*/
void PYXDefaultCoverage::loadGeometry(const std::string& strDir)
{
	boost::filesystem::path geometryPath = FileUtils::stringToPath(strDir);
	geometryPath /= kstrGeometryFileName;

	if (FileUtils::exists(geometryPath))
	{
		try
		{
			std::basic_ifstream</*unsigned*/ char> 
				in(FileUtils::pathToString(geometryPath).c_str(), std::ios_base::binary);
			in >> m_defaultTileDepth;
			m_spGeometry = PYXGeometrySerializer::deserialize(in);
			in.close();
			if (m_spGeometry.get() == 0)
			{
				throw "Invalid geometry in stream.";
			}
			m_nCellResolution = m_spGeometry->getCellResolution();

			// update the write time so the file will not be deleted by cache cleaners
			FileUtils::touchFile(geometryPath);
		}
		catch (...)
		{
			m_spGeometry = PYXTileCollection::create();
		}
	}
	else
	{
		// default to an empty tile collection
		m_spGeometry = PYXTileCollection::create();
	}
}

/*!
Save the geometry.

\param	strDir	Directory of data files.
*/
void PYXDefaultCoverage::saveGeometry(const std::string& strDir)
{
	boost::filesystem::path geometryPath = FileUtils::stringToPath(strDir);
	geometryPath /= kstrGeometryFileName;

	try
	{
		std::basic_ofstream</*unsigned*/ char> 
			out(FileUtils::pathToString(geometryPath).c_str(), std::ios_base::binary);
		out << m_defaultTileDepth;
		PYXGeometrySerializer::serialize(*(m_spGeometry.get()), out);
		out.close();
	}
	catch (...)
	{
		// something went wrong with our serializing.
		TRACE_ERROR ("Failed to save geometry." + strDir);
	}
}

/*!
Set the geometry to use for this coverage.  This is called so that the saved data 
set can have the same geometry as the input that it was saving to disk.

\param spGeometry The geometry to assign to the coverage.
*/
void PYXDefaultCoverage::setGeometry(PYXPointer<PYXGeometry> spGeometry)
{
	m_spGeometry = spGeometry;

	// write the new geometry out to disk if we are to set be persistent.
	if (m_bPersistent)
	{
		saveGeometry(getName());
	}
}

//! Purges any data stored for the given area.  (Null defaults to everything.)
void PYXDefaultCoverage::reset( PYXPointer<PYXGeometry> spInvalidatedArea)
{
	// free memory contents
	m_tileCache.detach(this);
	m_tileCache.clearAllTiles();
	m_tileCache.attach(this);

	boost::filesystem::path dirPath = FileUtils::stringToPath(getName());

	if (FileUtils::exists( dirPath) && FileUtils::isDirectory(dirPath))
	{
		for (boost::filesystem::directory_iterator d( dirPath), dirEnd;
			d != dirEnd;
			++d)
		{
			std::string ext = boost::filesystem::extension( *d);
			if ((ext.length() > 1) && ((ext[1] == 'd') || (ext[1] == 'D')))
			{
				boost::filesystem::remove( *d);
			}
		}
	}
}
