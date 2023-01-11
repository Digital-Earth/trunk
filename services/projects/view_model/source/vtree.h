#pragma once
#ifndef VIEW_MODEL__VTREE_H
#define VIEW_MODEL__VTREE_H
/******************************************************************************
vtree.h

begin		: 2008-04-25
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "addr.h"
#include "pyxtree.h"

// pyxlib includes
#include "pyxis/geometry/curve.h"
#include "pyxis/utility/coord_3d.h"

// standard includes
#include <cstdlib>
#include <cstdio>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

struct Fragment
{
	//! Type of index for point.
	typedef unsigned int PointIDType;

public:

	//! Convenience constructor.
	Fragment(PointIDType nBegin, PointIDType nEnd) :
		m_nBegin(nBegin),
		m_nEnd(nEnd)
	{
	}

public:

	//! Index of first point.
	PointIDType m_nBegin;
	//! Index of one past last point.
	PointIDType m_nEnd;
};

inline
bool operator ==(const Fragment& lhs, const Fragment& rhs)
{
	return lhs.m_nBegin == rhs.m_nBegin && lhs.m_nEnd == rhs.m_nEnd;
}

inline
bool operator <(const Fragment& lhs, const Fragment& rhs)
{
	return lhs.m_nBegin < rhs.m_nBegin
		|| lhs.m_nBegin == rhs.m_nBegin && lhs.m_nEnd < rhs.m_nEnd;
}

////////////////////////////////////////////////////////////////////////////////

struct FeatureNode
{
	//! Type of ID for feature.
	typedef unsigned int FeatureIDType;

public:

	FeatureNode(FeatureIDType nFid) :
		m_nFid(nFid)
	{
	}

	Fragment& insertPoint(Fragment::PointIDType nPid);

	Fragment& insertSegment(Fragment::PointIDType nPid);

private:

	std::vector<Fragment>::iterator lowerBoundPoint(Fragment::PointIDType nPid);

public:

	//! ID for this feature.
	FeatureIDType m_nFid;

	//! Fragments, stored by value, sorted by point ID.
	std::vector<Fragment> m_vecFragment;
};

////////////////////////////////////////////////////////////////////////////////

struct VTreeNode
{
public:

	//! Returns a feature node by ID, creating a new one if necessary.
	FeatureNode& insertFeatureNode(FeatureNode::FeatureIDType nFid);

	//! Returns a feature node by ID, or null if doesn't exist.
	FeatureNode* queryFeatureNode(FeatureNode::FeatureIDType nFid);

public:

	//! Feature nodes, stored by value, sorted by feature ID.
	std::vector<FeatureNode> m_vecFeatureNode;
};

////////////////////////////////////////////////////////////////////////////////

//! A VTree is just a PYX-Tree with VTreeNodes.
typedef BasicPYXTree<VTreeNode> VTreeNG;

std::pair<FeatureNode*, int> vtreeInsertPoint(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, Fragment::PointIDType nPid, const PYXIcosIndex& index);
std::pair<FeatureNode*, int> vtreeInsertSegment(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, Fragment::PointIDType nPid, const PYXIcosIndex& index);

FeatureNode* vtreeQueryFeatureNode(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index);
std::vector<Fragment> vtreeQueryFragments(const VTreeNG& vtree,  FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index);

void vtreeInsertFeatureCurve(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXCurve& curve);
void vtreeInsertFeatureCurve(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, int nPoints, const PYXCoord3DDouble* pPointData);

//! Trace a vtree to the trace log.
void vtreeTrace(const VTreeNG& vtree);

#endif
