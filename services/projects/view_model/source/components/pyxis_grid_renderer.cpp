/******************************************************************************
pyxis_grid_renderer.cpp

begin		: 2007-07-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "pyxis_grid_renderer.h"

#ifdef false

#include "view_open_gl_thread.h"
#include "gl_utils.h"

#include "exceptions.h"


PyxisGridRenderer::PyxisGridRenderer(ViewOpenGLThread & viewThread) : Component(viewThread), m_grid0Visible(false),m_grid2Visible(false),m_grid4Visible(false)
{	
	setVisible(false);
}

PyxisGridRenderer::~PyxisGridRenderer(void)
{
}

void PyxisGridRenderer::render()
{
	getViewThread().setState("render Pyxis grid");
	if (m_grid0Visible)
	{
		renderGrid0();
	}
	if (m_grid2Visible)
	{
		renderGrid2();
	}
	if (m_grid4Visible)
	{
		renderGrid4();
	}
}

void PyxisGridRenderer::renderGrid0()
{
	glColor3f(0.25, 0.25, 0.25);
	glLineWidth(4);

	glEnableClientState(GL_VERTEX_ARRAY);

	getViewThread().applyCamera();	
	double altitudeFactor = 1 + getViewThread().getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius;

	// Use this to control local coordinate transformations
	glPushMatrix();
	vec3 localOrigin(0, 0, 0); // default local origin
	glTranslated(-0, -0, -altitudeFactor); // default base origin

	// Use this matrix to rotate local origin
	mat4 m;
	cml::matrix_rotation_quaternion(m, getViewThread().getCamera().getOrbitalRotation());
	glMultMatrixd(m.data());

	for (std::vector< PYXPointer<STile> >::iterator it = getViewThread().getVisibleSTiles().begin(); it != getViewThread().getVisibleSTiles().end(); ++it)
	{
		PYXPointer<STile> pSTile = *it;

		if (pSTile->getOrigin() != localOrigin)
		{
			// Local origin has changed so redo the transformations
			glPopMatrix();
			glPushMatrix();
			localOrigin = pSTile->getOrigin();
			vec3 v = localOrigin;
			v = cml::transform_vector(m, v);
			v -= vec3(0, 0, altitudeFactor);
			glTranslated(v[0], v[1], v[2]);
			glMultMatrixd(m.data());
		}

		if (pSTile->isValid())
			glColor3f(0,1,0);
		else if (pSTile->isProcessing())
			glColor3f(1,1,0);
		else
			glColor3f(1,0,0);

		for (int nTri = 0; nTri != 3; ++nTri)
		{
			glVertexPointer(3, GL_FLOAT, 0, pSTile->getVertexPointer(nTri));
			const unsigned short* pInfo = pSTile->getGrid0Info(nTri);
			const unsigned short* pIndices = pSTile->getGrid0Indices(nTri);
			const int nDrawCount = *pInfo++;
			for (int nDraw = 0; nDraw != nDrawCount; ++nDraw)
			{
				const int nCount = *pInfo++;				

				glDrawElements(GL_LINE_STRIP, nCount, GL_UNSIGNED_SHORT, pIndices);
				pIndices += nCount;
			}
		}
	}

	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
}

void PyxisGridRenderer::renderGrid2()
{
	glColor3f(0.375, 0.375, 0.375);
	glLineWidth(2);

	glEnableClientState(GL_VERTEX_ARRAY);

	getViewThread().applyCamera();
	double altitudeFactor = 1 + getViewThread().getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius;

	// Use this to control local coordinate transformations
	glPushMatrix();
	vec3 localOrigin(0, 0, 0); // default local origin
	glTranslated(-0, -0, -altitudeFactor); // default base origin

	// Use this matrix to rotate local origin
	mat4 m;
	cml::matrix_rotation_quaternion(m, getViewThread().getCamera().getOrbitalRotation());
	glMultMatrixd(m.data());

	for (std::vector< PYXPointer<STile> >::iterator it = getViewThread().getVisibleSTiles().begin(); it != getViewThread().getVisibleSTiles().end(); ++it)
	{
		PYXPointer<STile> pSTile = *it;

		if (pSTile->getOrigin() != localOrigin)
		{
			// Local origin has changed so redo the transformations
			glPopMatrix();
			glPushMatrix();
			localOrigin = pSTile->getOrigin();
			vec3 v = localOrigin;
			v = cml::transform_vector(m, v);
			v -= vec3(0, 0, altitudeFactor);
			glTranslated(v[0], v[1], v[2]);
			glMultMatrixd(m.data());
		}

		for (int nTri = 0; nTri != 3; ++nTri)
		{
			glVertexPointer(3, GL_FLOAT, 0, pSTile->getVertexPointer(nTri));
			const unsigned short* pInfo = pSTile->getGrid2Info(nTri);
			const unsigned short* pIndices = pSTile->getGrid2Indices(nTri);
			const int nDrawCount = *pInfo++;
			for (int nDraw = 0; nDraw != nDrawCount; ++nDraw)
			{
				const int nCount = *pInfo++;
				glDrawElements(GL_LINE_STRIP, nCount, GL_UNSIGNED_SHORT, pIndices);
				pIndices += nCount;
			}
		}
	}

	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
}

void PyxisGridRenderer::renderGrid4()
{
	glColor3f(0.5, 0.5, 0.5);
	glLineWidth(1);

	glEnableClientState(GL_VERTEX_ARRAY);

	getViewThread().applyCamera();
	double altitudeFactor = 1 + getViewThread().getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius;

	// Use this to control local coordinate transformations
	glPushMatrix();
	vec3 localOrigin(0, 0, 0); // default local origin
	glTranslated(-0, -0, -altitudeFactor); // default base origin

	// Use this matrix to rotate local origin
	mat4 m;
	cml::matrix_rotation_quaternion(m, getViewThread().getCamera().getOrbitalRotation());
	glMultMatrixd(m.data());

	for (std::vector< PYXPointer<STile> >::iterator it = getViewThread().getVisibleSTiles().begin(); it != getViewThread().getVisibleSTiles().end(); ++it)
	{
		PYXPointer<STile> pSTile = *it;

		if (pSTile->getOrigin() != localOrigin)
		{
			// Local origin has changed so redo the transformations
			glPopMatrix();
			glPushMatrix();
			localOrigin = pSTile->getOrigin();
			vec3 v = localOrigin;
			v = cml::transform_vector(m, v);
			v -= vec3(0, 0, altitudeFactor);
			glTranslated(v[0], v[1], v[2]);
			glMultMatrixd(m.data());
		}

		for (int nTri = 0; nTri != 3; ++nTri)
		{
			glVertexPointer(3, GL_FLOAT, 0, pSTile->getVertexPointer(nTri));
			const unsigned short* pInfo = pSTile->getGrid4Info(nTri);
			const unsigned short* pIndices = pSTile->getGrid4Indices(nTri);
			const int nDrawCount = *pInfo++;
			for (int nDraw = 0; nDraw != nDrawCount; ++nDraw)
			{
				const int nCount = *pInfo++;
				glDrawElements(GL_LINE_STRIP, nCount, GL_UNSIGNED_SHORT, pIndices);
				pIndices += nCount;
			}
		}
	}

	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
}

#endif