#pragma once
#ifndef VIEW_MODEL__VDATA_H
#define VIEW_MODEL__VDATA_H
/******************************************************************************
vdata.h

begin		: 2007-12-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "addr.h"
#include "pyxtree.h"
#include "tuv.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"

// boost includes
#include <boost/scoped_array.hpp>

// standard includes
#include <cstdlib>
#include <cstdio>
#include <vector>

typedef unsigned int FeatID;
typedef unsigned short PartID;
typedef unsigned int VertCode;

struct PartAddr
{
	PartAddr() : m_nDir(0), m_nPartID(0) {}

	unsigned char m_nDir; // padding will occupy 2 bytes anyways
	PartID m_nPartID;
};

struct VectorPart
{
	PartAddr m_link[2]; // 0=next, 1=prev
	std::vector<VertCode> m_verts;
};

namespace std
{

inline void swap(VectorPart& lhs, VectorPart& rhs)
{
	std::swap(lhs.m_link[0], rhs.m_link[0]);
	std::swap(lhs.m_link[1], rhs.m_link[1]);
	std::swap(lhs.m_verts, rhs.m_verts);
}

}

struct VectorFeat
{
	VectorFeat(FeatID nFeatID = 0) : m_nFeatID(nFeatID) {}

	FeatID m_nFeatID;
	std::vector<VectorPart> m_parts;
};

namespace std
{

inline void swap(VectorFeat& lhs, VectorFeat& rhs)
{
	std::swap(lhs.m_nFeatID, rhs.m_nFeatID);
	std::swap(lhs.m_parts, rhs.m_parts);
}

}

typedef BasicPYXTree<std::vector<VectorFeat> > VectorPYXTree;

int vdataGetSize(const std::vector<VectorFeat>& vdata);
int vdataGetFeatSize(const VectorFeat& vfeat);
int vdataGetPartSize(const VectorPart& vpart);

void vdataWriteTree(FILE* fp, const VectorPYXTree& vtree);
void vdataReadTree(FILE* fp, VectorPYXTree& vtree);

void vdataWriteFeat(FILE* fp, const VectorFeat& vfeat);
void vdataReadFeat(FILE* fp, VectorFeat& vfeat);

void vdataWritePart(FILE* fp, const VectorPart& vpart);
void vdataReadPart(FILE* fp, VectorPart& vpart);

VectorFeat& vdataInsertFeat(std::vector<VectorFeat>& vecFeat, FeatID nFeatID);

////////////////////////////////////////////////////////////////////////////////

typedef BasicPYXTree<unsigned char*> VbinPYXTree;

//! Reallocs memory. Does not copy.
inline unsigned char* vbinAlloc(unsigned char *ptr, int osize, int nsize)
{
    if (nsize == 0)
	{
		free(ptr);
		return 0;
	}
	else
	{
		return static_cast<unsigned char*>(realloc(ptr, nsize));
	}
}

//! Returns the size in bytes. O(n)
int vbinGetSize(const unsigned char* ptr);
int vbinGetFeatSize(const unsigned char* ptr);
int vbinGetPartSize(const unsigned char* ptr);
inline int vbinGetVertSize(const unsigned char* ptr)
{
	return tuvGetRequiredPackBytes(tuvCountPacked(ptr)); // COUNT: vertexes
}

void vertCodeConvertFromIndex(const PYXIcosIndex& index0, const PYXIcosIndex& index11, VertCode& nVertCode);

inline int vertCodeGetDir(VertCode nVertCode)
{
	return (nVertCode >> 20) & 0x7;
}

inline int vertCodeGetOffset(VertCode nVertCode)
{
	return nVertCode & 0x0007ffff;
}


//Store the resulted RGB Data

class VectorRGBData : public PYXObject
{
	boost::scoped_array<unsigned char> m_buf;
	std::pair<ProcRef,PYXIcosIndex> m_id;

protected:
	//created by the static get method
	VectorRGBData(const ProcRef& procref, const PYXIcosIndex& index);

public:
	static PYXPointer<VectorRGBData> get(const ProcRef& procref, const PYXIcosIndex& index);
	~VectorRGBData();

	//check if RGB buffer was set
	bool hasRGB();
	
	//the buffer would be deallocated on VectorRGBData destruction.
	//It must be a heap-allocated array.
	void setRGB(unsigned char * buf);
	
	//get buffer
	unsigned char * getRGB();
};

#endif
