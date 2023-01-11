#ifndef PYXIS__GEOMETRY__CONSTANT_GEOMETRY_H
#define PYXIS__GEOMETRY__CONSTANT_GEOMETRY_H
/******************************************************************************
constant_geometry.h

begin		: 2006-11-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/multi_cell.h"

class PYXLIB_DECL PYXConstantGeometry : public PYXMultiCell
{
public:

	//! Unit test.
	static void test();

	virtual ~PYXConstantGeometry(void);

	void setDefinition( const std::string& definition);
	const std::string& getDefinition() const
	{
		return m_Definition;
	}

private:

	std::string m_Definition;
};

#endif // guard
