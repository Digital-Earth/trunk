// simple.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "pyxlib_instance.h"
#include "pyxis/utility/trace.h"
#include "pyxis/utility/value.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"

int _tmain(int argc, _TCHAR* argv[])
{
	PYXLibInstance pyxLib("simple (C++)");

	// Index
	PYXIcosIndex i("A-0202");
	TRACE_INFO("i=" << i.toString());

	// Math
	PYXIcosIndex i2 = PYXIcosMath::move(i, PYXMath::knDirectionTwo);
	TRACE_INFO("i2=" << i2.toString());

	// Coordinates and projection
	CoordLatLon ll;
	ll.setInDegrees(45, -76);
	SnyderProjection::getInstance()->nativeToPYXIS(ll, &i, 16);
	TRACE_INFO("i=" << i.toString());

	// Value
	PYXValue v(3.14);
	TRACE_INFO("v=" << v.getString());
	PYXValue v2("Why is a raven like a writing desk?");
	TRACE_INFO("v2=" << v2.getString());

	// Iterator
	for (PYXChildIterator it(PYXIcosIndex("A-0"));
		!it.end(); it.next())
	{
		TRACE_INFO("child: " + it.getIndex().toString());
	}
	for (PYXVertexIterator it(i2); !it.end(); it.next())
	{
		TRACE_INFO("vertex: " + it.getIndex().toString());
	}
	for (PYXIcosIterator it(2); !it.end(); it.next())
	{
		TRACE_INFO("icos: " + it.getIndex().toString());
	}
	for (PYXExhaustiveIterator it(PYXIcosIndex("A-0"), 5);
		!it.end(); it.next())
	{
		TRACE_INFO("exhaustive: " + it.getIndex().toString());
	}

	return 0;
}

