/******************************************************************************
constant_geometry.cpp

begin		: 2006-11-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/constant_geometry.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXConstantGeometry> gTester;

PYXConstantGeometry::~PYXConstantGeometry(void)
{
}

template <typename T>
int countCells(T& g)
{
	int result = 0;

	for (	PYXPointer<PYXIterator> spIt(g.getIterator());
			!spIt->end();
			spIt->next()	)
	{
		++result;
	}

	return result;
}

void PYXConstantGeometry::test()
{
	PYXConstantGeometry myGeometry;
	TEST_ASSERT(countCells(myGeometry) == 0);

	myGeometry.setDefinition("A-0000");
	TEST_ASSERT(countCells(myGeometry) == 1);

	myGeometry.setDefinition("A-0000 A-0003");
	TEST_ASSERT(countCells(myGeometry) == 2);

	myGeometry.setDefinition("A-0000 A-0003 garbage");
	TEST_ASSERT(countCells(myGeometry) == 2);

	myGeometry.setDefinition("garbage");
	TEST_ASSERT(countCells(myGeometry) == 0);

	myGeometry.setDefinition("");
	TEST_ASSERT(countCells(myGeometry) == 0);
}

void PYXConstantGeometry::setDefinition(const std::string &definition)
{
	// Clean up anything we had before.
	setEmpty();

	m_Definition = definition;

	// Now parse the definition.  
	std::stringstream def(definition);
	while (def)
	{
		std::string currentCellText;
		if (def >> currentCellText)
		{
			try
			{
				PYXIcosIndex currentCell(currentCellText);
				if (!currentCell.isNull())
				{
					addIndex(currentCell);
				}
			}
			catch (PYXIndexException&)
			{
				// We ignore garbage.
			}
		}
	}
}
