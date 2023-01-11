/******************************************************************************
blue_pyxis.cpp

begin		: 2007-03-04
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_PYXIS_COVERAGES_SOURCE
#include "blue_pyxis.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/procs/path.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {5FEC5F05-EC4F-4b2b-A620-30C8768D9520}
PYXCOM_DEFINE_CLSID(BluePyxis, 
0x5fec5f05, 0xec4f, 0x4b2b, 0xa6, 0x20, 0x30, 0xc8, 0x76, 0x8d, 0x95, 0x20);
PYXCOM_CLASS_INTERFACES(BluePyxis, IProcess::iid, ICoverage::iid, IFeature::iid, IUnknown::iid);

IPROCESS_SPEC_BEGIN(BluePyxis, "Blue Pyxis", "A compressed PYXIS format data source", "Drop",
					ICoverage::iid, IFeature::iid, IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IPath::iid, 1, 1, "File Directory", "The directory that contains the data files");
IPROCESS_SPEC_END

namespace
{

std::vector<PYXValue::eType> vecTypes(1, PYXValue::knUInt8);
std::vector<int> vecCounts(1, 3);
std::vector<PYXValue::eType> vecTypesElev(1, PYXValue::knFloat);
std::vector<int> vecCountsElev(1, 1);

}

BluePyxis::BluePyxis() :
	m_nRes(-1)
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE BluePyxis::getAttributes() const
{
	return m_mapAttr;
}

void STDMETHODCALLTYPE BluePyxis::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	m_mapAttr = mapAttr;
}

IProcess::eInitStatus BluePyxis::initImpl()
{
	m_strID = "Blue PYXIS: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Verify that we have input data.
	if (!(getParameter(0)->getValueCount() && getParameter(0)->getValue(0) &&
		getParameter(0)->getValue(0)->getOutput()))
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not retrieve the data path.");
		return knFailedToInit;
	}

	// Get the data path
	boost::intrusive_ptr<IPath> spPath;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IPath::iid, (void**) &spPath);
	if (!spPath)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("The data path is not a valid process");
		return knFailedToInit;
	}
	if (!spPath->isDirectory())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("The path must be a directory");
		return knFailedToInit;
	}
	m_path = boost::filesystem::path(spPath->getPath(), boost::filesystem::native);

#if 0
	// Determine min and max res
	m_nMinRes = -1;
	m_nMaxRes = -1;
	for (int nRes = PYXIcosIndex::knMinSubRes; nRes != PYXMath::knMaxAbsResolution; ++nRes)
	{
		// TODO should use boost filesystem here
		std::string strFileName = (m_path / (toString(nRes) + ".bin")).native_file_string();
		if (m_in.is_open())
		{
			m_in.close();
		}

		m_in.open(strFileName.c_str(), std::ofstream::binary);
		if (m_in.good())
		{
			if (m_nMinRes == -1)
			{
				m_nMinRes = nRes;
			}
			m_nMaxRes = nRes;
		}
		else if (m_nMaxRes != -1)
		{
			m_in.clear();
			if (m_nMaxRes != -1)
			{
				break;
			}
		}
	}

	TRACE_INFO("BluePyxis res " << m_nMinRes << " to " << m_nMaxRes);
#endif

	std::map<std::string, std::string>::const_iterator it;

	m_bElev = false;
	it = m_mapAttr.find("elev");
	if (it != m_mapAttr.end())
	{
		m_bElev = true;
	}

	m_spCovDefn = PYXTableDefinition::create();
	if (m_bElev)
	{
		m_spCovDefn->addFieldDefinition(
			"elev", PYXFieldDefinition::knContextElevation, PYXValue::knFloat, 1);
	}
	else
	{
		m_spCovDefn->addFieldDefinition(
			"rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
	}

	// TODO: Should move this into an IDataProcessor
	it = m_mapAttr.find("gen");
	if (it != m_mapAttr.end())
	{
		if (!gen())
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Unable to generate data successfully");
			return knFailedToInit;
		}
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue BluePyxis::getCoverageValue(	const PYXIcosIndex& index,
										int nFieldIndex	) const
{
	boost::mutex::scoped_lock lock(m_mutex);

	assert(nFieldIndex == 0);

	int nRes = index.getResolution();

	if (!configRes(nRes))
	{
		return PYXValue();
	}

	PYXPointer<PYXValueTile> spValueTile;

	for (int n = 0; n != m_spValueTileCache.size(); ++n)
	{
		if (m_spValueTileCache[n]->getTile().getCellResolution() == nRes &&
			index.isDescendantOf(m_spValueTileCache[n]->getTile().getRootIndex()))
		{
			spValueTile = m_spValueTileCache[n];
		}
	}

	if (!spValueTile)
	{
		int nRootRes = std::max(index.getResolution() - 6, PYXIcosIndex::knMinSubRes);
		PYXIcosIndex rootIndex(index);
		rootIndex.setResolution(nRootRes);
		PYXTile tile(rootIndex, nRes);
		spValueTile = getCoverageTile(tile);
		// TODO make this a LIFO?
		m_spValueTileCache.push_front(spValueTile);
		if (m_knNumValueTileCache < m_spValueTileCache.size())
		{
			m_spValueTileCache.pop_back();
		}
	}

	return spValueTile->getValue(index, nFieldIndex);
}

PYXPointer<PYXValueTile> BluePyxis::getFieldTile(	const PYXIcosIndex& index,
													int nRes,
													int nFieldIndex	) const
{
	boost::mutex::scoped_lock lock(m_mutex);

	PYXPointer<PYXValueTile> spValueTile;

	for (int n = 0; n != m_spValueTileCache.size(); ++n)
	{
		if (m_spValueTileCache[n]->getTile().getCellResolution() == nRes &&
			m_spValueTileCache[n]->getTile().getRootIndex() == index)
		{
			spValueTile = m_spValueTileCache[n];
		}
	}

	if (!spValueTile)
	{
		spValueTile = getCoverageTile(PYXTile(index, nRes));
		// TODO make this a LIFO?
		m_spValueTileCache.push_front(spValueTile);
		if (m_knNumValueTileCache < m_spValueTileCache.size())
		{
			m_spValueTileCache.pop_back();
		}
	}

	return spValueTile;
}

PYXPointer<PYXValueTile> BluePyxis::getCoverageTile(const PYXTile& tile) const
{
	boost::mutex::scoped_lock lock(m_mutex);

	//assert(nFieldIndex == 0);

	PYXPointer<PYXValueTile> spTile = PYXValueTile::create(tile, m_bElev ? vecTypesElev : vecTypes, m_bElev ? vecCountsElev : vecCounts);
	int nValues = spTile->getNumberOfCells();

	int nRes = tile.getCellResolution();
	if (!configRes(nRes))
	{
		PYXValue v;
		for (int n = 0; n != nValues; ++n)
		{
			spTile->setValue(n, 0/*nFieldIndex*/, v);
		}
		return spTile;
	}

	PYXIcosIndex index = tile.getRootIndex();
	index.setResolution(nRes);

	unsigned int nPos = PYXIcosMath::calcCellPosition(index);
	unsigned int nSeek = nPos * (m_bElev ? 4 : 3);

	m_in.seekg(nSeek);
	if (!m_in.good())
	{
		TRACE_ERROR("BluePyxis seek error: size=" << m_nSize << " seek=" << nSeek);
	}

	if (m_bElev)
	{
		float* buf = new float[nValues];
		m_in.read((char*) buf, nValues * 4);
		if (!m_in.good())
		{
			TRACE_ERROR("BluePyxis read error: size=" << m_nSize << " seek=" << nSeek);
		}
		float* p = buf;
		for (int n = 0; n != nValues; ++n, ++p)
		{
			spTile->setValue(n, 0/*nFieldIndex*/, PYXValue(*p));
		}
		delete[] buf;
	}
	else
	{
		unsigned char* buf = new unsigned char[nValues * 3];
		m_in.read((char*) buf, nValues * 3);
		if (!m_in.good())
		{
			TRACE_ERROR("BluePyxis read error: size=" << m_nSize << " seek=" << nSeek);
		}
		unsigned char* p = buf;
		for (int n = 0; n != nValues; ++n, p += 3)
		{
			spTile->setValue(n, 0/*nFieldIndex*/, PYXValue(p, 3));
		}
		delete[] buf;
	}

	return spTile;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void BluePyxis::createGeometry() const
{
	// Blue PYXIS uses 2km data, currently converted to res 15
	// TODO[mlepage] need to verify this resolution
	m_spGeom = PYXGlobalGeometry::create(15);
}

