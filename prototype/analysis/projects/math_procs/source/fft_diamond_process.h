#ifndef FFT_DIAMOND_FILTER
#define FFT_DIAMOND_FILTER
/******************************************************************************
fft_dimaond_filter.h

begin		: 2006-06-29
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"

#include "fourier_transform_diamond.h" 

// standard includes

// forward declarations

/*!
PYXFftDiamondFilter, is a wrapper filter for the PYXFourierTransformDiamond. 
This class is the fourier filter, providing access to the coverage in both 
the spatial domain, before the fourier transform, and the frequency domain, 
after the transform. The default implementation visualizes the data in 
spatial domain on the globe prior to performing the transform. Since 
it doesn't make sense to visualize frequency domain data in the spatial domain
there is currently no way to view the frequency domain data inside of the 
TVT. However access is provided through methods to get the PYXValueTile 
which contains the frequency data and to get the geometry of that tile. 
*/
//! PYXFftDiamondFilter, wrapper Filter for the PYXFourierTransformDiamond. 
class PYXFFTDiamondFilter : public FilterImpl
{
public:

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXFFTDiamondFilter(const PYXIcosIndex& index, 
		int nRadius);

	//! Destructor.
	virtual ~PYXFFTDiamondFilter();

	//! Set the input coverage (ownership shared with caller)
	virtual void setInput(PYXPointer<PYXCoverage> spInput);

	//! Perform the FFT calculation. 
	void performFFT(); 

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;
	
	//! Set the index which is the centre of the diamond. 
	void setIndex(const PYXIcosIndex& index); 

	//! Set the radius of the diamond. 
	void setRadius(int nRadius); 

	//! Launch the lua viewer to view output.
	void viewOutput(); 

private:
	/*
	Currently there is no need for these methods. They have been 
	implemented for some future design in which access to this data 
	is required. At which time these methods just need to be 
	elevated. 
	*/

	//! Get the PYXValueTile which contains the spatial domain data. 
	PYXPointer<PYXValueTile> getSpatialData() {
		return m_spSpatialDomainData;}

	//! Get the PYXValueTile which contains the frequency domain data. 
	PYXPointer<PYXValueTile> getFrequencyData() {
		return m_spFreqDomainData;}
	
	//! Get the geometry of the spatial domain tile. 
	PYXPointer<PYXGeometry> getSpatialGeometry() {
		return m_spSpatialGeometry;}

	//! Get the geometry of the frequency domain tile. 
	PYXPointer<PYXGeometry> getFrequencyGeometry() {
		return m_spFrequencyGeometry;}

		
	//! Tests for valid radius, throws exception.
	void assignRadius(int nRadius); 

protected:

	//! Disable copy constructor
	PYXFFTDiamondFilter(const PYXFFTDiamondFilter&);

	//! Disable copy assignment
	void operator =(const PYXFFTDiamondFilter&);

private:

	//! FFT Transform object, created when the filter is created. 
	PYXPointer<PYXFourierTransformDiamond> m_spTransform; 

	//! The input coverage, set only in setInput. 
	PYXPointer<PYXCoverage> m_spInput; 

	/*! 
	The geometry of the spatial domain tile. This is created
	everytime performFFT is executed. 
	*/
	PYXPointer<PYXGeometry> m_spSpatialGeometry; 

	/*! 
	The geometry of the frequency domain tile. This is created
	everytime performFFT is executed. 
	*/
	PYXPointer<PYXGeometry> m_spFrequencyGeometry; 

	/*!  
	Tile containing the values in the spatial domain. 
	This is created each time performFFT is executed. 
	*/
	PYXPointer<PYXValueTile> m_spSpatialDomainData; 
	
	/*!  
	Tile containing the values in the frequency domain. 
	This is created each time performFFT is executed. 
	*/
	PYXPointer<PYXValueTile> m_spFreqDomainData; 
	
	/*!
	The index at which the diamond is centred. 
	The index is set on construction and then can be changed 
	on setIndex. Changing the index will cause performFFT to be
	called and all data calculated in performFFT to be recalculated. 
	*/
	PYXIcosIndex m_index; 

	/*!
	The radius of the diamond. This value is set on construction and 
	can be changed in setRadius. Calling setRadius will cause performFFT 
	to be called and all data calculated in that method call to be 
	recalculated. 
	*/
	int m_nRadius; 

	/*!
	Indicates to performFFT whether any data has been changed. Thus 
	resulting in the need to recalculate the FFT. Is initialized to 
	true on construction. Set to false in performFFT and everytime 
	a setter method is called, set back to true. 
	*/
	bool m_bIsDirty; 

};

#endif	// end if
