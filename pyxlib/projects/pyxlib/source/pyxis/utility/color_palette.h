#ifndef PYXIS__UTILITY__COLOR_PALETTE_H
#define PYXIS__UTILITY__COLOR_PALETTE_H
/******************************************************************************
color_palette.h

begin		: 2011-09-07
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include <map>

#include "pyxis/utility/value.h"

/*!
PYXColorPalette - class to covert a the range [0..1] into RGB PYXValue
*/
//! This class used to estimate the cost of operation.
class PYXLIB_DECL PYXColorPalette : public PYXObject
{
private:
	struct Step
	{
		double position;
		uint8_t color[4];
	};
	typedef std::list<Step> ColorStepsList;

public:
	static const std::string knNumericPalette;

	static const std::string knDefault;
	static const std::string knHSV;
	static const std::string knGreenToRed;

public:
	static PYXPointer<PYXColorPalette> create() 
	{
		return PYXNEW(PYXColorPalette);
	}

	static PYXPointer<PYXColorPalette> create(const ColorStepsList & steps) 
	{
		return PYXNEW(PYXColorPalette,steps);
	}

	//palette can be a one of the default palette names or can be seralized string look like:
	//steps_count [position r g b a]*steps_count
	static PYXPointer<PYXColorPalette> create(const std::string & palette) 
	{
		return PYXNEW(PYXColorPalette,palette);
	}

	PYXColorPalette(); //default grayscale palette
	PYXColorPalette(const ColorStepsList & steps); //custom palette
	PYXColorPalette(const std::string & palette); //one of the built-in palettes or seralized values.

	const std::string & getName() { return m_name; };

private:
	ColorStepsList m_steps;
	std::string m_name;

	void compileNameFromSteps();

public:
	void convert(double position,uint8_t *rgba, bool alpha = true) const;

	void convert(double position,PYXValue & rgba) const;

	PYXValue convert(double position,bool alpha = true) const;

public:
	//! Unit test
	static void test();
};


/*!
PYXStringColorPalette - class to covert string values into RGB PYXValue
*/
//! This class used to estimate the cost of operation.
class PYXLIB_DECL PYXStringColorPalette : public PYXObject
{
private:
	
	struct Color
	{
		uint8_t color[4];
	};
	typedef std::map<std::string,Color> ColorMap;

public:
	static const std::string knStringPalette;	

public:
	static PYXPointer<PYXStringColorPalette> create() 
	{
		return PYXNEW(PYXStringColorPalette);
	}

	static PYXPointer<PYXStringColorPalette> create(const ColorMap & map) 
	{
		return PYXNEW(PYXStringColorPalette,map);
	}

	//palette can be a one of the default palette names or can be seralized string look like:
	//steps_count [base64(utf8(key)) r g b a]*steps_count
	static PYXPointer<PYXStringColorPalette> create(const std::string & palette) 
	{
		return PYXNEW(PYXStringColorPalette,palette);
	}

	PYXStringColorPalette(); //default transparent palette
	PYXStringColorPalette(const ColorMap & map); //custom palette
	PYXStringColorPalette(const std::string & palette); //one of the built-in palettes or seralized values.

	const std::string & getName() { return m_name; };

private:
	ColorMap m_map;
	std::string m_name;

	void compileNameFromSteps();

public:
	void convert(const std::string & key,uint8_t *rgba, bool alpha = true) const;

	void convert(const std::string & key,PYXValue & rgba) const;

	PYXValue convert(const std::string & key,bool alpha = true) const;

public:
	//! Unit test
	static void test();
};

/*!
PYXValueColorPalette  - class to covert PYXValue (numeric/string) values into RGB PYXValue
*/
//! This class used to estimate the cost of operation.
class PYXLIB_DECL PYXValueColorPalette : public PYXObject
{

public:
	static PYXPointer<PYXValueColorPalette> create() 
	{
		return PYXNEW(PYXValueColorPalette);
	}
	//palette can be a one of the default palette names or can be seralized string look like:
	//[StringPalette] [...]
	//[NumericPalette] [...]
	static PYXPointer<PYXValueColorPalette> create(const std::string & palette) 
	{
		return PYXNEW(PYXValueColorPalette,palette);
	}

	PYXValueColorPalette(); //default transparent palette	
	PYXValueColorPalette(const std::string & palette); //one of the built-in palettes or seralized values.

	const std::string & getName() { return m_name; };

private:
	PYXPointer<PYXColorPalette> m_numericPalette;
	PYXPointer<PYXStringColorPalette> m_stringPalette;
	std::string m_name;

public:
	bool isNumericPalette() { return m_numericPalette; }
	bool isStringPalette() { return m_stringPalette; }

	void convert(const PYXValue & value,uint8_t *rgba, bool alpha = true) const;
	void convert(const PYXValue & value,PYXValue & rgba) const;
	PYXValue convert(const PYXValue & value,bool alpha = true) const;

public:
	//! Unit test
	static void test();
};

#endif // guard