/*!
\return true if succeeded, false if failed.
*/
bool BluePyxis::configRes(int nRes) const
{
#if 0
	if (nRes < m_nMinRes || m_nMaxRes < nRes)
	{
		return false;
	}
#endif

	if (nRes != m_nRes)
	{
		// TODO should use boost filesystem here
		std::string strFileName = (m_path / (toString(nRes) + ".bin")).native_file_string();
		if (m_in.is_open())
		{
			m_in.close();
		}
		m_nRes = -1;

#if 0
		// No need to clear cache now
		for (int n = 0; n != m_knNumValueTileCache; ++n)
		{
			m_spValueTileCache[n] = 0;
		}
#endif

		m_in.open(strFileName.c_str(), std::ofstream::binary);
		if (!m_in.good())
		{
			m_in.clear();
			return false;
		}

		m_in.seekg(0, std::ios::end);
		if (!m_in.good())
		{
			m_in.clear();
			return false;
		}

		m_nSize = m_in.tellg();
		if (!m_in.good())
		{
			m_in.clear();
			return false;
		}

		m_nRes = nRes;
	}

	return true;
}

/*!
Generates data.

\return true if succeeded, false if failed.
*/
bool BluePyxis::gen()
{
	std::map<std::string, std::string>::const_iterator it;

	int nMinRes = 5;
	it = m_mapAttr.find("minres");
	if (it != m_mapAttr.end())
	{
		nMinRes = atoi(it->second.c_str());
	}

	int nMaxRes = 15;
	it = m_mapAttr.find("maxres");
	if (it != m_mapAttr.end())
	{
		nMaxRes = atoi(it->second.c_str());
	}

	std::vector<boost::intrusive_ptr<ICoverage> > vecCov;

	for (int nInput = 1; ; ++nInput)
	{
		it = m_mapAttr.find(std::string("in") + toString(nInput));
		if (it == m_mapAttr.end())
		{
			break;
		}
		boost::intrusive_ptr<IProcess> spProc = PipeManager::getProcess(strToProcRef(it->second));

		if (spProc->initProc(true) != IProcess::knInitialized)
		{
			TRACE_ERROR("Unable to initialize process " << spProc);
			return false;
		}
		boost::intrusive_ptr<ICoverage> spCov;
		spProc->getOutput()->QueryInterface(ICoverage::iid, (void**) &spCov);
		assert(spCov);
		vecCov.push_back(spCov);
	}

	int nCovCount = static_cast<int>(vecCov.size());

	PYXValue v;
	unsigned char buf[4];

	for (int nRes = nMinRes; nRes <= nMaxRes; ++nRes)
	{
		std::string strFileName = (m_path / (toString(nRes) + ".bin")).native_file_string();
		TRACE_INFO("BluePyxis gen: processing data for " << strFileName);
		std::ofstream out(strFileName.c_str(), std::ofstream::binary);
		PYXIcosIterator it(nRes);
		for (; !it.end(); it.next())
		{
			bool bSuccess = false;
			for (int nCov = 0; nCov != nCovCount; ++nCov)
			{
				v = vecCov[nCov]->getCoverageValue(it.getIndex());
				if (!v.isNull())
				{
					bSuccess = true;
					if (nCov != 0)
					{
						// Adaptive input ordering
						std::swap(vecCov[0], vecCov[nCov]);
					}
					break;
				}
			}
			if (!bSuccess)
			{
				TRACE_ERROR("BluePyxis gen: no data for index " << it.getIndex().toString());
			}
			if (m_bElev)
			{
				*reinterpret_cast<float*>(buf) = v.getFloat();
				out.write((char*) buf, 4);
			}
			else
			{
				buf[0] = v.getUChar(0);
				buf[1] = v.getUChar(1);
				buf[2] = v.getUChar(2);
				out.write((char*) buf, 3);
			}
		}
	}

	TRACE_INFO("BluePyxis gen: done processing data");
	return true;
}
