/******************************************************************************
vdata.cpp

begin		: 2007-12-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "vdata.h"
#include "performance_counter.h"

// view model includes

// pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"

// boost includes

// standard includes
#include <deque>

// TEMP for debugging
#include "pyxis/utility/trace.h"

namespace
{

struct AutoTest
{
	AutoTest()
	{
		unsigned char random[] = { 23, 44, 11, 54, 76, 54, 77, 33 };
		int nSize = vbinGetPartSize(random);
	}
} autoTest;

}

int vdataGetSize(const std::vector<VectorFeat>& vdata)
{
	int nSize = 4; // COUNT: feat count

	for (std::vector<VectorFeat>::const_iterator it = vdata.begin();
		it != vdata.end(); ++it)
	{
		nSize += vdataGetFeatSize(*it); // COUNT: feat
	}

	return nSize;
}

int vdataGetFeatSize(const VectorFeat& vfeat)
{
	int nSize = 6; // COUNT: feat ID, part count

	for (std::vector<VectorPart>::const_iterator it = vfeat.m_parts.begin();
		it != vfeat.m_parts.end(); ++it)
	{
		nSize += vdataGetPartSize(*it); // COUNT: part
	}

	return nSize;
}

int vdataGetPartSize(const VectorPart& vpart)
{
	int nSize = 1; // COUNT: inout codes

	if (vpart.m_link[0].m_nDir && vpart.m_link[1].m_nDir)
	{
		nSize += 4; // COUNT: two part IDs
	}
	else if (vpart.m_link[0].m_nDir || vpart.m_link[1].m_nDir)
	{
		nSize += 2; // COUNT: one part ID
	}

	nSize += tuvGetRequiredPackBytes(static_cast<int>(vpart.m_verts.size())); // COUNT: verts

	return 0;
}

void vdataWriteTree(FILE* fp, const VectorPYXTree& vtree)
{
	std::deque<const VectorPYXTree*> deq(1, &vtree);

	while (!deq.empty())
	{
		// Pop self.
		const VectorPYXTree* pvt = deq.front();
		deq.pop_front();

		// Push children.
		const int nFlagCount = pvt->getFlagCount();
		for (int nPos = 0; nPos != nFlagCount; ++nPos)
		{
			deq.push_back(pvt->getChildAtPos(nPos));
		}

		// Write self.
		const unsigned char flagByte = pvt->getFlags();
		fwrite(&flagByte, 1, 1, fp); // WRITE: flags
		const unsigned int nFeatCount = static_cast<unsigned int>((**pvt).size());
		fwrite(&nFeatCount, 4, 1, fp); // WRITE: feat count
		//TRACE_INFO("~~~ featcount " << nFeatCount);
		for (std::vector<VectorFeat>::const_iterator it = (**pvt).begin();
			it != (**pvt).end(); ++it)
		{
			vdataWriteFeat(fp, *it); // WRITE: feat
		}
	}
}

void vdataReadTree(FILE* fp, VectorPYXTree& vtree)
{
	std::deque<VectorPYXTree*> deq(1, &vtree);

	while (!deq.empty())
	{
		// Pop self.
		VectorPYXTree* pvt = deq.front();
		deq.pop_front();

		// Read self.
		unsigned char flagByte;
		fread(&flagByte, 1, 1, fp); // READ: flags
		pvt->setFlags(flagByte);
		unsigned int nFeatCount;
		fread(&nFeatCount, 4, 1, fp); // READ: feat count
		//TRACE_INFO("~~~ featcount " << nFeatCount);
		pvt->data().resize(nFeatCount);
		for (std::vector<VectorFeat>::iterator it = (**pvt).begin();
			it != (**pvt).end(); ++it)
		{
			vdataReadFeat(fp, *it); // READ: feat
		}

		// Push children.
		const int nFlagCount = pvt->getFlagCount();
		pvt->allocChildren(nFlagCount);
		for (int nPos = 0; nPos != nFlagCount; ++nPos)
		{
			deq.push_back(pvt->getChildAtPos(nPos));
		}
	}
}

void vdataWriteFeat(FILE* fp, const VectorFeat& vfeat)
{
	fwrite(&vfeat.m_nFeatID, 4, 1, fp); // WRITE: feat ID
	//TRACE_INFO("~~~ featid " << vfeat.m_nFeatID);

	const unsigned short nPartCount = static_cast<unsigned short>(vfeat.m_parts.size());
	fwrite(&nPartCount, 2, 1, fp); // WRITE: part count
	//TRACE_INFO("~~~ partcount " << nPartCount);

	for (std::vector<VectorPart>::const_iterator it = vfeat.m_parts.begin();
		it != vfeat.m_parts.end(); ++it)
	{
		vdataWritePart(fp, *it); // WRITE: part
	}
}

void vdataReadFeat(FILE* fp, VectorFeat& vfeat)
{
	fread(&vfeat.m_nFeatID, 4, 1, fp); // READ: feat ID
	//TRACE_INFO("~~~ featid " << vfeat.m_nFeatID);

	// DEBUGGING
	if (2000 < vfeat.m_nFeatID)
	{
		int n = 42;
	}

	unsigned short nPartCount;
	fread(&nPartCount, 2, 1, fp); // READ: part count
	//TRACE_INFO("~~~ partcount " << nPartCount);
	vfeat.m_parts.resize(nPartCount);

	for (std::vector<VectorPart>::iterator it = vfeat.m_parts.begin();
		it != vfeat.m_parts.end(); ++it)
	{
		vdataReadPart(fp, *it); // READ: part
	}
}

void vdataWritePart(FILE* fp, const VectorPart& vpart)
{
	unsigned char linkByte = (vpart.m_link[1].m_nDir << 4) | vpart.m_link[0].m_nDir;
	fwrite(&linkByte, 1, 1, fp); // WRITE: link dirs
	//TRACE_INFO("~~~ linkbyte " << (int)linkByte);

	if (vpart.m_link[0].m_nDir)
	{
		fwrite(&vpart.m_link[0].m_nPartID, 2, 1, fp); // WRITE: next part ID
		//TRACE_INFO("~~~ nextpartid " << vpart.m_link[0].m_nPartID);
	}

	if (vpart.m_link[1].m_nDir)
	{
		fwrite(&vpart.m_link[1].m_nPartID, 2, 1, fp); // WRITE: prev part ID
		//TRACE_INFO("~~~ prevpartid " << vpart.m_link[1].m_nPartID);
	}

#if 0
	// Write out tuvs in blocks of 4.
	unsigned char buf[9];
	int nCount = static_cast<int>(vpart.m_verts.size());
	TUV dummy;
	const TUV* pTuv = nCount ? &vpart.m_verts[0] : &dummy;
	while (4 <= nCount)
	{
		tuvPack4(buf, pTuv);
		fwrite(buf, 1, 9, fp); // WRITE: verts
		pTuv += 4;
		nCount -= 4;
	}
	tuvPackN(buf, pTuv, nCount);
	fwrite(buf, 1, tuvGetRequiredPackBytes(nCount), fp); // WRITE: verts
#else
	unsigned short nVertCount = static_cast<unsigned short>(vpart.m_verts.size());
	fwrite(&nVertCount, 2, 1, fp); // WRITE: vert count
	//TRACE_INFO("~~~ vertcount " << nVertCount);
	for (std::vector<unsigned int>::const_iterator it = vpart.m_verts.begin();
		it != vpart.m_verts.end(); ++it)
	{
		// write out 3 little endian bytes
		VertCode nCode = *it;
		fwrite(reinterpret_cast<char*>(&nCode), 3, 1, fp);
		//TRACE_INFO("~~~ vertcode " << nCode);
	}
#endif
}

void vdataReadPart(FILE* fp, VectorPart& vpart)
{
	unsigned char linkByte;
	fread(&linkByte, 1, 1, fp); // READ: link dirs
	//TRACE_INFO("~~~ linkbyte " << (int)linkByte);
	vpart.m_link[0].m_nDir = linkByte & 0x0f;
	vpart.m_link[1].m_nDir = linkByte >> 4;

	if (vpart.m_link[0].m_nDir)
	{
		fread(&vpart.m_link[0].m_nPartID, 2, 1, fp); // READ: next part ID
		//TRACE_INFO("~~~ nextpartid " << vpart.m_link[0].m_nPartID);
	}

	if (vpart.m_link[1].m_nDir)
	{
		fread(&vpart.m_link[1].m_nPartID, 2, 1, fp); // READ: prev part ID
		//TRACE_INFO("~~~ prevpartid " << vpart.m_link[1].m_nPartID);
	}

	unsigned short nVertCount;
	fread(&nVertCount, 2, 1, fp); // READ: vert count
	//TRACE_INFO("~~~ vertcount " << nVertCount);
	vpart.m_verts.resize(nVertCount);

	for (std::vector<unsigned int>::iterator it = vpart.m_verts.begin();
		it != vpart.m_verts.end(); ++it)
	{
		// read in 3 little endian bytes
		VertCode nCode = 0;
		fread(reinterpret_cast<char*>(&nCode), 3, 1, fp);
		*it = nCode;
		//TRACE_INFO("~~~ vertcode " << nCode);
	}
}

VectorFeat& vdataInsertFeat(std::vector<VectorFeat>& vecFeat, FeatID nFeatID)
{
	for (std::vector<VectorFeat>::iterator it = vecFeat.begin();
		it != vecFeat.end(); ++it)
	{
		if (it->m_nFeatID == nFeatID)
		{
			return *it;
		}
	}

	// Not found so create.
	vecFeat.push_back(VectorFeat(nFeatID));
	return vecFeat.back();
}

int vbinGetSize(const unsigned char* ptr)
{
	const unsigned char* oldptr = ptr;

	int nFeatCount = *reinterpret_cast<const unsigned int*>(ptr);
	ptr += 4; // COUNT: feat count

	while (nFeatCount)
	{
		ptr += vbinGetFeatSize(ptr); // COUNT: feat
	}

	return static_cast<int>(ptr - oldptr);
}

int vbinGetFeatSize(const unsigned char* ptr)
{
	const unsigned char* oldptr = ptr;

	ptr += 4; // COUNT: feat ID

	int nPartCount = *reinterpret_cast<const unsigned short*>(ptr);
	ptr += 2; // COUNT: part count

	while (nPartCount)
	{
		ptr += vbinGetPartSize(ptr); // COUNT: part
	}

	return static_cast<int>(ptr - oldptr);
}

int vbinGetPartSize(const unsigned char* ptr)
{
	const unsigned char* oldptr = ptr;

	if ((*ptr & 0xf0) && (*ptr & 0x0f))
	{
		ptr += 4; // COUNT: two part IDs
	}
	else if (*ptr)
	{
		ptr += 2; // COUNT: one part ID
	}

	ptr += vbinGetVertSize(++ptr); // COUNT: inout code, verts

	return static_cast<int>(ptr - oldptr);
}

void vertCodeConvertFromIndex(const PYXIcosIndex& index0, const PYXIcosIndex& index11, VertCode& nVertCode)
{
	PYXIcosIndex root(index11); // root of offset
	root.setResolution(index0.getResolution()); // truncate

	nVertCode = PYXIcosMath::calcCellPosition(root, index11);
	assert(nVertCode < 320503); // number of possible children at res+11

	if (root != index0)
	{
		unsigned int nDir = static_cast<unsigned int>(PYXIcosMath::getNeighbourDirection(index0, root));
		nVertCode |= (nDir << 20); // leave 19 bits for storing the offset plus one more for nice alignment
	}
}

//Static map to keep track of all VectorRGBData
std::map<std::pair<ProcRef,PYXIcosIndex>,VectorRGBData*> s_vectorMap;
static boost::recursive_mutex s_vectorMapMutex;

VectorRGBData::VectorRGBData(const ProcRef& procref, const PYXIcosIndex& index) : m_id(procref,index), m_buf(NULL)
{
	PerformanceCounter::getValuePerformanceCounter("VectorRGBData",0.0f,1.0f,0.0f)->addToMeasurement(1);

	boost::recursive_mutex::scoped_lock lock(s_vectorMapMutex);
	s_vectorMap[m_id] = this;
}

PYXPointer<VectorRGBData> VectorRGBData::get(const ProcRef& procref, const PYXIcosIndex& index)
{
	std::pair<ProcRef,PYXIcosIndex> pair(procref,index);

	//check if we have this RGB data already...
	boost::recursive_mutex::scoped_lock lock(s_vectorMapMutex);
	if (s_vectorMap.find(pair) == s_vectorMap.end())
	{
		//Create a new map..
		//would insert itself in the vector map...
		return PYXNEW(VectorRGBData,procref,index);		
	}
	//does addRef
	return s_vectorMap[pair];
}

VectorRGBData::~VectorRGBData()
{
	//Remove itself from map
	{
		boost::recursive_mutex::scoped_lock lock(s_vectorMapMutex);
		s_vectorMap.erase(m_id);
	}

	PerformanceCounter::getValuePerformanceCounter("VectorRGBData",0.0f,1.0f,0.0f)->addToMeasurement(-1);
}

bool VectorRGBData::hasRGB()
{
	return m_buf != NULL;
}
void VectorRGBData::setRGB(unsigned char * buf)
{
	m_buf.reset(buf);
}
unsigned char * VectorRGBData::getRGB()
{
	return m_buf.get();
}

