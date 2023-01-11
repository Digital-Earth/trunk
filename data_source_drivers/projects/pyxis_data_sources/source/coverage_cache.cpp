/******************************************************************************
coverage_cache.cpp

begin		: 2006-05-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE
#include "coverage_cache.h"

// local includes
#include "default_coverage.h"
#include "exceptions.h"

// pyxis includes
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/procs/geopacket_source.h"
#include "pyxis/utility/profile.h"
#include "pyxis/storage/pyxis_blob_provider.h"


#define Airborne_Imaging_Demo

// {83F35C37-5D0A-41c9-A937-F8C9C1E86850}
PYXCOM_DEFINE_CLSID(PYXCoverageCache,
					0x83f35c37, 0x5d0a, 0x41c9, 0xa9, 0x37, 0xf8, 0xc9, 0xc1, 0xe8, 0x68, 0x50);

PYXCOM_CLASS_INTERFACES(PYXCoverageCache, IProcess::iid,  ICoverage::iid, IFeatureCollection::iid, IFeature::iid, ICache::iid, IGeoPacketSource::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(PYXCoverageCache,
					"Coverage Cache",
					"Cache all data retrieval to increase performance.",
					"Utility",
					ICoverage::iid, IFeatureCollection::iid, IFeature::iid, ICache::iid, PYXCOM_IUnknown::iid, IGeoPacketSource::iid)
					IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Coverage to Cache", "The source coverage that is stored in local files.")
					IPROCESS_SPEC_END

					//! The name of the class
					const std::string PYXCoverageCache::kstrScope = "PYXCoverageCache";

//! The unit test class
Tester<PYXCoverageCache> gTester;

/*!
The unit test method for the class.
*/
void PYXCoverageCache::test()
{
	boost::intrusive_ptr<IProcess> spConstCoverageProcess(new ConstCoverage);
	boost::intrusive_ptr<ICoverage> spConstCoverage;
	spConstCoverageProcess->QueryInterface(ICoverage::iid, (void**) &spConstCoverage);

	//test valueTile upload and download from blobs
	{

		PYXPointer<PYXCoverageCache> spCacheProc(new PYXCoverageCache);
		// create a value tile with 4 channels, one of which is has array type
		const PYXIcosIndex myRootIndex("A-0");
		const int myCellResolution = 6;
		PYXTile myTile(myRootIndex, myCellResolution);
		std::vector<PYXValue::eType> vecTypes;
		vecTypes.push_back(PYXValue::knInt16);
		vecTypes.push_back(PYXValue::knFloat);
		vecTypes.push_back(PYXValue::knFloat);	// array type: float[3]
		vecTypes.push_back(PYXValue::knString);
		std::vector<int> vecCounts;
		vecCounts.push_back(1);
		vecCounts.push_back(1);
		vecCounts.push_back(3);							// array type: float[3]
		vecCounts.push_back(1);

		// create a table definition that matches the above.
		PYXPointer<PYXTableDefinition> spDefn = PYXTableDefinition::create();
		spDefn->addFieldDefinition("first value", PYXFieldDefinition::knContextNone, PYXValue::knInt16, 1);
		spDefn->addFieldDefinition("second value", PYXFieldDefinition::knContextNone, PYXValue::knFloat, 1);
		spDefn->addFieldDefinition("third value", PYXFieldDefinition::knContextNone, PYXValue::knFloat, 3);
		spDefn->addFieldDefinition("fourth value", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);

		// define some indices
		const PYXIcosIndex aValidCellIndex("A-01010");
		const PYXIcosIndex anotherValidCellIndex("A-01000");
		const PYXIcosIndex anIndexBiggerThanOneCell("A-010");
		const PYXIcosIndex anIndexSmallerThanOneCell("A-0101000");
		const PYXIcosIndex anIndexOutsideThisTile("B-001");

		// create some values
		PYXValue myInt16((int16_t)100);
		PYXValue myFloat((float)1000.0);
		float rgb[3]; rgb[0] = 100.; rgb[1] = 200.; rgb[2] = 300.;
		PYXValue myRGB(rgb,3);
		std::string myStr("yahöo");
		PYXValue myString(myStr);
		auto vt = PYXValueTile::create(myTile, vecTypes, vecCounts);

		// populate just one cell's data
		vt->setValue(aValidCellIndex, 0, myInt16);
		vt->setValue(aValidCellIndex, 1, myFloat);
		vt->setValue(aValidCellIndex, 2, myRGB);
		vt->setValue(aValidCellIndex, 3, myString);

		// retrieve values and check that they are correct (true value equivalence)
		PYXValue myOtherInt16((int16_t)100);
		PYXValue myOtherFloat((float)1000.0);
		PYXValue myOtherRGB(rgb, 3);
		PYXValue myOtherString(myStr);
		TEST_ASSERT(vt->getValue(aValidCellIndex, 0) == myOtherInt16);
		TEST_ASSERT(vt->getValue(aValidCellIndex, 1) == myOtherFloat);
		TEST_ASSERT(vt->getValue(aValidCellIndex, 2) == myOtherRGB);
		TEST_ASSERT(vt->getValue(aValidCellIndex, 3) == myOtherString);
		TEST_ASSERT(vt->getValue(aValidCellIndex, 3).getString() == myStr);
		TEST_ASSERT(vt->getValue(anotherValidCellIndex, 0).isNull());
		TEST_ASSERT(vt->getValue(anotherValidCellIndex, 1).isNull());
		TEST_ASSERT(vt->getValue(anotherValidCellIndex, 2).isNull());
		TEST_ASSERT(vt->getValue(anotherValidCellIndex, 3).isNull());


		spCacheProc->streamToBlob(vt);
		auto vt2 = spCacheProc->streamFromBlob(myTile);

		TEST_ASSERT(vt2->getValue(aValidCellIndex, 0) == myOtherInt16);
		TEST_ASSERT(vt2->getValue(aValidCellIndex, 1) == myOtherFloat);
		TEST_ASSERT(vt2->getValue(aValidCellIndex, 2) == myOtherRGB);
		TEST_ASSERT(vt2->getValue(aValidCellIndex, 3) == myOtherString);
		TEST_ASSERT(vt2->getValue(aValidCellIndex, 3).getString() == myStr);
		TEST_ASSERT(vt2->getValue(anotherValidCellIndex, 0).isNull());
		TEST_ASSERT(vt2->getValue(anotherValidCellIndex, 1).isNull());
		TEST_ASSERT(vt2->getValue(anotherValidCellIndex, 2).isNull());
		TEST_ASSERT(vt2->getValue(anotherValidCellIndex, 3).isNull());
	}

	// simple attribute-value tests
	{
		boost::intrusive_ptr<IProcess> spCacheProc(new PYXCoverageCache);

		boost::intrusive_ptr<ICoverage> spCov;
		spCacheProc->QueryInterface(ICoverage::iid, (void**) &spCov);
		TEST_ASSERT(spCov.get() != 0);

		boost::intrusive_ptr<ICache> spCache;
		spCacheProc->QueryInterface(ICache::iid, (void**) &spCache);
		TEST_ASSERT(spCache.get() != 0);

		spCache->setCachePersistence(false);

		std::string strCacheDir = FileUtils::pathToString(AppServices::makeTempDir());
		spCache->setCacheDir(strCacheDir);
		TEST_ASSERT(strCacheDir == spCache->getCacheDir());
		spCache->setCacheCellResolution(20);
		TEST_ASSERT(20 == spCache->getCacheCellResolution());
		TEST_ASSERT(PYXTile::knDefaultTileDepth == spCache->getCacheTileDepth());
		spCache->setCacheTileDepth(5);
		TEST_ASSERT(5 == spCache->getCacheTileDepth());
	}

	// copying of attributes from input
	{
		boost::intrusive_ptr<IProcess> spCacheProc(new PYXCoverageCache);

		boost::intrusive_ptr<ICoverage> spCov;
		spCacheProc->QueryInterface(ICoverage::iid, (void**) &spCov);
		TEST_ASSERT(spCov.get() != 0);

		boost::intrusive_ptr<ICache> spCache;
		spCacheProc->QueryInterface(ICache::iid, (void**) &spCache);
		TEST_ASSERT(spCache.get() != 0);

		spCache->setCachePersistence(false);

		spConstCoverage->addField("0", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8);
		std::vector<PYXValue> vecAccum;
		vecAccum.push_back(PYXValue::create(PYXValue::knUInt8, 0, 3, 0));
		spConstCoverage->setFieldValues(vecAccum);
		spCacheProc->getParameter(0)->addValue(spConstCoverageProcess);
		spCacheProc->initProc(true);

		TEST_ASSERT(!(spCache->getCacheDir().empty()));

		TEST_ASSERT(spCov->getGeometry()->getCellResolution() ==
			spConstCoverage->getGeometry()->getCellResolution());
		PYXPointer<const PYXGeometry> spInputGeometry = spCov->getGeometry()->clone();
		PYXPointer<const PYXGeometry> spCacheGeometry = spConstCoverage->getGeometry();

		// TODO: replace following 5 tests with a single PYXGeometry equality test
		// when one has been implemented
		TEST_ASSERT(spInputGeometry->isEmpty() == spCacheGeometry->isEmpty());
		TEST_ASSERT(spInputGeometry->isCollection() == spCacheGeometry->isCollection());
		//TEST_ASSERT(spInputGeometry->getType() == spCacheGeometry->getType());
		TEST_ASSERT(	spInputGeometry->getCellResolution() ==
			spCacheGeometry->getCellResolution()	);
		TEST_ASSERT(spInputGeometry->intersects(*spCacheGeometry.get()));
	}

	/* TODO[kabiraman]: Investigate the failure in this unit test, see ticket #2052.
	// verify transparency of caching
	{
	boost::intrusive_ptr<IProcess> spCacheProc(new PYXCoverageCache);

	boost::intrusive_ptr<ICoverage> spCov;
	spCacheProc->QueryInterface(ICoverage::iid, (void**) &spCov);
	TEST_ASSERT(spCov.get() != 0);

	boost::intrusive_ptr<ICache> spCache;
	spCacheProc->QueryInterface(ICache::iid, (void**) &spCache);
	TEST_ASSERT(spCache.get() != 0);

	spCache->setCachePersistence(false);

	const unsigned char myRGB[3] = {100, 150, 200};
	PYXValue myConstantValue(myRGB, 3);
	const unsigned char myRGB2[3] = {10, 20, 30};
	PYXValue stuffedValue(myRGB2, 3);

	// Set the output for the const coverage.
	std::map<std::string, std::string> attribs = spConstCoverageProcess->getAttributes();
	std::ostringstream ost;
	ost << myConstantValue;
	attribs["value"] = ost.str();
	ost.str("");
	ost << PYXFieldDefinition::knContextRGB;
	attribs["context"] = ost.str();
	spConstCoverageProcess->setAttributes(attribs);

	// set the const coverage as the input to the cache
	PYXPointer<ParameterSpec> spSpec = ParameterSpec::create(
	spConstCoverage->iid, 1, 1, "Cached Coverage", "");
	PYXPointer<Parameter> param = Parameter::create(spSpec);
	param->addValue(spConstCoverageProcess);

	std::vector<PYXPointer<Parameter> > vecParams;
	vecParams.push_back(param);
	spCacheProc->setParameters(vecParams);

	spCache->setCacheCellResolution(6);
	spCacheProc->initProc(true);

	int nCellCount = 0;
	for (	PYXPointer<PYXIterator> spIt(spCov->getGeometry()->getIterator());
	!spIt->end();
	spIt->next()	)
	{
	const PYXIcosIndex& index = spIt->getIndex();
	if (nCellCount == 100)
	{
	// just once, stuff a value into the cache
	spCov->setCoverageValue(stuffedValue, index, 0);
	PYXValue cacheValue = spCov->getCoverageValue(index, 0);
	TEST_ASSERT(cacheValue == stuffedValue
	&& "The value from the cache was not the value that we put into it.");
	}
	else
	{
	// other caching should be transparent
	PYXValue inputValue = spConstCoverage->getCoverageValue(index, 0);
	PYXValue cacheValue = spCov->getCoverageValue(index, 0);
	TEST_ASSERT(cacheValue == inputValue
	&& "The value from the cache was not the value that went into it.");
	TEST_ASSERT(cacheValue == myConstantValue
	&& "The value from the cache should be the value we set for the const coverage.");
	}
	nCellCount++;
	}
	TEST_ASSERT(nCellCount == PYXIcosMath::getCellCount(6)
	&& "The cache geometry should have contained all cells for resolution 6.");
	}*/

	// verify multi-channel data can be stored in the cache
	{
		// creat a constant coverage
		boost::intrusive_ptr<IProcess> spConstProc(new ConstCoverage);

		// The functionality is not exposed through PyxCom to set the constant coverage up
		// to be multi-field, so we have to go directly to the C++ interface.
		ConstCoverage* pConstCoverage = dynamic_cast<ConstCoverage*>(spConstProc.get());
		assert(pConstCoverage != 0);

		// set the contant coverage to have 3 channels of data
		pConstCoverage->setFieldCount(3);
		const unsigned char RGB0[3] = {100, 150, 200};
		PYXValue aPYXValueRGB0(RGB0, 3);
		pConstCoverage->setReturnValue(aPYXValueRGB0, PYXFieldDefinition::knContextRGB,0);
		const unsigned char RGB1[3] = {50, 144, 192};
		PYXValue aPYXValueRGB1(RGB1, 3);
		pConstCoverage->setReturnValue(aPYXValueRGB1, PYXFieldDefinition::knContextRGB,1);
		const unsigned char RGB2[3] = {17, 44, 21};
		PYXValue aPYXValueRGB2(RGB2, 3);
		pConstCoverage->setReturnValue(aPYXValueRGB2, PYXFieldDefinition::knContextRGB,2);

		// create the cache
		boost::intrusive_ptr<IProcess> spCacheProc(new PYXCoverageCache);

		boost::intrusive_ptr<ICoverage> spCov;
		spCacheProc->QueryInterface(ICoverage::iid, (void**) &spCov);
		TEST_ASSERT(spCov.get() != 0);

		boost::intrusive_ptr<ICache> spCache;
		spCacheProc->QueryInterface(ICache::iid, (void**) &spCache);
		TEST_ASSERT(spCache.get() != 0);

		spCache->setCachePersistence(false);

		// set the const coverage as the input to the cache
		spCacheProc->getParameter(0)->addValue(spConstProc);
		spCacheProc->initProc(true);

		// get values and make sure they are good.
		PYXIcosIndex index1("A-000");
		TEST_ASSERT(spCov->getCoverageValue(index1, 0) == aPYXValueRGB0);
		TEST_ASSERT(spCov->getCoverageValue(index1, 1) == aPYXValueRGB1);
		TEST_ASSERT(spCov->getCoverageValue(index1, 2) == aPYXValueRGB2);

		// do it a second time so that they are retrieved from the cache
		TEST_ASSERT(spCov->getCoverageValue(index1, 0) == aPYXValueRGB0);
		TEST_ASSERT(spCov->getCoverageValue(index1, 1) == aPYXValueRGB1);
		TEST_ASSERT(spCov->getCoverageValue(index1, 2) == aPYXValueRGB2);

		// try it all again with a second index
		PYXIcosIndex index2("B-00000403");
		TEST_ASSERT(spCov->getCoverageValue(index2, 0) == aPYXValueRGB0);
		TEST_ASSERT(spCov->getCoverageValue(index2, 1) == aPYXValueRGB1);
		TEST_ASSERT(spCov->getCoverageValue(index2, 2) == aPYXValueRGB2);

		// do it a second time so that they are retrieved from the cache
		TEST_ASSERT(spCov->getCoverageValue(index2, 0) == aPYXValueRGB0);
		TEST_ASSERT(spCov->getCoverageValue(index2, 1) == aPYXValueRGB1);
		TEST_ASSERT(spCov->getCoverageValue(index2, 2) == aPYXValueRGB2);
	}
}

