/******************************************************************************
resolution_change_iterator.cpp

begin		: 2005-12-09
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/resolution_change_iterator.h"

// pyxlib includes
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXResolutionChangeIterator> gTester;

//! Test method
void PYXResolutionChangeIterator::test()
{
	// Empty iterator.
	{
		PYXPointer<PYXIterator> spItRaw(PYXEmptyIterator::create());
		PYXResolutionChangeIterator it(spItRaw, 4);
		PYXEmptyIterator itCheck;
		TEST_ASSERT(it.end() == itCheck.end());
		while (!itCheck.end())
		{
			TEST_ASSERT(it.getIndex() == itCheck.getIndex());
			it.next();
			itCheck.next();
			TEST_ASSERT(it.end() == itCheck.end());
		}
	}

	// Single iterator, same resolution.
	{
		PYXPointer<PYXIterator> spItRaw(PYXSingleIterator::create(PYXIcosIndex("A-000")));
		PYXResolutionChangeIterator it(spItRaw, 4);
		PYXSingleIterator itCheck(PYXIcosIndex("A-000"));
		TEST_ASSERT(it.end() == itCheck.end());
		while (!itCheck.end())
		{
			TEST_ASSERT(it.getIndex() == itCheck.getIndex());
			it.next();
			itCheck.next();
			TEST_ASSERT(it.end() == itCheck.end());
		}
	}

	// Single iterator, lower resolution.
	{
		PYXPointer<PYXIterator> spItRaw(PYXSingleIterator::create(PYXIcosIndex("A-000")));
		PYXResolutionChangeIterator it(spItRaw, 2);
		PYXSingleIterator itCheck(PYXIcosIndex("A-0"));
		TEST_ASSERT(it.end() == itCheck.end());
		while (!itCheck.end())
		{
			TEST_ASSERT(it.getIndex() == itCheck.getIndex());
			it.next();
			itCheck.next();
			TEST_ASSERT(it.end() == itCheck.end());
		}
	}

	// Single iterator, higher resolution.
	{
		PYXPointer<PYXIterator> spItRaw(PYXSingleIterator::create(PYXIcosIndex("A-000")));
		PYXResolutionChangeIterator it(spItRaw, 6);
		PYXExhaustiveIterator itCheck(PYXIcosIndex("A-000"), 6);
		TEST_ASSERT(it.end() == itCheck.end());
		while (!itCheck.end())
		{
			TEST_ASSERT(it.getIndex() == itCheck.getIndex());
			it.next();
			itCheck.next();
			TEST_ASSERT(it.end() == itCheck.end());
		}
	}

	// Exhaustive iterator, same resolution.
	{
		PYXPointer<PYXIterator> spItRaw(PYXExhaustiveIterator::create(PYXIcosIndex("A-00"), 4));
		PYXResolutionChangeIterator it(spItRaw, 4);
		PYXExhaustiveIterator itCheck(PYXIcosIndex("A-00"), 4);
		TEST_ASSERT(it.end() == itCheck.end());
		while (!itCheck.end())
		{
			TEST_ASSERT(it.getIndex() == itCheck.getIndex());
			it.next();
			itCheck.next();
			TEST_ASSERT(it.end() == itCheck.end());
		}
	}

	// Exhaustive iterator, lower resolution.
	{
		PYXPointer<PYXIterator> spItRaw(PYXExhaustiveIterator::create(PYXIcosIndex("A-00"), 4));
		PYXResolutionChangeIterator it(spItRaw, 2);
		PYXSingleIterator itCheck(PYXIcosIndex("A-0"));
		TEST_ASSERT(it.end() == itCheck.end());
		while (!itCheck.end())
		{
			TEST_ASSERT(it.getIndex() == itCheck.getIndex());
			it.next();
			itCheck.next();
			TEST_ASSERT(it.end() == itCheck.end());
		}
	}

	// Exhaustive iterator, higher resolution.
	{
		PYXPointer<PYXIterator> spItRaw(PYXExhaustiveIterator::create(PYXIcosIndex("A-00"), 4));
		PYXResolutionChangeIterator it(spItRaw, 6);
		PYXExhaustiveIterator itCheck(PYXIcosIndex("A-00"), 6);
		TEST_ASSERT(it.end() == itCheck.end());
		while (!itCheck.end())
		{
			TEST_ASSERT(it.getIndex() == itCheck.getIndex());
			it.next();
			itCheck.next();
			TEST_ASSERT(it.end() == itCheck.end());
		}
	}
}

/*!
Constructor creates an iterator given a source iterator, which iterates at
a specified resolution, decimating or tiling as necessary.

\param spIt				The source iterator. (must not be null)
\param nResolution		The resolution over which to iterate. (must be valid)
*/
PYXResolutionChangeIterator::PYXResolutionChangeIterator(	PYXPointer<PYXIterator> spIt,
															int nResolution	) :
	m_spIt(spIt),
	m_nResolution(nResolution)
{
	assert(m_spIt != 0 && "Iterator pointer must not be null.");
	assert(1 <= nResolution && nResolution <= PYXMath::knMaxAbsResolution && "Resolution must be valid.");

	if (!m_spIt->end())
	{
		const PYXIcosIndex& index = m_spIt->getIndex();

		if (m_nResolution < index.getResolution())
		{
			// Lower resolution: decimate.
			m_index = index;
			m_index.setResolution(m_nResolution);
		}
		else if (index.getResolution() < m_nResolution)
		{
			// Higher resolution: iterate.
			m_spItExhaustive = PYXExhaustiveIterator::create(index, m_nResolution);
		}
		else
		{
			// Same resolution: use index.
			m_index = index;
		}
	}
}

/*!
Move to the next cell.
*/
void PYXResolutionChangeIterator::next()
{
	if (end())
	{
		return;
	}

	if (m_spItExhaustive != 0)
	{
		m_spItExhaustive->next();
		if (!m_spItExhaustive->end())
		{
			return;
		}
		m_spItExhaustive = 0;
	}

	while (true)
	{
		m_spIt->next();
		if (m_spIt->end())
		{
			m_index.reset();
			return;
		}

		const PYXIcosIndex& index = m_spIt->getIndex();

		if (m_nResolution < index.getResolution())
		{
			// Lower resolution: decimate.
			if (!m_index.isAncestorOf(index))
			{
				m_index = index;
				m_index.setResolution(m_nResolution);
				return;
			}
		}
		else if (index.getResolution() < m_nResolution)
		{
			// Higher resolution: iterate.
			m_spItExhaustive = PYXExhaustiveIterator::create(index, m_nResolution);
			return;
		}
		else
		{
			// Same resolution: use index.
			m_index = index;
			return;
		}
	}
}
