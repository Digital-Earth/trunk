#ifndef PYX_RTREE_H
#define PYX_RTREE_H
/******************************************************************************
pyx_rtree.h

begin		: 2004-11-30
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "module_driver_utility.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/rect_2d.h"

// system includes
#include <set>

// boost includes
#include <boost/thread/recursive_mutex.hpp>
#include <boost/scoped_ptr.hpp>

// forward declarations
class gist;

/*!
PYXrTree uses Guttmans rTree indexing scheme to manage a collection of
keys with rectangular area. Queries can be performed to determine which
keys contain a specified point or overlap with a specified rectangle.
*/
//! Indexes spatial data using the rTree indexing scheme
class MODULE_DRIVER_UTILITY_DECL PYXrTree
{
public:

	//! Test method.
	static void test();

	//! Constructor
	PYXrTree();

	//! Destructor.
	virtual ~PYXrTree();

	//! Initialize the rTree.  Return true if file was just created and is empty.
	bool initialize(std::string& strFileName = std::string());

	//! Insert a key into the tree.
	void insert(const PYXRect2DDouble& rect, const void* pKey);

	//! Flush the data to a file.
	void flush() const;

	//! Get the first key whose area contains the specified point.
	const void* containsFirst(const PYXCoord2DDouble& pt) const;

	//! List of keys
	typedef std::set<const void*> KeyList;

	//! Get all the keys whose areas contain the specified point.
	void contains(const PYXCoord2DDouble& pt, KeyList& lstKey) const;

	//! Get all the keys whose areas overlap with the specified rectangle
	void overlaps(const PYXRect2DDouble& r, KeyList& lstKey) const;

	//! Get all the keys whose area overlap either Rectangle. 
	void overlaps(const PYXRect2DDouble& r1, PYXRect2DDouble& r2, KeyList& lstKey) const;

	//! Removes the R-Tree entry which contains the point being passed in.
	void removeTreeEntry(const PYXCoord2DDouble& pt);

	//! Is the tree empty?
	bool isEmpty() const;

protected:

	//! Mutex to serialize concurrent access by multiple threads
	mutable boost::recursive_mutex m_mutex;

private:

	//! Disable copy constructor.
	PYXrTree(const PYXrTree&);

	//! Disable copy assignment.
	void operator=(const PYXrTree&);

	//! The rtree
	boost::scoped_ptr<gist> m_pRTree;

	//! The temporary file
	std::string m_strTempFile;

	//! Have values been flushed to file?
	mutable bool m_bFlushed;

	//! true if we are using a temporary file for the on-disk index.
	bool m_bTempFile;
};

#endif	// PYX_RTREE_H