/*!
Default Constructor.
*/
PYXCoverageCache::PYXCoverageCache() :
	m_bPersistent(true),
	m_nCellResolution(-1),
	m_nTileDepth(PYXTile::knDefaultTileDepth),
	m_needATileNotifier("Coverage Cache needs a tile notifier"),
	m_cacheChangedNotifier("Coverage Cache changed notifier"),
	m_bIsGreedy(false),
	m_tileHasValuesCache(500)
{
}

const void PYXCoverageCache::initCacheDir() const
{
	// if we are persistent, then use a cache directory that will survive until the next run.
	if (m_bPersistent)
	{
		boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");

		const ProcessIdentityCache cache(cacheDir);
		const boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);
		if (pathCache.empty())
		{
			PYXTHROW(PYXException, "Could not create cache directory.");
		}
		m_strCacheDir = FileUtils::pathToString(pathCache);
	}
	else
	{
		m_strCacheDir = FileUtils::pathToString(AppServices::makeTempDir());
	}
}

/*!
Get the cache data source.  If none exists, a new empty one will be created.

\return Shared pointer to data source (always a PYXDefaultCoverage).
*/
PYXPointer<PYXDefaultCoverage> PYXCoverageCache::createCache() const
{
	// TODO: we must be able to make second default coverages to support new inputs being applied.

	if (m_spCache)
	{
		return m_spCache;
	}
	else
	{
		if (m_strCacheDir.empty())
		{
			initCacheDir();
		}

		m_spCache = PYXDefaultCoverage::create();

		m_spCache->setCachePersistence(m_bPersistent);

		if (getInput())
		{
			assert(m_nCellResolution != -1 && "Can't make DS until resolution defined");

			// if we have an input use that to open that cache
			m_spCache->setCoverageDefinition(getInput()->getCoverageDefinition()->clone());
			m_spCache->openReadWrite(m_strCacheDir,
				*(getInput()->getDefinition().get()),
				getInput()->getFieldValues(),
				*(getInput()->getCoverageDefinition().get()),
				m_nCellResolution,
				m_nTileDepth);
		}
		else
		{
			// if there is no input then open the cache that is presumably already on disk.
			m_spCache->openReadOnly(m_strCacheDir);

			// if we didn't find the resolution from the files on disk, then set it here.
			if (m_spCache->getCellResolution() == -1)
			{
				m_spCache->setCellResolution(m_nCellResolution);
			}
			else
			{
				// otherwise grab it from the internal cache.
				m_nCellResolution = m_spCache->getCellResolution();
			}
		}
	}
	return m_spCache;
}

