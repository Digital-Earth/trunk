/******************************************************************************
pyx_rtree.cpp

begin		: 2004-11-30
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_DRIVER_UTILITY_SOURCE
#include "pyx_rtree.h"

// pyxlib includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/pyxcom.h"

// GiST Includes
#include "gist.h"
#include "gist_extensions.h"
#include "gist_rtpred_point.h"

// standard includes
#include <cassert>
#include <io.h>

//! The number of dimensions we are using
static const int knNumDimensions = 2;

//! Tester class
Tester<PYXrTree> gTester;

//! Test method
void PYXrTree::test()
{
	PYXrTree rTree;
	rTree.initialize();

	PYXRect2DDouble area;

	// insert above two coordinates & keys in the rtree
	for (long nIndex = 0; nIndex < 200; nIndex++)
	{
		area.setXMin(nIndex);
		area.setYMin(nIndex);
		area.setXMax(nIndex + 5);
		area.setYMax(nIndex + 5);

// disable warning C4312: 'type cast' : conversion from 'long' to 'void *' of greater size
#pragma warning(disable: 4312)

		void* pValue= reinterpret_cast<void*>(nIndex);
		rTree.insert(area, pValue);
	}

	// flush the values to file
	rTree.flush();

	// setup a couple of test points
	PYXCoord2DDouble ptIn(46.0, 50.0);
	PYXCoord2DDouble ptOut(1.0, 50.0);
	
	// test containsFirst
	const void* pData = rTree.containsFirst(ptIn);
	if (pData != 0)
	{
// disable warning C4311:'reinterpret_cast' : pointer truncation from 'const void *' to 'long'
#pragma warning(disable: 4311)

		long nKey = reinterpret_cast<long>(pData);
		TEST_ASSERT(nKey == 45);
	}
	else
	{
		TEST_ASSERT(false);
	}

	pData = rTree.containsFirst(ptOut);
	TEST_ASSERT(pData == 0);

	// test contains
	KeyList lstKeys;
	rTree.contains(ptIn, lstKeys);
	TEST_ASSERT(lstKeys.size() == 2);

	rTree.contains(ptOut, lstKeys);
	TEST_ASSERT(lstKeys.empty());

	// test overlaps
	PYXRect2DDouble rectIn(50.0, 50.0, 60.0, 60.0);
	rTree.overlaps(rectIn, lstKeys);
	TEST_ASSERT(lstKeys.size() == 16);

	PYXRect2DDouble rectOut(0.0, 50.0, 10.0, 60.0);
	rTree.overlaps(rectOut, lstKeys);
	TEST_ASSERT(lstKeys.empty());
}

/*!
Constructor initializes member variables.
*/
PYXrTree::PYXrTree() :
	m_pRTree(0),
	m_bFlushed(true),
	m_bTempFile(false)
{
}

/*!
Destructor deletes the rtree
*/
PYXrTree::~PYXrTree()
{
	// delete the rTree
	if (m_pRTree != 0)
	{
		m_pRTree->close();
	}

	// delete the temporary file
	if (m_bTempFile && !m_strTempFile.empty())
	{
		remove(m_strTempFile.c_str());
	}
}

