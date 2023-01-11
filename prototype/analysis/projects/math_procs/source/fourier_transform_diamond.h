#ifndef FOURIER_TRANSFORM_DIAMOND_H
#define FOURIER_TRANSFORM_DIAMOND_H
/******************************************************************************
fourier_transform.h

begin		: 2006-05-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "fourier_transform.h"

// standard includes

// forward declarations
class PYXValueTile;

/*!
PYXFourierTransformDiamond, performs an FFT on an image. The
actual area of the image that the transform is to be performed on
is determined by the total number of cells as well as the centre index. 
The centre index which is given when constructed is the centre point 
of the diamond and positions the diamond anywhere on the globe.
The size of the diamond is given by the radius. The radius is the
number of the cells from the centre point moving in a horizontal
direction to a vertex of the diamond. The radius also determines 
the number of the cells in the dimaond which is given by
totalCells = 9 ^ radius. Currently due to limitations
of the third party library which actually peforms the FFT, 
a size greater than 4 isn't possible. This entirely based 
on the limitations of the library.  
*/
//! Performs an FFT on a dimaond, placed anywhere on the globe. 
class PYXFourierTransformDiamond : public PYXFourierTransform
{
public:
	
	//! Unit test method
	static void test();

	static PYXPointer<PYXFourierTransformDiamond> create(PYXIcosIndex& index, int nRadius)
	{
		return PYXNEW(PYXFourierTransformDiamond,index,nRadius);
	}

	//! Default constructor.
	PYXFourierTransformDiamond(PYXIcosIndex& index, int nRadius);

	//! Destructor.
	virtual ~PYXFourierTransformDiamond();

	//! Gets the lattice points from the image. 
	void getLatticePts(); 

	//! Creates a PYXValueTile, representing the datasource frequency domain. 
	PYXPointer<PYXValueTile> createFreqDataSource()const; 

	//! Writes spatital data files out to a text file for viewing in lua. 
	void createSpatialData()const;
	
protected:

	//! Disable copy constructor
	PYXFourierTransformDiamond(const PYXFourierTransformDiamond&);

	//! Disable copy assignment
	void operator =(const PYXFourierTransformDiamond&);

private:

};

#endif	// end if