/*!
Set the input coverage.

\param spInput	Shared pointer to input coverage (ownership shared with caller).
*/
void PYXCoverageCache::setInput(boost::intrusive_ptr<ICoverage> spInput)
{
	assert(spInput && "Invalid Input.");

	m_spGeom = PYXEmptyGeometry::create();

	// Get data resolution
	if (m_nCellResolution == -1)
	{
		m_nCellResolution = spInput->getGeometry()->getCellResolution();
		if (m_nCellResolution < PYXIcosIndex::knMinSubRes)
		{
			// problem: input data set doesn't have a valid resolution
			TRACE_ERROR("PYXCoverageCache: input data set doesn't have valid resolution.");
			TRACE_ERROR("	You must set one using PYXCoverageCache::setCellResolution().");
			return;
		}
	}
	if (m_spCov.get() != spInput.get())
	{
		m_spCache = 0;
		m_strCacheDir.clear();
		m_spCov = spInput;
		// Create cache now
		createCache();
	}

	// copy input geometry as default, and adjust cell resolution
	m_spGeom = spInput->getGeometry()->clone();
	m_spGeom->setCellResolution(m_nCellResolution);
	getCache()->setGeometry(m_spGeom);
}

boost::intrusive_ptr<const ICoverage> PYXCoverageCache::getInput() const
{
	return m_spCov;
}

