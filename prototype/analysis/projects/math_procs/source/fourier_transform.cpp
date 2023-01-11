/******************************************************************************
fourier_transform.cpp

begin		: 2006-08-11
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "fourier_transform.h"

// local includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"

// standard includes

//! The unit test class
TesterUnit<PYXFourierTransform> gTester;

/*!
The unit test method for the class.
*/
void PYXFourierTransform::test()
{
	TRACE_INFO("Test");
}

/*!
Default Constructor.
*/
PYXFourierTransform::PYXFourierTransform(const PYXIcosIndex& index, int nRadius):
	m_index(index),
	m_nRadius(nRadius)
{
	m_strDftRawOutput = AppServices::makeTempFile("txt").string();
	m_strDftProcOutput = AppServices::makeTempFile("txt").string();
}

/*!
Destructor.
*/
PYXFourierTransform::~PYXFourierTransform()
{
}

/*! 
Performs the fourier transform on a set of lattice points. Using a third party 
library from perfectly scientific, converts between lattice point vector 
to an array pointer to perform the transform and then back to a vector 
to store the points in the frequency domain. 
*/
void PYXFourierTransform::spatialToFreq() 
{ 
	assert(!m_vecLatticePts.empty() && 
		"No lattice points to perform fourier on."); 

	std::vector<double>::size_type nSize = m_vecLatticePts.size(); 
	m_vecSpatialVect.resize(nSize); 
	boost::shared_ptr<fft_complex> spInputComplex; 
	boost::shared_ptr<fft_complex> spOutputFourier; 

	//Allocate memory for input signal and output signal(fourier). 
	spInputComplex = boost::shared_ptr<fft_complex>((fft_complex*) new_fft_complex_signal(nSize),&free); 

	//Initialize values to zero.
	for (int nIndex = 0; nIndex < static_cast<signed int>(nSize); ++nIndex) 
	{
		spInputComplex.get()[nIndex].re = 0;
		spInputComplex.get()[nIndex].im = 0;
	}

	//converts real points to complex signal. 
	real_to_fft_complex(&(m_vecLatticePts[0]), spInputComplex.get(), nSize);

	//performs fourier transform. 
	dft(spInputComplex.get(), &(m_vecSpatialVect[0]), nSize);
}