/*!
Initialize the rTree. This method creates the rTree structure and a temporary
file for holding data.

\param	strFileName	Optional file name to use for permanent storage of index file on disk.
					If specified the file will be opened if it exists, or created if necessary.

\return true if file is newly created and empty, otherwise false.
*/
bool PYXrTree::initialize(std::string& strFileName) 
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// If we already have an rTree - close it and delete it.
	if (m_pRTree != 0)
	{
		m_pRTree->close();
		m_pRTree.reset();

		// Delete file associated with rTree.
		remove(m_strTempFile.c_str());
	}

	bool bNewEmptyFile = false;

	// get a temporary file
	if (strFileName.empty())
	{
		// get a temporary file
		m_strTempFile = FileUtils::pathToString(AppServices::makeTempFile());
		remove(m_strTempFile.c_str());
		m_bTempFile = true;
	}
	else
	{
		strFileName.append(".rtree");
		m_strTempFile = strFileName;
		m_bTempFile = false;
	}

	// create the new rtree index.
	m_pRTree.reset(new gist);
	if (m_pRTree == 0)
	{
		PYXTHROW(PYXException, "Unable to create rTree.");
	}

	if (m_pRTree->open(m_strTempFile.c_str()) != 0)
	{
		// create a new index file, using the given extension 
		// (here an rtree, specified by &rt_rect_ext)
		if (m_pRTree->create(m_strTempFile.c_str(), &rt_rect_ext) != 0)
		{
			// If not a temp file - we may not have access to this directory - default to a temp file and try again.
			if (m_bTempFile == false)
			{
				m_strTempFile = FileUtils::pathToString(AppServices::makeTempFile());
				remove(m_strTempFile.c_str());
				m_bTempFile = true;
				if (m_pRTree->create(m_strTempFile.c_str(), &rt_rect_ext) != 0)
				{
					PYXTHROW(	PYXFileException,
								"Unable to create file: '" << m_strTempFile << "'."	);
				}
			}
			else
			{
				PYXTHROW(	PYXFileException,
							"Unable to create file: '" << m_strTempFile << "'."	);
			}
		}
	}
	return m_pRTree->is_empty();
}

/*!
Inserts a rectangle and it's data into the rTree.

\param	rect	The rectangle.
\param	pData	The data (ownership retained by caller).
*/
void PYXrTree::insert(const PYXRect2DDouble& rect, const void* pData) 
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert((m_pRTree != 0) && "rTree not initialized.");
	
	// next 3 lines specify a rectangular search area
	double pfRect[] = {rect.xMin(), rect.xMax(), rect.yMin(), rect.yMax()};

	// insert above two coordinates & labels in the rtree
	m_pRTree->insert((void *) pfRect, sizeof(pfRect), (void*) &pData, sizeof(pData));
	m_bFlushed = false;
}

/*!
Flush the data to a file. This should be done after all items are added.
*/
void PYXrTree::flush() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert((m_pRTree != 0) && "rTree not initialized.");
	
	// flush data if it has not been flushed already
	if (!m_bFlushed)
	{
		m_pRTree->flush();
		m_bFlushed = true;
	}
}

/*!
Removes the R-Tree entry(ies) which contains the native point.

\param	native	The point which will be contained by the rectangle key.
*/
void PYXrTree::removeTreeEntry(const PYXCoord2DDouble& pt)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	TRACE_INFO("Removing an entry from the R-Tree.");

	assert((m_pRTree != 0) && "rTree not initialized.");

	// flush data if it has not been flushed already
	flush();

	// query the list
	const double pfPt[] =
	{
		pt.x(),
		pt.y()
	};

	rt_query_t query(rt_query_t::rt_contains, rt_query_t::rt_pointarg, new rt_point(knNumDimensions, pfPt));

	m_pRTree->remove(&query);
}

/*!
Get the first key whose area contains a specific point.

\param	pt	The point.

\return	The key or 0 if no key found.
*/
const void* PYXrTree::containsFirst(const PYXCoord2DDouble& pt) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert((m_pRTree != 0) && "rTree not initialized.");

	// flush data if it has not been flushed already
	flush();

	// query the list
	const double pfPt[] =
	{
		pt.x(),
		pt.y()
	};

	rt_query_t query(rt_query_t::rt_contains, rt_query_t::rt_pointarg, new rt_point(knNumDimensions, pfPt)); 

	// create a cursor (or iterator) based on the specified query
	gist_cursor_t cursor;
	m_pRTree->fetch_init(cursor, &query);

	double pfRectFetch[4];
	smsize_t nRectSize = sizeof(pfRectFetch);
	void* pKey = 0;
	smsize_t nKeySize = sizeof(pKey);

	bool bEOF = false;

	int nError = m_pRTree->fetch(	cursor, (void *) pfRectFetch, nRectSize, 
									(void*) &pKey, nKeySize, bEOF);

	/*
	The following code works around a bug in libgist where memory is not freed
	unless the query runs to completion.
	*/
	void* pTempKey = 0;
	while (!bEOF && (nError == RCOK))
	{
		int nError = m_pRTree->fetch(	cursor, (void *) pfRectFetch, nRectSize, 
										(void*) &pTempKey, nKeySize, bEOF);
	}

	return pKey;
}