/*!
Get (and cache) a single field value within a single PYXIS cell.

\param	index			PYXIS cell index.
\param	nFieldIndex		Field index within the cell.

\return	The field value.
*/
PYXValue PYXCoverageCache::getCoverageValue(	const PYXIcosIndex &index,
											int nFieldIndex ) const
{
	bool bInitialized;

	if (m_spGeom->intersects((PYXCell(index)),true))
	{
		// first, try to get the value from the cache
		PYXValue val;

		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
			val = getCache()->getCoverageValue(index, nFieldIndex, &bInitialized);
		}

		if (bInitialized)
		{
			return val;
		}

		// if value not yet cached, get it (aka - Download or get from Input)
		PYXPointer<PYXValueTile> spValueTile = getCoverageTile(getCache()->getDefaultTile(index));

		if (spValueTile)
		{
			return spValueTile->getValue(index, nFieldIndex);
		}
	}

	// No data available.  Return a null.
	return PYXValue();
}

/*!
Get the coverage values for the specified PYXIS tile.

\param	index		The PYXIS index.
\param	nRes		The resolution of the tile.
\param	nFieldIndex	The field index.

\return	The values. May return null if no values.
*/
PYXPointer<PYXValueTile> STDMETHODCALLTYPE PYXCoverageCache::getFieldTile(	const PYXIcosIndex& index,
																		  int nRes,
																		  int nFieldIndex) const
{
	PYXPointer<PYXValueTile> tile = getCoverageTile(PYXTile(index, nRes));
	if (!tile)
	{
		return tile;
	}
	if (nFieldIndex >= tile->getNumberOfDataChannels())
	{
		PYXTHROW(PYXException,"nFieldIndex is out of range. requesting #" << nFieldIndex << " field, but tile contains only "<< tile->getNumberOfDataChannels() << "fields");
	}
	if (tile->getNumberOfDataChannels() == 1)
	{
		return tile;
	}
	return tile->cloneFieldTile(nFieldIndex);
}

