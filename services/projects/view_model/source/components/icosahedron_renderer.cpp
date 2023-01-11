/******************************************************************************
icosahedron_renderer.cpp

begin		: 2007-07-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "icosahedron_renderer.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"

#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/snyder_projection.h"

const unsigned short IcosahedronRenderer::icosIndices[] =
{
	0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 1,
	1, 6, 2, 2, 7, 3, 3, 8, 4, 4, 9, 5, 5, 10, 1,
	2, 6, 7, 3, 7, 8, 4, 8, 9, 5, 9, 10, 1, 10, 6,
	6, 11, 7, 7, 11, 8, 8, 11, 9, 9, 11, 10, 10, 11, 6
};

unsigned short IcosahedronRenderer::icosInfo[] =
{
	20, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};


bool IcosahedronRenderer::initialize()
{
	// TODO this should be in pyxlib somewhere, I use it often
	//! Vector of resolution 2 indices with which to begin traversal.
	std::vector<PYXIcosIndex> vecRes2;

	PYXIcosIterator it(2);
	for (; !it.end(); it.next())
	{
		vecRes2.push_back(it.getIndex());
	}

	// Icosahedron vertices.
	const SnyderProjection* pSnyder = SnyderProjection::getInstance();
	int nVert = 0;
	for (int n = 0; n != static_cast<int>(vecRes2.size()); ++n)
	{
		if (vecRes2[n].isPentagon())
		{
			PYXCoord3DDouble coord;
			pSnyder->pyxisToXYZ(vecRes2[n], &coord);
			icosVert[nVert*3+0] = static_cast<float>(coord.x());
			icosVert[nVert*3+1] = static_cast<float>(coord.y());
			icosVert[nVert*3+2] = static_cast<float>(coord.z());
			++nVert;
		}
	}

	return true;
}

IcosahedronRenderer::IcosahedronRenderer(ViewOpenGLThread & viewThread) : Component(viewThread)
{
}

IcosahedronRenderer::~IcosahedronRenderer(void)
{
}

void IcosahedronRenderer::render()
{
	getViewThread().setState("render Icosahedron");
	getViewThread().applyCameraWorld();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &icosVert[0]);

	// Solid.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 10);
	glColor3d(1, 0, 0);
	{
		const unsigned short* pInfo = icosInfo;
		const unsigned short* pIndices = icosIndices;
		const int nDrawCount = *pInfo++;
		for (int nDraw = 0; nDraw != nDrawCount; ++nDraw)
		{
			const int nCount = *pInfo++;
			glDrawElements(GL_TRIANGLES, nCount, GL_UNSIGNED_SHORT, pIndices);
			pIndices += nCount;
		}
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	// Wireframe.
	glColor3d(1, 1, 1);
	{
		const unsigned short* pInfo = icosInfo;
		const unsigned short* pIndices = icosIndices;
		const int nDrawCount = *pInfo++;
		for (int nDraw = 0; nDraw != nDrawCount; ++nDraw)
		{
			const int nCount = *pInfo++;
			glDrawElements(GL_TRIANGLES, nCount, GL_UNSIGNED_SHORT, pIndices);
			pIndices += nCount;
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);
}

