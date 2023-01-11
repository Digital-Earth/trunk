#ifndef FOURIER_TRANSFORM_H
#define FOURIER_TRANSFORM_H
/******************************************************************************
fourier_transform.h

begin		: 2006-08-11
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"

#include "fft.h"

// standard includes

// forward declarations

/*!
Performs an FFT on an image. The actual area of the image that the
transform is to be performed onis determined by the total number 
of cells as well as the centre index. The centre index which is
given when constructed is the centre point of the diamond and 
positions the diamond anywhere on the globe. 
*/
//! Base class for performing an FFT on a Pyxis Grid.
class PYXFourierTransform : public FilterImpl
{
public:

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXFourierTransform(const PYXIcosIndex& index, int nRadius);

	//! Destructor.
	virtual ~PYXFourierTransform();
	
	//! Sets the output path of the file containing the original iamge.
	virtual void setFileOriginalPath(const std::string& strPath) 
		{m_strDftRawOutput = strPath;} 

	//! Sets the path of the processed file containing the processed image.
	virtual void setFileProcPath(const std::string& strPath) 
		{m_strDftProcOutput = strPath;} 

	 //! Gets the output path of the orginal image.
	 virtual const std::string getRawOutputFilePath() const
	 {return  m_strDftRawOutput;} 

	 //! Gets the processed path of the output file.
	 virtual std::string getProcessedOutputFilePath() const 
	 {return m_strDftProcOutput;} 

	 //! Sets the index to perform the fourier on.
	 virtual void setIndex(PYXIcosIndex index) {m_index = index;}

	 //! Sets the radius of the area to perform the fourier on. 
	 virtual void setRadius(int nRadius) {m_nRadius = nRadius;}

	 //! Get the radius of the transform. 
	 virtual int getRadius() const {return m_nRadius;};

	 //! Get the index that the fourier transform is to be performed on.
	 virtual const PYXIcosIndex& getIndex() const {return m_index;}

	 //! Runs the fourier transform. Translating spatial -> freq. 
	 virtual void spatialToFreq();

protected:

	//! Disable copy constructor
	PYXFourierTransform(const PYXFourierTransform&);

	//! Disable copy assignment
	void operator =(const PYXFourierTransform&);

	//! Vector containing the results of the FFT. 
	std::vector<fft_complex> m_vecSpatialVect;

	//! Vector containing the original lattice points to be fed into the FFT. 
	std::vector<double> m_vecLatticePts;

private:

	//! Index which is the centre of the diamond to perform the FFT on. 
	 PYXIcosIndex m_index;
	
	//! Radius of the diamond. 
	int m_nRadius;

	/*! 
	String path to the 	file of the output original image. For processing in 
	the lua viewer. This set on construction or on setFileOriginalPath 
	*/
	std::string m_strDftRawOutput; 

	/*! 
	String path to the 	file of the output processed image. For processing in the lua viewer.
	This set on construction or on setFileOriginalPath 
	*/
	std::string m_strDftProcOutput; 

};

#endif	// end if
