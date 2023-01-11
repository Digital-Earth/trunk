#pragma once
#ifndef VIEW_MODEL__VECTOR_UTILS_H
#define VIEW_MODEL__VECTOR_UTILS_H
/******************************************************************************
vector_utils.h

begin		: 2007-10-22
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "addr.h"
#include "pyxtree.h"
#include "stile.h"
#include "vdata.h"

// pyxlib includes

// boost includes
#include <boost/shared_ptr.hpp>

// standard includes
#include <vector>

// TODO m_verts as vector uses extra memory capacity and is on heap...
struct SData
{
	unsigned short m_sid;
	Addr m_addr[2]; // 0=prev, 1=next
	std::vector<PYXIcosIndex> m_verts;
};

struct FData
{
	unsigned int m_fid;
	std::vector<SData> m_sdata;
};

struct VData
{
	std::vector<FData> m_fdata;
};

typedef BasicPYXTree<VData> VTree;

inline
std::ostream& pyxtreeNodeToStream(std::ostream& out, const VTree& vtree)
{
	const VData& val = *vtree;
	int fcount = static_cast<int>(val.m_fdata.size());
	for (int fn = 0; fn != fcount; ++fn)
	{
		const FData& fdata = val.m_fdata[fn];
		out << fdata.m_fid << '(';
		int scount = static_cast<int>(fdata.m_sdata.size());
		for (int sn = 0; sn != scount; ++sn)
		{
			const SData& sdata = fdata.m_sdata[sn];
			out << sdata.m_sid << "->" << sdata.m_verts.size() << ' ';
		}
		out << ") ";
	}
	return out;
}

/*!
TODO need utilities for:
- manipulating denormalized indices
- manipulating trees
- generating vector data (for testing)
- managing vector data (trees)
- requesting vector data

Is it easier to build an expanded tree then collapse it, or to just build the collapsed tree?
*/
//! Vector data utilities.
class VIEW_MODEL_API VectorUtils
{
public:

	static void pushVerts(int fid, int nCount, const float* verts);

	static VTree& getTree();
	
	static boost::shared_ptr<VectorPYXTree> makeVectorTree(boost::intrusive_ptr<IProcess> spProc);
};

#endif
