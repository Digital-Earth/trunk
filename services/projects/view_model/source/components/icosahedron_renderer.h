#pragma once
#ifndef VIEW_MODEL__ICOSAHEDRON_RENDERER_H
#define VIEW_MODEL__ICOSAHEDRON_RENDERER_H
/******************************************************************************
icosahedron_renderer.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"


/*!

IcosahedronRenderer - render the the icosahedron

-- Description:
     - render the the icosahedron

-- OpenGL extentions: 
     - None

-- Limiations:
     - None

*/
//! IcosahedronRenderer - render the pyxis grid for all visible tiles on View. 
class IcosahedronRenderer : public Component
{

public:
	IcosahedronRenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<IcosahedronRenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(IcosahedronRenderer,viewThread); }

	virtual ~IcosahedronRenderer(void);

	virtual bool initialize();
	virtual void render();

private:
	float icosVert[12*3];
	static const unsigned short icosIndices[];
	static unsigned short icosInfo[];
};

#endif
