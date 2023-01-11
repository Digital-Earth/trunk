using System;
using System.Collections.Generic;
using System.Text;

class Program
{

static int Main(string[] args)
{
    PYXLibInstance.initialize("simple (C#)");

    // Index
    PYXIcosIndex i = new PYXIcosIndex("A-0202");
    Trace.info("i=" + i.toString());

    // Math
    PYXIcosIndex i2 = PYXIcosMath.move(i, PYXMath.eHexDirection.knDirectionTwo);
    Trace.info("i2=" + i2.toString());

    // Coordinates and projection
    CoordLatLon ll = new CoordLatLon();
    ll.setInDegrees(45, -76);
    SnyderProjection.getInstance().nativeToPYXIS(ll, i, 16);
    Trace.info("i=" + i.toString());

    // Value
    PYXValue v = new PYXValue(3.14);
    Trace.info("v=" + v.getString());
    PYXValue v2 = new PYXValue("Why is a raven like a writing desk?");
    Trace.info("v2=" + v2.getString());

    // Iterator
    for (PYXChildIterator it = new PYXChildIterator(new PYXIcosIndex("A-0"));
        !it.end(); it.next())
    {
        Trace.info("child: " + it.getIndex().toString());
    }
    for (PYXVertexIterator it = new PYXVertexIterator(i2); !it.end(); it.next())
    {
        Trace.info("vertex: " + it.getIndex().toString());
    }
    for (PYXIcosIterator it = new PYXIcosIterator(2); !it.end(); it.next())
    {
        Trace.info("icos: " + it.getIndex().toString());
    }
    for (PYXExhaustiveIterator it = new PYXExhaustiveIterator(new PYXIcosIndex("A-0"), 5);
        !it.end(); it.next())
    {
        Trace.info("exhaustive: " + it.getIndex().toString());
    }

    PYXLibInstance.destroy();

    return 0;
}

}
