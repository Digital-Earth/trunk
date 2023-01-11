#ifndef PYX_FOURIER_TRANSFORM_HEXAGONAL_H
#define PYX_FOURIER_TRANSFORM_HEXAGONAL_H
/******************************************************************************
pyx_fourier_transform_hexagonal.h

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
PYXFourierTransformHexagon, performs a FFT computation on a group of
cells that form a hexagonal geometry. Indices are calculated in
hexagon iterator order. Processed by an FFT library from
Perfectly Scientific and stored in a tile. Two files
are created by creating a spatial data source and a
frequency data soure for use in an external viewer.
*/
//! PYXFourierTransformHexagon, Performs an FFT on a hexagonal group of cells.
class PYXFourierTransformHexagon : public PYXFourierTransform
{
public:
	
	//! Unit test method
	static void test();

	static PYXPointer<PYXFourierTransformHexagon> create(const PYXIcosIndex& index, int nRadius)
	{
		return PYXNEW(PYXFourierTransformHexagon,index,nRadius);
	}
	//! Default constructor.
	PYXFourierTransformHexagon(const PYXIcosIndex& index, int nRadius);



	//! Destructor.
	virtual ~PYXFourierTransformHexagon();

	//! Creates a PYXValueTile, representing the datasource frequency domain. 
	PYXPointer<PYXValueTile> createFreqData() const;

	//! Writes spatial data out to a text file. 
	void createSpatialData() const;

	//! Gets the lattice points from the image. 
	void getLatticePts();
	
protected:

	//! Disable copy constructor
	PYXFourierTransformHexagon(const PYXFourierTransformHexagon&);

	//! Disable copy assignment
	void operator =(const PYXFourierTransformHexagon&);

private:

};

#endif	// PYX_FOURIER_TRANSFORM_HEXAGONAL_H
