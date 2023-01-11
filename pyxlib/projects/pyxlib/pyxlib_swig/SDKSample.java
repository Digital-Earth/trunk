/**
 * Sample JAVA application to demonstrate the operation of the PYXIS SDK.
 * Note that this file is over-written when CompileDemo is run from the pyxlib_swig directory.
 */

import java.io.*;

import com.pyxisinnovation.pyxlib.*;
  
class SDKSample {
    public static void main(String[] args) 
    {
        try
        {
            // The dll must be loaded, or nothing will work.
			System.loadLibrary("pyxlib_swig_java");
			// Likewise, pyxlib must be initialized.
			PYXLibInstance.initialize( "test");

			// Demonstrates that we can access constants out of pyxlib.
			System.out.println("Pi is " + pyxlibConstants.PI);

			// Play with some PYXValue's                
			PYXValue x = new PYXValue( "42");
			System.out.println("x is " + x.getBool() );
			x.set( "42 fish");
			System.out.println("x is " + x.getString() );

			// And calculate a bounding box.
			PYXIcosIndex testLocation = GetSingleCellBoundingBox( 11.1999, 11.1999, 11.2, 11.2);
			System.out.println( "The rectangle from 11.1999, 11.1999 to 11.2, 11.2 is contained in " +
				testLocation.toString());
	    
			System.out.println("All is good."); // Display the string.
		}
		catch (Exception ex)
		{
			System.out.println("Error!"); // Display the string.
		}
    }
    
    public static PYXIcosIndex GetSingleCellBoundingBox(
        double xMin, double yMin, double xMax, double yMax)
    {
        int maxResolution = 18; // Res 18 == 400m
        PYXIcosIndex result = null;

        WGS84CoordConverter converter = new WGS84CoordConverter(1); // 1 unit per degree

        PYXRect2DDouble boundaryXY = new PYXRect2DDouble(
            xMin, yMin, xMax, yMax);
        for (int resolution = 3; resolution < maxResolution; ++resolution)
        {
            PYXXYBoundsGeometry_SPtr boundary = PYXXYBoundsGeometry.create(
                boundaryXY, converter, resolution);
            int cellCount = 0;
            PYXIcosIndex firstCell;

            for (PYXIterator_SPtr cell = boundary.getIterator(); !cell.end(); cell.next())
            {
                ++cellCount;
                if (cellCount > 3)
                    return result;
                
                if (cellCount == 1)
                    firstCell = new PYXIcosIndex( cell.getIndex());
            }
            if (cellCount == 1)
            {
                result = firstCell;
            }
        }

        // TODO: Spit out a warning here, or perhaps throw an exception...
        return result;
    }

}