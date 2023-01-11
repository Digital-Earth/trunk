/******************************************************************************
gl_utils.cpp

begin		: 2007-07-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "gl_utils.h"

// standard includes
#include <cassert>
#include <vector>
#include <string>

// view_model exceptions
#include "exceptions.h"

// font data
#include "font.txt"


static GLuint font_listbase;



void GLUtils::init()
{

	// Init the fonts
	font_listbase = glGenLists(256);
	if (font_listbase == 0) {
		// Checks if OpenGL is in a bad state.  This can occur if there is insufficient OpenGL
		// support by the graphics drivers.
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			PYXTHROW(PYXVisualizationException, "GUtils::init() failed: " << gluErrorString(error));
		}
		PYXTHROW(PYXVisualizationException, "GUtils::init() failed: Unknown OpenGL exception; could not create fonts.");
	}
	for (int row = 0; row != char_height; ++row)
	{
		for (int col = 0; col != char_width; ++col)
		{
			glNewList(font_listbase + row*char_width + col, GL_COMPILE);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, font_width);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, font_height - (row+1)*char_height);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, col*char_width);
			glDrawPixels(char_width, char_width, GL_LUMINANCE, GL_UNSIGNED_BYTE, font_bits);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glBitmap(0, 0, 0, 0, static_cast<GLfloat>(char_width), 0, 0); // advance 2D raster pos
			glEndList();
		}
	}
}

void GLUtils::uninit()
{
	glDeleteLists(font_listbase,256);
}

void GLUtils::drawCharacter(int nChar)
{
	glCallList(font_listbase + nChar);
}

void GLUtils::drawString(const std::string& str)
{
	glListBase(font_listbase);
	glCallLists(static_cast<GLsizei>(str.size()), GL_UNSIGNED_BYTE, str.c_str());
	glListBase(0);
}

void GLUtils::drawCharacterAt(int x, int y, int nChar)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glRasterPos2i(x, y);
	drawCharacter(nChar);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void GLUtils::drawStringAt(int x, int y, const std::string& str)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
		
	std::string buf(str);

	while (buf!="")
	{
		std::string line;

		int i = buf.find_first_of('\n');
		if (-1 == i)
		{
			line = buf;
			buf = "";
		} 
		else
		{
			line = buf.substr(0,i); // get line without the '\n' at the end.
			buf = buf.substr(i+1,buf.length()); //remove the line from buf, including skip the '\n'...
		}

		glRasterPos2i(x, y);
		drawString(line);

		y-=char_height;
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
