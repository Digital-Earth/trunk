/******************************************************************************
vector_utils.cpp

begin		: 2007-10-22
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "vector_utils.h"

// view model includes
#include "addr_utils.h"
#include "pyxtree_utils.h"
#include "tuv.h"
#include "vdata.h"

// pyxlib includes
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/curve.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/string_utils.h"

// boost includes

// standard includes
#include <deque>
#include <map>

namespace
{

template <typename T>
inline
void writebin(std::ostream& out, T v)
{
	out.write(reinterpret_cast<char*>(&v), sizeof(v));
}

struct VectorInfo
{
	std::map<ProcRef, unsigned char*> m_map;
};

std::map<PYXIcosIndex, VectorInfo> vectorInfo;

// Return the feature data in a vector data, creating it if necessary.
FData& insertFData(VData& vdata, unsigned int fid)
{
	for (std::vector<FData>::iterator it = vdata.m_fdata.begin();
		it != vdata.m_fdata.end(); ++it)
	{
		if (it->m_fid == fid)
		{
			return *it;
		}
	}
	vdata.m_fdata.resize(vdata.m_fdata.size() + 1);
	vdata.m_fdata.back().m_fid = fid;
	return vdata.m_fdata.back();
}

// Return the segment data in a feature data, creating it if necessary.
SData& insertSData(FData& fdata, unsigned short sid)
{
	for (std::vector<SData>::iterator it = fdata.m_sdata.begin();
		it != fdata.m_sdata.end(); ++it)
	{
		if (it->m_sid == sid)
		{
			return *it;
		}
	}
	fdata.m_sdata.resize(fdata.m_sdata.size() + 1);
	fdata.m_sdata.back().m_sid = sid;
	return fdata.m_sdata.back();
}

SData& insertSData(VTree& vt, const Addr& addr, unsigned int fid, unsigned short sid)
{
	VTree* pvt = insertPTree(&vt, addr);
	FData& fdata = insertFData(**pvt, fid);
	SData& sdata = insertSData(fdata, sid);
	return sdata;
}

FData& insertFData(VTree& vt, const Addr& addr, unsigned int fid)
{
	VTree* pvt = insertPTree(&vt, addr);
	FData& fdata = insertFData(**pvt, fid);
	return fdata;
}

#if 0
SData* getPrevSData(VTree& vt, SData& sdata, int fid)
{
	SData* psdata = 0;
	unsigned short sid = sdata.m_slink[0].m_sid;
	if (sid != -1)
	{
		VTree* pvt = queryPTree(&vt, sdata.m_slink[0].m_addr);
		FData& fdata = insertFData(**pvt, fid);
		SData& sdata = insertSData(fdata, sid);
		psdata = &sdata;
	}
	return psdata;
}

SData* getNextSData(VTree& vt, SData& sdata, int fid)
{
	SData* psdata = 0;
	unsigned short sid = sdata.m_slink[1].m_sid;
	if (sid != -1)
	{
		VTree* pvt = queryPTree(&vt, sdata.m_slink[1].m_addr);
		FData& fdata = insertFData(**pvt, fid);
		SData& sdata = insertSData(fdata, sid);
		psdata = &sdata;
	}
	return psdata;
}
#endif

struct AutoTest
{
	AutoTest()
	{
	}
} autoTest;

VTree vt;

}

// To build level X of tree:
// - make PYXCurve at res X+11
// - follow curve to compute segments at res X
// - now go through verts at res X, assigning to segments)
void VectorUtils::pushVerts(int fid, int nCount, const float* verts)
{
#if 0
	const SnyderProjection* pSnyder = SnyderProjection::getInstance();

	int nRes = 2;

	PYXCoord3DDouble xyz;

	std::vector<PYXIcosIndex> vec11;
	std::vector<PYXCoord3DDouble> vecXYZ;
	std::vector<PYXIcosIndex> vec0;

	PYXIcosIndex index11;
	PYXIcosIndex index11Prev;
	PYXIcosIndex index0;

	// Convert each vertex at multiple resolutions suitable for use in curve.
	PYXCurve curve;
	for (int nVert = 0; nVert != nCount; ++nVert)
	{
		const float* v = verts + 3*nVert;
		xyz.set(v[0], v[1], v[2]);
		pSnyder->xyzToPYXIS(xyz, &index11, nRes + 11);
		if (index11Prev == index11)
		{
			continue;
		}
		index11Prev = index11;
		pSnyder->pyxisToXYZ(index11, &xyz);
		pSnyder->xyzToPYXIS(xyz, &index0, nRes);
		vec11.push_back(index11);
		vecXYZ.push_back(xyz);
		vec0.push_back(index0);
		curve.addNode(index11);
	}

	// Use curve to generate segments.
	unsigned short sid = 0;
	PYXPointer<PYXIterator> spIt = curve.getIterator();
	spIt->next(); // process first here before loop
	index0 = vec0.front();
	PYXIcosIndex index0Prev = index0;
	Addr addr0 = indexToAddr(index0);
	Addr addr0Prev = addr0;
	Addr addr0First = addr0;
	SData* psdata = &insertSData(vt, addr0, fid, sid);
	for (; !spIt->end(); spIt->next())
	{
		pSnyder->pyxisToXYZ(spIt->getIndex(), &xyz);
		pSnyder->xyzToPYXIS(xyz, &index0, nRes);
		if (index0Prev == index0)
		{
			continue;
		}
		index0Prev = index0;
		addr0 = indexToAddr(index0);
		psdata->m_addr[1] = addr0; // link current to next
		++sid;
		psdata = &insertSData(vt, addr0, fid, sid); // advance current
		psdata->m_sid = sid;
		psdata->m_addr[0] = addr0Prev; // link current to prev
		addr0Prev = addr0;
	}

	// Put vertices into appropriate segments.
	addr0Prev = addr0First;
	psdata = &insertSData(vt, addr0Prev, fid, 0);
	const int nVerts = static_cast<int>(vec11.size());
	for (int nVert = 0; nVert != nVerts; ++nVert)
	{
		addr0 = indexToAddr(vec0[nVert]);
		while (addr0Prev != addr0)
		{
			// Advance to next segment.
			addr0Prev = psdata->m_addr[1];
			psdata = &insertSData(vt, addr0Prev, fid, psdata->m_sid + 1);
		}

		psdata->m_verts.resize(psdata->m_verts.size() + 3);
		*(psdata->m_verts.end() - 3) = static_cast<float>(vecXYZ[nVert][0]);
		*(psdata->m_verts.end() - 2) = static_cast<float>(vecXYZ[nVert][1]);
		*(psdata->m_verts.end() - 1) = static_cast<float>(vecXYZ[nVert][2]);
	}

//	TRACE_INFO("\n" << ptreeToString(vt));
	int n = 42;
#endif
}

VTree& VectorUtils::getTree()
{
	return vt;
}

std::ostream& pyxtreeNodeToStream(std::ostream& out, const VectorPYXTree& vtree)
{
	const std::vector<VectorFeat>& val = *vtree;
	if (!val.empty())
	{
		out << val[0].m_parts.size();
	}
	return out;
}

boost::shared_ptr<VectorPYXTree> VectorUtils::makeVectorTree(boost::intrusive_ptr<IProcess> spProc)
{
#if 1
	// Do this only for a few data sources at the moment.
	{
		ProcRef pr(spProc);
		if (pr != strToProcRef("{E90C7BC1-D7EF-46CA-B284-9B45102868D7}[1]") // eurnasia
			&& pr != strToProcRef("{107EAC6B-7423-4FC3-8E1C-81CCC0ABD13A}[1]") // noamer
			&& pr != strToProcRef("{31CB99AE-2183-496C-B720-2745C97A09AE}[1]") // sasaus
			&& pr != strToProcRef("{50A932BE-9456-4DFE-916F-9782EBF0F312}[1]")) // soamafr
		{
			return boost::shared_ptr<VectorPYXTree>();
		}
	}
#endif

	const SnyderProjection* pSnyder = SnyderProjection::getInstance();

	boost::shared_ptr<VectorPYXTree> spVTree(new VectorPYXTree);
	VectorPYXTree& vtree = *spVTree; // the tree we are building

	boost::intrusive_ptr<IFeatureCollection> spFC;
	spProc->getOutput()->QueryInterface(IFeatureCollection::iid, (void**) &spFC);
	assert(spFC);

	// xyz we will use for coordinate conversions.
	PYXCoord3DDouble xyz;

	// The current res 0 index.
	PYXIcosIndex cur0;

	PYXIcosIndex next0;

	PYXIcosIndex origin;

	//TUV tuv; unreferenced variable

	TRACE_TIME("");

	for (int nRes = 2; nRes != 36; ++nRes)
	{
		// Use these for determining whether to continue with higher resolutions.
		int nContCount = 0;
		int nFeatCount = 0;

#if 1
		// The problem with this is we get features in a particular order which varies
		// (depending on the rtree I guess)
		PYXPointer<FeatureIterator> spFIt = spFC->getIterator(PYXGlobalGeometry(nRes + 11));
		for (; !spFIt->end(); spFIt->next())
		{
			boost::intrusive_ptr<IFeature> spF = spFIt->getFeature();
			unsigned int fid = atoi(spF->getID().c_str()); // TODO assuming much about FIDs here
#elif 0
		// The problem with this code is we get features with geometry not at the proper res.
		for (unsigned int fid = 90; fid != 91/*1981*/; ++fid)
		{
			boost::intrusive_ptr<IFeature> spF = spFC->getFeature(toString(fid));
#else
		std::vector<boost::intrusive_ptr<IFeature> > vecFeat;
		PYXPointer<FeatureIterator> spFIt = spFC->getIterator(PYXGlobalGeometry(nRes + 11));
		for (; !spFIt->end(); spFIt->next())
		{
			vecFeat.push_back(spFIt->getFeature());
		}
		random_shuffle(vecFeat.begin(), vecFeat.end());

		for (std::vector<boost::intrusive_ptr<IFeature> >::const_iterator it = vecFeat.begin();
			it != vecFeat.end(); ++it)
		{
			boost::intrusive_ptr<IFeature> spF = *it;
			unsigned int fid = atoi(spF->getID().c_str()); // TODO assuming much about FIDs here
#endif

			PYXPointer<PYXCurve> spCurve =
				boost::dynamic_pointer_cast<PYXCurve>(spF->getGeometry());
			if (!spCurve)
			{
				// If we get anything other than a curve, abort.
				return boost::shared_ptr<VectorPYXTree>();
			}
			PYXCurve& curve = *spCurve;

			const std::vector<PYXIcosIndex>& vec11 = spCurve->getNodes();

			// allow for up to 5% dropped nodes in the feature
			bool bCont = 0.05 <
				static_cast<double>(spCurve->getNodeDropCount()) /
					(spCurve->getNodeDropCount() + vec11.size());
			nContCount += bCont;
			++nFeatCount;

			// At this point, vec11 contains the curve's vertexes rasterized at res+11,
			// where each consecutive index is diffent (though it will repeat if the
			// curve goes back upon itself). This is basically the LOD.

//			TRACE_INFO("VTREE " << nRes << " " << fid << " nodecount " << vec11.size());

			// Use curve geometry to generate curve cells, which are broken into parts.
			PYXPointer<PYXIterator> spIt = curve.getIterator();

			// Process first cell (a curve vertex) here.

			// Convert to lower res and set new origin.
			if (spIt->getIndex().getSubIndex().getDigit(nRes + 1) == 0)
			{
				cur0 = spIt->getIndex();
				cur0.setResolution(nRes);
			}
			else
			{
				pSnyder->pyxisToXYZ(spIt->getIndex(), &xyz);
				pSnyder->xyzToPYXIS(xyz, &cur0, nRes);
			}
			origin = cur0;
			origin.setResolution(origin.getResolution() + 11);

			// Create new part.
			VectorPYXTree* pNode = insertPTree(&vtree, indexToAddr(cur0));
			VectorFeat* pFeat = &vdataInsertFeat(pNode->data(), fid);
			pFeat->m_parts.push_back(VectorPart());
			VectorPart* pPart = &pFeat->m_parts.back();

			// Add vertex to part as TUV.
#if 0
			tuvFromIndex(origin, tuv, spIt->getIndex());
			pPart->m_verts.push_back(tuv);
#else
			VertCode nVertCode;
			vertCodeConvertFromIndex(cur0, spIt->getIndex(), nVertCode);
			pPart->m_verts.push_back(nVertCode);
#endif

			spIt->next();
			int nVec11Pos = 1; // the vertex whose line segment (following) we are on.

			for (; !spIt->end(); spIt->next())
			{
				// Convert this cell to lower res. TODO should calculate in pure PYXIS.
				if (spIt->getIndex().getSubIndex().getDigit(nRes + 1) == 0)
				{
					next0 = spIt->getIndex();
					next0.setResolution(nRes);
				}
				{
					pSnyder->pyxisToXYZ(spIt->getIndex(), &xyz);
					pSnyder->xyzToPYXIS(xyz, &next0, nRes);
				}

				if (cur0 != next0)
				{
					// Create new part.
					VectorPYXTree* pNextNode = insertPTree(&vtree, indexToAddr(next0));
					VectorFeat* pNextFeat = &vdataInsertFeat(pNextNode->data(), fid);
					pNextFeat->m_parts.push_back(VectorPart());
					VectorPart* pNextPart = &pNextFeat->m_parts.back();

					// Note that although we've just invalidated pNode,
					// pFeat and pPart should still be OK.

					// Link it from/to previous part.
					pPart->m_link[0].m_nDir = PYXIcosMath::getNeighbourDirection(cur0, next0);
					pPart->m_link[0].m_nPartID = static_cast<PartID>(pNextFeat->m_parts.size() - 1);
					pNextPart->m_link[1].m_nDir = PYXIcosMath::getNeighbourDirection(next0, cur0);
					pNextPart->m_link[1].m_nPartID = static_cast<PartID>(pFeat->m_parts.size() - 1);

					// Advance to next part.
					pNode = pNextNode;
					pFeat = pNextFeat;
					pPart = pNextPart;
					cur0 = next0;
					origin = cur0;
					origin.setResolution(origin.getResolution() + 11);
				}

				if (spIt->getIndex() == vec11[nVec11Pos])
				{
					// Add vertex to part as TUV.
#if 0
					tuvFromIndex(origin, tuv, spIt->getIndex());
					pPart->m_verts.push_back(tuv);
#else
					//VertCode nCode; unreferenced
					vertCodeConvertFromIndex(cur0, spIt->getIndex(), nVertCode);
					pPart->m_verts.push_back(nVertCode);
#endif

					++nVec11Pos;
				}
			}
		}

		TRACE_TIME("VECTREE res " << nRes << " " <<
			nContCount << "/" << nFeatCount << " " << ((double)nContCount/nFeatCount));

		// allow for up to 5% features that want more resolution
		bool bContinue = 0.05 <
				static_cast<double>(nContCount) / nFeatCount;
		if (!bContinue)
		{
			break;
		}
	}

	return spVTree;
}
