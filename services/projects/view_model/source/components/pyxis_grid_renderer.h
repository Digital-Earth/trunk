#pragma once
#ifndef VIEW_MODEL__PYXIS_GRID_RENDERER_H
#define VIEW_MODEL__PYXIS_GRID_RENDERER_H
/******************************************************************************
pyxis_grid_renderer.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"


/*!

PyxisGridRenderer - render the pyxis grid for all visible tiles on View. 

-- Description:
     - This renderer is strongly depend on View visible tiles implemetation and on STile structure.

-- OpenGL extentions: 
     - None

-- Limiations:
     - None

*/
//! PyxisGridRenderer - render the pyxis grid for all visible tiles on View. 
class PyxisGridRenderer : public Component
{
public:
	PyxisGridRenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<PyxisGridRenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(PyxisGridRenderer,viewThread); }

	virtual ~PyxisGridRenderer(void);
	
	virtual void render();

protected:
	bool m_grid0Visible;
	bool m_grid2Visible;
	bool m_grid4Visible;

	void renderGrid0();
	void renderGrid2();
	void renderGrid4();

public:
	const bool & isGrid0Visbile() const { return m_grid0Visible; }
	const bool & isGrid2Visbile() const { return m_grid2Visible; }
	const bool & isGrid4Visbile() const { return m_grid4Visible; }

	void setGrid0Visbile(const bool & visible) { m_grid0Visible = visible; setVisible(m_grid0Visible || m_grid2Visible || m_grid4Visible); }	
	void setGrid2Visbile(const bool & visible) { m_grid2Visible = visible; setVisible(m_grid0Visible || m_grid2Visible || m_grid4Visible); }	
	void setGrid4Visbile(const bool & visible) { m_grid4Visible = visible; setVisible(m_grid0Visible || m_grid2Visible || m_grid4Visible); }	
};

#endif
