/******************************************************************************
color_palette.cpp

begin		: 2011-09-07
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/color_palette.h"
#include "pyxis/utility/xml_utils.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>
#include <string>
#include <sstream>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
// PYXColorPalette
///////////////////////////////////////////////////////////////////////////////

//! Tester class
Tester< PYXColorPalette > gTester;

const std::string PYXColorPalette::knNumericPalette = "[NumericPalette]";

const std::string PYXColorPalette::knDefault = "Grayscale";
const std::string PYXColorPalette::knHSV = "HSV";
const std::string PYXColorPalette::knGreenToRed = "Green to Red";

PYXColorPalette::PYXColorPalette() //default grayscale palette
	: m_name(knNumericPalette + " " + knDefault)
{
	Step step;
	step.position = 0;
	step.color[0] = 0;
	step.color[1] = 0;
	step.color[2] = 0;
	step.color[3] = 255;
	m_steps.push_back(step);

	step.position = 1;
	step.color[0] = 255;
	step.color[1] = 255;
	step.color[2] = 255;
	step.color[3] = 255;
	m_steps.push_back(step);
}

PYXColorPalette::PYXColorPalette(const ColorStepsList & steps) : m_steps(steps)
{
	compileNameFromSteps();
}

PYXColorPalette::PYXColorPalette(const std::string & palette) : m_name(palette)
{
	if (m_name.find(knNumericPalette) == 0)
	{
		m_name = m_name.substr(knNumericPalette.size()+1);
	}

	if (m_name == knDefault)
	{
		Step step;
		step.position = 0;
		step.color[0] = 0;
		step.color[1] = 0;
		step.color[2] = 0;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 1;
		step.color[0] = 255;
		step.color[1] = 255;
		step.color[2] = 255;
		step.color[3] = 255;
		m_steps.push_back(step);

		m_name = knNumericPalette + " " + m_name;
		return;
	}

	if (m_name == knHSV)
	{
		Step step;
		step.position = 0;
		step.color[0] = 255;
		step.color[1] = 0;
		step.color[2] = 0;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 1/6.0;
		step.color[0] = 255;
		step.color[1] = 255;
		step.color[2] = 0;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 2/6.0;
		step.color[0] = 0;
		step.color[1] = 255;
		step.color[2] = 0;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 3/6.0;
		step.color[0] = 0;
		step.color[1] = 255;
		step.color[2] = 255;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 4/6.0;
		step.color[0] = 0;
		step.color[1] = 0;
		step.color[2] = 255;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 5/6.0;
		step.color[0] = 255;
		step.color[1] = 0;
		step.color[2] = 255;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 1;
		step.color[0] = 255;
		step.color[1] = 0;
		step.color[2] = 0;
		step.color[3] = 255;
		m_steps.push_back(step);

		m_name = knNumericPalette + " " + m_name;

		return;	
	}

	if (m_name == knGreenToRed)
	{
		Step step;
		step.position = 0;
		step.color[0] = 0;
		step.color[1] = 255;
		step.color[2] = 0;
		step.color[3] = 255;
		m_steps.push_back(step);

		step.position = 1;
		step.color[0] = 255;
		step.color[1] = 0;
		step.color[2] = 0;
		step.color[3] = 255;
		m_steps.push_back(step);

		m_name = knNumericPalette + " " + m_name;

		return;
	}

	std::istringstream input(m_name);

	int size;
	
	input >> size;

	for(int i=0;i<size;i++)
	{
		Step step;
		int rgba[4];
		input >> step.position >> rgba[0] >> rgba[1] >> rgba[2] >> rgba[3];

		step.color[0] = (uint8_t)rgba[0];
		step.color[1] = (uint8_t)rgba[1];
		step.color[2] = (uint8_t)rgba[2];
		step.color[3] = (uint8_t)rgba[3];

		m_steps.push_back(step);
	}

	compileNameFromSteps();
}

void PYXColorPalette::compileNameFromSteps()
{
	std::ostringstream out;

	out << knNumericPalette << " ";

	out << m_steps.size();

	for(ColorStepsList::iterator it = m_steps.begin(); it != m_steps.end(); ++it)
	{
		out << "  " << it->position << " " << (int)it->color[0] << " " << (int)it->color[1] << " " << (int)it->color[2] << " " << (int)it->color[3];
	}

	m_name = out.str();
}


void PYXColorPalette::convert(double position,uint8_t *rgba, bool alpha) const
{
	//check if position is nan, or palette is empty - make it a transparent pixel (or black if no alpha)
	if (_isnan(position) || m_steps.size()==0 ) 
	{
		memset(rgba,0,alpha?4:3);
	}

	if (position <= m_steps.front().position) 
	{
		memcpy(rgba,m_steps.front().color,alpha?4:3);
		return;
	}
	
	if (position >= m_steps.back().position)
	{
		memcpy(rgba,m_steps.back().color,alpha?4:3);
		return;
	}

	ColorStepsList::const_iterator it = m_steps.begin();

	while (it->position <= position) ++it;

	ColorStepsList::const_iterator last(it);
	--last;

	int step = (int)(256*(position - last->position) / (it->position - last->position));

	rgba[0] = (uint8_t)( ((int)last->color[0]*(256-step) + (int)it->color[0]*(step))>>8 );
	rgba[1] = (uint8_t)( ((int)last->color[1]*(256-step) + (int)it->color[1]*(step))>>8 );
	rgba[2] = (uint8_t)( ((int)last->color[2]*(256-step) + (int)it->color[2]*(step))>>8 );

	if (alpha)
	{
		rgba[3] = (uint8_t)( ((int)last->color[3]*(256-step) + (int)it->color[3]*(step))>>8 );
	}
}

void PYXColorPalette::convert(double position,PYXValue & rgba) const
{
	uint8_t rgba_bits[4];

	convert(position,rgba_bits);

	if (rgba.getArraySize()>=3)
	{
		rgba.set(0,rgba_bits[0]);
		rgba.set(1,rgba_bits[1]);
		rgba.set(2,rgba_bits[2]);
	}
	if (rgba.getArraySize()==4)
	{
		rgba.set(3,rgba_bits[3]);
	}
}

PYXValue PYXColorPalette::convert(double position,bool alpha) const
{
	uint8_t rgba[4];
	convert(position,rgba);
	return alpha?PYXValue(rgba,4):PYXValue(rgba,3);
}

void PYXColorPalette::test()
{
	uint8_t rgba[4];
	PYXColorPalette pal1;

	TEST_ASSERT(pal1.getName() == knNumericPalette + " " + knDefault);

	pal1.convert(0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==0 && rgba[2]==0 && rgba[3]==255);

	pal1.convert(1,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==255 && rgba[2]==255 && rgba[3]==255);

	pal1.convert(0.5,rgba);
	TEST_ASSERT(rgba[0]==127 && rgba[1]==127 && rgba[2]==127 && rgba[3]==255);

	rgba[3] = 30;
	pal1.convert(0,rgba,false);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==0 && rgba[2]==0 && rgba[3]==30);


	PYXColorPalette pal2(knHSV);

	pal2.convert(0,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==0 && rgba[2]==0 && rgba[3]==255);

	pal2.convert(0.5/6.0,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==127 && rgba[2]==0 && rgba[3]==255);

	pal2.convert(1/6.0,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==255 && rgba[2]==0 && rgba[3]==255);

	pal2.convert(1.5/6.0,rgba);
	TEST_ASSERT(rgba[0]==127 && rgba[1]==255 && rgba[2]==0 && rgba[3]==255);

	pal2.convert(2/6.0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==255 && rgba[2]==0 && rgba[3]==255);

	pal2.convert(2.5/6.0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==255 && rgba[2]==127 && rgba[3]==255);

	pal2.convert(3/6.0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==255 && rgba[2]==255 && rgba[3]==255);

	pal2.convert(3.5/6.0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==127 && rgba[2]==255 && rgba[3]==255);

	pal2.convert(4/6.0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==0 && rgba[2]==255 && rgba[3]==255);

	pal2.convert(4.5/6.0,rgba);
	TEST_ASSERT(rgba[0]==127 && rgba[1]==0 && rgba[2]==255 && rgba[3]==255);

	pal2.convert(5/6.0,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==0 && rgba[2]==255 && rgba[3]==255);

	pal2.convert(5.5/6.0,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==0 && rgba[2]==128 && rgba[3]==255);

	pal2.convert(6/6.0,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==0 && rgba[2]==0 && rgba[3]==255);


	PYXColorPalette pal3(knGreenToRed);

	pal3.convert(0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==255 && rgba[2]==0 && rgba[3]==255);

	pal3.convert(1,rgba);
	TEST_ASSERT(rgba[0]==255 && rgba[1]==0 && rgba[2]==0 && rgba[3]==255);

	pal3.convert(0.5,rgba);
	TEST_ASSERT(rgba[0]==127 && rgba[1]==127 && rgba[2]==0 && rgba[3]==255);

	//const between 0 and 1 and then jump to 100 and fade into 200
	PYXColorPalette pal4("4    0  0 0 0 255     1  0 0 0 255    1 100 100 100 255    2 200 200 200 255");


	TEST_ASSERT(pal4.getName() == knNumericPalette + " 4  0 0 0 0 255  1 0 0 0 255  1 100 100 100 255  2 200 200 200 255"); //canonized the string...

	pal4.convert(0,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==0 && rgba[2]==0 && rgba[3]==255);

	pal4.convert(0.5,rgba);
	TEST_ASSERT(rgba[0]==0 && rgba[1]==0 && rgba[2]==0 && rgba[3]==255);

	pal4.convert(1,rgba);
	TEST_ASSERT(rgba[0]==100 && rgba[1]==100 && rgba[2]==100 && rgba[3]==255);

	pal4.convert(1.5,rgba);
	TEST_ASSERT(rgba[0]==150 && rgba[1]==150 && rgba[2]==150 && rgba[3]==255);

	pal4.convert(2,rgba);
	TEST_ASSERT(rgba[0]==200 && rgba[1]==200 && rgba[2]==200 && rgba[3]==255);

	ColorStepsList list;
	Step step;

	step.position = 0;
	step.color[0] = 200;
	step.color[1] = 100;
	step.color[2] = 0;
	step.color[3] = 255;
	list.push_back(step);

	step.position = 1.5;
	step.color[0] = 0;
	step.color[1] = 50;
	step.color[2] = 70;
	step.color[3] = 255;
	list.push_back(step);

	PYXColorPalette pal5(list);

	PYXColorPalette pal6(pal5.getName());

	TEST_ASSERT(pal6.m_steps.size()==2);
	TEST_ASSERT(pal6.m_steps.front().position == 0);
	TEST_ASSERT(pal6.m_steps.front().color[0] == 200);
	TEST_ASSERT(pal6.m_steps.front().color[1] == 100);
	TEST_ASSERT(pal6.m_steps.front().color[2] == 0);
	TEST_ASSERT(pal6.m_steps.front().color[3] == 255);

	TEST_ASSERT(pal6.m_steps.back().position == 1.5);
	TEST_ASSERT(pal6.m_steps.back().color[0] == 0);
	TEST_ASSERT(pal6.m_steps.back().color[1] == 50);
	TEST_ASSERT(pal6.m_steps.back().color[2] == 70);
	TEST_ASSERT(pal6.m_steps.back().color[3] == 255);

	TEST_ASSERT(pal6.getName() == knNumericPalette +" 2  0 200 100 0 255  1.5 0 50 70 255");
}

///////////////////////////////////////////////////////////////////////////////
// PYXStringColorPalette
///////////////////////////////////////////////////////////////////////////////


//! Tester class
Tester< PYXStringColorPalette > gStringTester;

const std::string PYXStringColorPalette::knStringPalette = "[StringPalette]";

PYXStringColorPalette::PYXStringColorPalette() //default transparent palette
	: m_name(knStringPalette + " 0")
{
}

PYXStringColorPalette::PYXStringColorPalette(const ColorMap & map) : m_map(map)
{
	compileNameFromSteps();
}

PYXStringColorPalette::PYXStringColorPalette(const std::string & palette) : m_name(palette)
{
	if (m_name.find(knStringPalette) == 0)
	{
		m_name = m_name.substr(knStringPalette.size()+1);

		std::istringstream input(m_name);

		int size;

		input >> size;

		for(int i=0;i<size;i++)
		{
			std::string key;
			int rgba[4];
			input >> key >> rgba[0] >> rgba[1] >> rgba[2] >> rgba[3];

			if (key == "@")
			{
				//special encoding for empty string
				key = "";
			}
			else
			{
				//parse base64 string
				key = XMLUtils::fromBase64(key);
			}

			Color color;
			color.color[0] = (uint8_t)rgba[0];
			color.color[1] = (uint8_t)rgba[1];
			color.color[2] = (uint8_t)rgba[2];
			color.color[3] = (uint8_t)rgba[3];

			m_map[key ] = color;
		}
	}
	else 
	{
		//unexpected string - create empty palette...
	}

	compileNameFromSteps();
}

void PYXStringColorPalette::compileNameFromSteps()
{
	std::ostringstream out;

	out << knStringPalette << " ";
	
	out << m_map.size();

	for(ColorMap::iterator it = m_map.begin(); it != m_map.end(); ++it)
	{
		//"@" is special encoding for empty string.
		std::string key = "@";
		if (it->first.size()>0)
		{
			//if string is not empty, encode to base64.
			key = XMLUtils::toBase64(it->first);
		}
		out << "  " << key << " " << (int)it->second.color[0] << " " << (int)it->second.color[1] << " " << (int)it->second.color[2] << " " << (int)it->second.color[3];
	}

	m_name = out.str();
}


void PYXStringColorPalette::convert(const std::string & key,uint8_t *rgba, bool alpha) const
{
	ColorMap::const_iterator it = m_map.lower_bound(key);

	if (it == m_map.end())
	{
		if (m_map.empty())
		{
			memset(rgba,0,alpha?4:3);
		}
		else 
		{
			memcpy(rgba,&(m_map.rbegin()->second.color[0]),alpha?4:3);
		}
	}
	else 
	{
		memcpy(rgba,&(it->second.color[0]),alpha?4:3);
	}
}

void PYXStringColorPalette::convert(const std::string & key,PYXValue & rgba) const
{
	uint8_t rgba_bits[4];

	convert(key,rgba_bits);

	if (rgba.getArraySize()>=3)
	{
		rgba.set(0,rgba_bits[0]);
		rgba.set(1,rgba_bits[1]);
		rgba.set(2,rgba_bits[2]);
	}
	if (rgba.getArraySize()==4)
	{
		rgba.set(3,rgba_bits[3]);
	}
}

PYXValue PYXStringColorPalette::convert(const std::string & key,bool alpha) const
{
	uint8_t rgba[4];
	convert(key,rgba);
	return alpha?PYXValue(rgba,4):PYXValue(rgba,3);
}

void PYXStringColorPalette::test()
{
}

///////////////////////////////////////////////////////////////////////////////
// PYXValueColorPalette
///////////////////////////////////////////////////////////////////////////////


//! Tester class
Tester< PYXValueColorPalette > gValueTester;

PYXValueColorPalette::PYXValueColorPalette() //default transparent palette
	: m_name("")
{
}


PYXValueColorPalette::PYXValueColorPalette(const std::string & palette) : m_name(palette)
{
	if (m_name.find(PYXStringColorPalette::knStringPalette) == 0)
	{
		m_stringPalette = PYXStringColorPalette::create(palette);
		m_name = m_stringPalette->getName();
	}
	else 
	{
		m_numericPalette = PYXColorPalette::create(palette);
		m_name = m_numericPalette->getName();
	}
}

void PYXValueColorPalette::convert(const PYXValue & key,uint8_t *rgba, bool alpha) const
{
	if (m_stringPalette)
	{
		m_stringPalette->convert(key.getString(),rgba,alpha);
	}
	else {
		m_numericPalette->convert(key.getDouble(),rgba,alpha);
	}
}

void PYXValueColorPalette::convert(const PYXValue & key,PYXValue & rgba) const
{
	uint8_t rgba_bits[4];

	convert(key,rgba_bits);

	if (rgba.getArraySize()>=3)
	{
		rgba.set(0,rgba_bits[0]);
		rgba.set(1,rgba_bits[1]);
		rgba.set(2,rgba_bits[2]);
	}
	if (rgba.getArraySize()==4)
	{
		rgba.set(3,rgba_bits[3]);
	}
}

PYXValue PYXValueColorPalette::convert(const PYXValue & key,bool alpha) const
{
	uint8_t rgba[4];
	convert(key,rgba);
	return alpha?PYXValue(rgba,4):PYXValue(rgba,3);
}

void PYXValueColorPalette::test()
{
}