PYXCost PYXCoverageCache::getFieldTileCost(const PYXIcosIndex& index,
										   int nRes,
										   int nFieldIndex) const
{
	WARN_IF_FUNCTION_TOOK_MORE_THAN_SEC(0.5);
	return getTileCost(PYXTile(index, nRes));
}

PYXCost PYXCoverageCache::getTileCost(const PYXTile& tile) const
{
	// Try the tile has value cache first,
	{
		boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
		//if this cache has some information - then results is immidate
		if (m_tileHasValuesCache.exists(tile))
		{
			return PYXCost::knImmediateCost;
		}
	}

	// Try to get the tile from the cache.
	PYXPointer<PYXValueTile> spTile = getCache()->getTileFromMemory(tile);

	if (!spTile)
	{
		if (!m_spGeom->intersects(tile))
		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
			m_tileHasValuesCache[tile] = false;
			return PYXCost::knImmediateCost;
		}

		//try to recover the tile from disk if cache is persistent
		if (m_bPersistent)
		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
			spTile = getCache()->getCoverageTile(tile);
		}
	}

	if (spTile && spTile->isComplete())
	{
		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
			m_tileHasValuesCache[tile] = true;
		}
		return PYXCost::knImmediateCost;
	}

	if (cacheHasDataSupply())
	{
		//getInputCost + write the tile to disk
		return getInput()->getTileCost(tile) + PYXCost::knImmediateCost;
	}
	else
	{
		return PYXCost::knNetworkCost;
	}
}