/*!
Get all the keys whose areas contain the specified point.

\param	pt		The point.
\param	plstKey	The list of keys (in/out)
*/
void PYXrTree::contains(const PYXCoord2DDouble& pt, KeyList& lstKey) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert((m_pRTree != 0) && "rTree not initialized.");

	// flush data if it has not been flushed already
	flush();

	// clear the list
	lstKey.clear();

	// query the list
	const double pfPt[] =
	{
		pt.x(),
		pt.y()
	};

	rt_query_t query(rt_query_t::rt_contains, rt_query_t::rt_pointarg, new rt_point(knNumDimensions, pfPt)); 

	// create a cursor (or iterator) based on the specified query
	gist_cursor_t cursor;
	m_pRTree->fetch_init(cursor, &query);

	double pfRectFetch[4];
	smsize_t nRectSize = sizeof(pfRectFetch);
	void* pKey = 0;
	smsize_t nKeySize = sizeof(pKey);

	bool bEOF = false;
	do
	{
		int nError = m_pRTree->fetch(	cursor, (void *) pfRectFetch, nRectSize, 
										(void*) &pKey, nKeySize, bEOF	);
		if (!bEOF && (nError == RCOK))
		{
			lstKey.insert(pKey);
		}
	}
	while (!bEOF);
}

/*!
Get all the keys whose areas overlap with the specified rectangle.

\param	r		The rectangle.
\param	plstKey	The list of keys (in/out)
*/
void PYXrTree::overlaps(const PYXRect2DDouble& r, KeyList& lstKey) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert((m_pRTree != 0) && "rTree not initialized.");

	if (r.empty())
		return;

	// flush data if it has not been flushed already
	flush();

	// clear the list
	lstKey.clear();

	// query the list
	const double pfRect[] =
	{
		r.xMin(),
		r.xMax(),
		r.yMin(),
		r.yMax()
	};

	rt_query_t query(rt_query_t::rt_overlap, rt_query_t::rt_rectarg, new rt_rect(knNumDimensions, pfRect)); 

	// create a cursor (or iterator) based on the specified query
	gist_cursor_t cursor;
	m_pRTree->fetch_init(cursor, &query);

	double pfRectFetch[4];
	smsize_t nRectSize = sizeof(pfRectFetch);
	void* pKey = 0;
	smsize_t nKeySize = sizeof(pKey);

	bool bEOF = false;
	do
	{
		int nError = m_pRTree->fetch(	cursor, (void *) pfRectFetch, nRectSize, 
										(void*) &pKey, nKeySize, bEOF	);
		if (!bEOF && (nError == RCOK))
		{
			lstKey.insert(pKey);
		}
	}
	while (!bEOF);
}

void PYXrTree::overlaps(const PYXRect2DDouble &r1, PYXRect2DDouble &r2, PYXrTree::KeyList& lstKey) const
{
	KeyList keyListQ1;
	KeyList keyListQ2;
	overlaps(r1, keyListQ1);
	overlaps(r2, keyListQ2);

	for (KeyList::const_iterator listIt = keyListQ1.begin(); listIt != keyListQ1.end(); ++listIt)
	{
		lstKey.insert(*listIt);
	}
	for (KeyList::const_iterator listIt = keyListQ2.begin(); listIt != keyListQ2.end(); ++listIt)
	{
		lstKey.insert(*listIt);
	}
}

//! Is the tree empty?
bool PYXrTree::isEmpty() const
{
	return m_pRTree != 0 ? m_pRTree->is_empty() : false;
}

