#pragma once
#ifndef VIEW_MODEL__GL_UTILS_H
#define VIEW_MODEL__GL_UTILS_H
/******************************************************************************
gl_utils.h

begin		: 2007-07-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// standard includes
#include <string>

#define glError() { \
	GLenum err = glGetError(); \
	while (err != GL_NO_ERROR) { \
		TRACE_ERROR("glError: " << (char *)gluErrorString(err)); \
		err = glGetError(); \
	} \
}

/*!
OpenGL stuff.
*/
//! OpenGL stuff.
class VIEW_MODEL_API GLUtils
{
public:

	static void init();

	static void uninit();

public: // font stuff

	//! Draws a character at the current 2D raster position, which advances.
	static void drawCharacter(int nChar);

	//! Draws a string at the current 2D raster position, which advances.
	static void drawString(const std::string& str);

	//! Draws a character at the specified 2D raster position.
	static void drawCharacterAt(int x, int y, int nChar);

	//! Draws a string at the specified 2D raster position.
	static void drawStringAt(int x, int y, const std::string& str);

public:

	// TODO these would be more useful if they could be easily SWIG'd for use from C#.

	//! Converts screen coordinates to normalized device coordinates.
	static void screenToNDC(int x, int y, int w, int h, double& ndx, double& ndy)
	{
		ndx = ((double)x / w) * 2 - 1;
		ndy = -(((double)y / h) * 2 - 1);
	}

	//! Converts normalized device coordinates to screen coordinates.
	static void ndcToScreen(double ndx, double ndy, int w, int h, int& x, int& y)
	{
		// TODO should likely round not truncate?
		x = (int)((ndx + 1) / 2 * w);
		y = (int)((-ndy + 1) / 2 * h);
	}

};

#endif