/*!
Get coverage values for an entire tile (all fields).

\param	tile	Defines requested tile (root index, depth).

\return Shared pointer to value tile.
*/
PYXPointer<PYXValueTile>
	PYXCoverageCache::getCoverageTile(const PYXTile& tile) const
{
	PYXPointer<PYXValueTile> spTile;

	//Check has values cache first...
	{
		boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);

		if (m_tileHasValuesCache.exists(tile))
		{
			notifyProcessing(ProcessProcessingEvent::Fetching);

			bool hasValue = m_tileHasValuesCache[tile];

			if (hasValue)
			{
				WARN_IF_FUNCTION_TOOK_MORE_THAN_SEC(0.5);
				// Try to get the tile from the memory cache - quick and dirty, if it works - we have a tile :D
				spTile = getCache()->getTileFromMemory(tile);
			}
			else
			{
				return PYXPointer<PYXValueTile>();
			}
		}
	}

	//We failed to receive a from memory or that this tile has not be requested before
	if (!spTile)
	{
		WARN_IF_FUNCTION_TOOK_MORE_THAN_SEC(0.5);
		if (!m_spGeom->intersects(tile))
		{
			{
				//make that this tile has not value for next time query
				boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
				m_tileHasValuesCache[tile] = false;
			}
			notifyProcessing(ProcessProcessingEvent::Fetching);
			return PYXPointer<PYXValueTile>();
		}

		//try to recover the tile from disk
		if (m_bPersistent)
		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
			spTile = getCache()->getCoverageTile(tile);

			if (spTile)
			{
				m_tileHasValuesCache[tile] = true;
			}
		}
	}

	if (spTile)
	{
		// We got the tile from the cache, but check it for completeness,
		// as it may be a tile with only some values initialized.
		if (!spTile->isComplete() && cacheHasDataSupply())
		{
			PYXPointer<PYXValueTile> spInputTile = getInput()->getCoverageTile(tile);

			if (spInputTile)
			{
				WARN_IF_FUNCTION_TOOK_MORE_THAN_SEC(0.5);
				//updateling the spTile - lock  everything until we done
				boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);

				int nChannelCount = spTile->getNumberOfDataChannels();
				int nCellCount = spTile->getNumberOfCells();

				// for every channel of data
				for (int channelNumber = 0; channelNumber < nChannelCount; ++channelNumber)
				{
					// For every index in the tile.
					for (int nIndexOffset = 0; nIndexOffset < nCellCount; ++nIndexOffset)
					{
						bool initialized = false;
						PYXValue notUsed = spTile->getValue(nIndexOffset, channelNumber, &initialized);
						if (!initialized)
						{
							spTile->setValue(nIndexOffset, channelNumber,
								spInputTile->getValue(nIndexOffset, channelNumber));
						}
					}
				}
				spTile->setIsComplete(true);
			}
		}
		notifyProcessing(ProcessProcessingEvent::Fetching);
		return spTile;
	}

	// Tile doesn't exist in the cache.
	if (cacheHasDataSupply())
	{
		notifyProcessing(ProcessProcessingEvent::Processing);
		spTile = getInput()->getCoverageTile(tile);

		if (spTile)
		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);

			// mark it as a full tile.
			spTile->setIsComplete(true);

			//Got the tile from our input. Add to cache and return.
			getCache()->setCoverageTile(spTile);

			m_tileHasValuesCache[tile] = true;
			return spTile;
		}
		else
		{
			//the input has not return any value...
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
			m_tileHasValuesCache[tile] = false;
		}
	}
	else
	{
		//try to fetch the tile from blob storage
		spTile = streamFromBlob(tile);
		if (spTile)
		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);

			// mark it as a full tile.
			spTile->setIsComplete(true);

			//Got the tile from Blob Storage. Add to cache and return.
			getCache()->setCoverageTile(spTile);

			m_tileHasValuesCache[tile] = true;
			return spTile;
		}
	
		notifyProcessing(ProcessProcessingEvent::Downloading);
		// if no input, fire off the NeedATile Notifier and try and get the tile again.
		boost::intrusive_ptr<ICache> spCache;
		const_cast<PYXCoverageCache*>(this)->QueryInterface(ICache::iid, (void**) &spCache);
		PYXPointer<PYXTile> spNotifyTile = PYXTile::create(tile);
		PYXPointer<CacheNeedsTileEvent> ev = CacheNeedsTileEvent::create(spCache, spNotifyTile);
		m_needATileNotifier.notify(ev);

		if(ev->getDownloadFailed())
		{
			notifyProcessing(ProcessProcessingEvent::Error);
			throw TileUnavailableException("tile " + tile.getRootIndex().toString() + " (depth=" + StringUtils::toString(tile.getDepth()) + ") is currently unavailable, but should be available later.");
		}

		{
			boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);

			// Try again to get the tile from the cache.
			PYXPointer<PYXValueTile> spTile = getCache()->getCoverageTile(tile);
			if (spTile)
			{
				m_tileHasValuesCache[tile] = true;
				// Tile exists in the cache -- so the Notify must have caused some action.
				return spTile;
			}
		}
	}

	//Something is wrong, return a null ptr.
	return 0;
}


PYXPointer <PYXValueTile> PYXCoverageCache::streamFromBlob ( const PYXTile& tile) const
{
	auto key = "Version2:" +procRefToStr(ProcRef(getProcID(), getProcVersion())) + "-Depth:" +  StringUtils::toString(tile.getDepth()) + "-Index:" + tile.getRootIndex().toString();
	PyxisBlobProvider bprovider;
	std::stringstream downloaded;
	if(bprovider.getBlob(key, downloaded))
	{
		PYXPointer <PYXValueTile> result = PYXValueTile::create(downloaded); 
		return result;
	}
	return 0;
}


bool PYXCoverageCache::streamToBlob (PYXPointer <PYXValueTile>  spValueTile) const
{
	auto tile = spValueTile->getTile();
	auto key = "Version2:" +procRefToStr(ProcRef(getProcID(), getProcVersion())) + "-Depth:" +  StringUtils::toString(tile.getDepth()) + "-Index:" + tile.getRootIndex().toString();
	PyxisBlobProvider blobProvider;
	std::stringstream toUpload;
	spValueTile->serialize(toUpload);
	auto result = blobProvider.addBlob(key, toUpload);
	return result;
}

/*!
Proactively set a single field value within a single PYXIS cell in the cache.

\param nValue			The value to set.
\param index			PYXIS cell index.
\param nFieldIndex		Field index within the cell.

*/
void PYXCoverageCache::setCoverageValue(const PYXValue& nValue, const PYXIcosIndex& index, int nFieldIndex)
{
	boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
	getCache()->setCoverageValue(nValue,index,nFieldIndex);
}

/*!
Set coverage values for an entire tile (all fields).

\param	spValueTile	Shared pointer to a PYXValueTile containing value data.
*/
void PYXCoverageCache::setCoverageTile(PYXPointer<PYXValueTile> spValueTile)
{
	boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);
	getCache()->setCoverageTile(spValueTile);
}

bool PYXCoverageCache::forceCoverageTile(PYXTile tile)
{
	if (cacheHasDataSupply())
	{
		PYXPointer<PYXValueTile> valueTile = getCoverageTile(tile);
		if(!valueTile && m_spGeom->intersects(tile))
		{
			valueTile = PYXValueTile::create(tile.getRootIndex(),tile.getCellResolution(), getCoverageDefinition());
		}
		if(valueTile)
		{
			streamToBlob(valueTile);
		}
		persistAllTiles();
		return true;
	}
	return false;
}

IProcess::eInitStatus PYXCoverageCache::initImpl()
{
	m_strID = "Coverage Cache: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	boost::intrusive_ptr<ICoverage> spCov;
	if (cacheInputIsOK())
	{
		spCov = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
		assert(spCov);

		// we have an input that is a coverage so all is good.
		setInput(spCov);
	}
	else
	{
		m_spCov.reset();
		m_spGeom = PYXEmptyGeometry::create();

		boost::intrusive_ptr<IProcess> spErrorProc = PipeUtils::findFirstError(this);

		assert(spErrorProc);
		assert(spErrorProc->getInitError());

		// Missing SRS, no need to look for data over PyxNet.  After the SRS
		// is added, if there is still local data missing, this code will be
		// ignored and data will be looked for over PyxNet.
		// TODO: Replace hard-coded GUID with a constant, and do the same for other hard-coded GUIDs.
		if (spErrorProc->getInitError()->getErrorID() == "{68F0FC89-2D83-439C-BD4E-72A8A9CCDCED}")
		{
			return knFailedToInit;
		}

		// Missing or empty geometry, no need to look for data over PyxNet.
		// TODO: Replace hard-coded GUID with a constant, and do the same for other hard-coded GUIDs.
		if (spErrorProc->getInitError()->getErrorID() == "{62878998-D2B8-4F98-BA48-7ECAD2B523F0}")
		{
			return knFailedToInit;
		}

		// Missing world file, no need to look for data over PyxNet.
		// TODO: Replace hard-coded GUID with a constant, and do the same for other hard-coded GUIDs.
		if (spErrorProc->getInitError()->getErrorID() == "{ACA50CE2-E822-49D2-AFE1-1AE5BA7966E9}")
		{
			return knFailedToInit;
		}

		// notify if we are initialized and have no data supply so that
		// we can get the cache initialized from PyxNet.
		boost::intrusive_ptr<ICache> spCache;
		this->QueryInterface(ICache::iid, (void**) &spCache);
		boost::intrusive_ptr<IProcess> spProcess;
		this->QueryInterface(IProcess::iid, (void**) &spProcess);

		PYXPointer<CacheWithProcessEvent> ev =
			CacheWithProcessEvent::create(spCache, spProcess);

		// Note: Always release a lock before raising an event.
		m_procMutex.unlock();
		CacheManager::getCacheCreatedNotifier().notify(ev);
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		if (!ev->getHandled())
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Cache has no source of data.");
			return knFailedToInit;
		}

		// hook up to the cache that may have been created for us.
		createCache();

		// get the geometry of the cache that was transfered over PyxNet.
		m_spGeom = getCache()->getGeometry();
		m_nCellResolution =  m_spGeom->getCellResolution();
	}

	m_tileHasValuesCache.clear();

	m_cacheHasDataSupplyResult = getInput() &&  cacheInputIsOK();

	return knInitialized;
}

/*!
Overrides Process::initProc() to ignore errors on inputs until this process is initialized.
*/
IProcess::eInitStatus PYXCoverageCache::initProc(bool bRecursive)
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	if (m_initState == knInitializing)
	{
		// we are in the middle of initializing.
		return knInitializing;
	}

	// unless we are already initialized, then set our state to initializing.
	if (m_initState != knInitialized)
	{
		m_initState = knInitializing;
	}

	// clear out the current error state
	m_spInitError = boost::intrusive_ptr<IProcessInitError>();

	// process all of the children first
	if (bRecursive)
	{
		std::vector<PYXPointer<Parameter>> parameters = static_cast<std::vector<PYXPointer<Parameter>>> (getParameters());
		// cycle through each of the children
		std::vector<PYXPointer<Parameter>>::iterator it =
			parameters.begin();
		for (; it != parameters.end(); ++it)
		{
			for (int n = 0; n < (*it)->getValueCount(); ++n)
			{
				assert((*it)->getValue(n));
				(*it)->getValue(n)->initProc(true);
			}
		}
	}

	// if the process is already initialized, return
	if (m_initState != knInitialized)
	{
		if (!verifySpec(this))
		{
			m_initState = knDoesNotMeetSpec;
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new ProcSpecFailure());
			return m_initState;
		}

		try
		{
			// perform the actual initialization and examine the state
			m_initState = initImpl();
			if (m_initState == knFailedToInit)
			{
				if (!m_spInitError)
				{
					TRACE_ERROR(
						"A process failed to initialize but no error state was set, creating generic error for " <<
						getProcName());
					m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
					m_spInitError->setError(
						"Generic process initialization error.");
				}
			}
			else if (m_initState != knInitialized)
			{
				TRACE_ERROR("Process initialization should only return pass or fail, Policy Breach!");
				assert(false && "Process initialization should only return pass or fail, Policy Breach!");
			}
		}
		catch (PYXException& e)
		{
			TRACE_ERROR("A PYXIS exception was thrown during process initialization. Processes should never throw during init.");
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError(
				"A PYXIS error occurred during process initialization: \n" + e.getFullErrorString());
			m_initState = knFailedToInit;
		}
		catch (...)
		{
			TRACE_ERROR("A generic exception occurred during process initialization for: " << getProcName());
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError(
				"A generic error occurred during process initialization");
			m_initState = knFailedToInit;
		}
		assert(	(m_initState == knInitialized || m_spInitError) &&
			"Policy Breach! invalid error and state combination.");
	}
	return m_initState;
}

bool PYXCoverageCache::cacheInputIsOK() const
{
	boost::intrusive_ptr<IProcess> spProc = getParameter(0)->getValue(0);
	if(spProc->getInitState() == IProcess::knNeedsInit)
	{
		spProc->initProc(true);
	}
	return (spProc->getInitState() == IProcess::knInitialized);
}

bool PYXCoverageCache::cacheHasDataSupply() const
{
	return m_cacheHasDataSupplyResult; // (getInput() &&  cacheInputIsOK()); should never change after initImpl has been called...
}

std::map<std::string, std::string> PYXCoverageCache::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	PYXValue value(m_bIsGreedy);
	std::ostringstream ost;
	ost << value;
	mapAttr["greedy"] = ost.str();
	return mapAttr;
}

void PYXCoverageCache::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;

	// extract the value
	it = mapAttr.find("greedy");
	if (it != mapAttr.end())
	{
		PYXValue value;
		StringUtils::fromString(it->second, &value);
		m_bIsGreedy = value.getBool();
	}
}

std::string PYXCoverageCache::getAttributeSchema() const
{
	return "";
	//return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
	//	"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
	//		"elementFormDefault=\"qualified\" "
	//		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
	//		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
	//		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
	//	">"
	//	  "<xs:element name=\"CoverageCache\">"
	//	  "<xs:complexType>"
	//		"<xs:sequence>"
	//		 // "<xs:element name=\"greedy\" type=\"xs:boolean\">"
	//			//"<xs:annotation>"
	//			//  "<xs:appinfo>"
	//			//	"<friendlyName>Greedy</friendlyName>"
	//			//	"<description></description>"
	//			//  "</xs:appinfo>"
	//			//"</xs:annotation>"
	//		 // "</xs:element>"
	//		"</xs:sequence>"
	//	  "</xs:complexType>"
	//	  "</xs:element>"
	//	"</xs:schema>";
}

//! Handler for any input changing its data.
void PYXCoverageCache::handleInputDataChanged( PYXPointer<NotifierEvent> eventData)
{
	PYXPointer<ProcessDataChangedEvent> processDataChangedEvent =
		boost::dynamic_pointer_cast<ProcessDataChangedEvent>( eventData);

	if (processDataChangedEvent->getDataChangeTrigger() == ProcessDataChangedEvent::knInputDataChange)
	{
		boost::recursive_mutex::scoped_lock lock(m_getCoverageMutex);

		getCache()->reset(processDataChangedEvent->getGeometry());

		//Helper class to clear the needed tiles from the tile has values cache

		struct CheckIntersection
		{
			PYXPointer<PYXGeometry> m_geometry;

			bool operator()(const PYXTile & tile,const bool & hasValue)
			{
				return (tile.intersects(*m_geometry));
			}

			CheckIntersection(const PYXPointer<PYXGeometry> geometry) : m_geometry(geometry)
			{
			}
		};

		m_tileHasValuesCache.eraseIf(CheckIntersection(processDataChangedEvent->getGeometry()));
	}

	// Note: The base class has already passed the event on to our own observers.
	// Note: That is not true :(
	onDataChanged( processDataChangedEvent->getGeometry(), processDataChangedEvent->getDataChangeTrigger());
}

void PYXCoverageCache::geometryHint(PYXPointer<PYXGeometry> spGeom)
{
	if (cacheHasDataSupply())
	{
		m_spCov->geometryHint(spGeom);
	}
}

void PYXCoverageCache::endGeometryHint(PYXPointer<PYXGeometry> spGeom)
{
	if (cacheHasDataSupply())
	{
		m_spCov->endGeometryHint(spGeom);
	}
}