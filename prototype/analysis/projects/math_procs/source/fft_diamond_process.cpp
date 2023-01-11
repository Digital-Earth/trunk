/******************************************************************************
fft_diamond_filter.cpp

begin		: 2006-06-29
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "fft_diamond_filter.h"

// local includes
//#include "binary_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"


// standard includes
#include <windows.h>

//! The unit test class
TesterUnit<PYXFFTDiamondFilter> gTester;


/*!
The unit test method for the class.
*/
void PYXFFTDiamondFilter::test()
{
	/*PYXIcosIndex index = "A-0200000"; 
	boost::shared_ptr<PYXBinaryCoverage> spBinaryCov(new PYXBinaryCoverage()); 

	PYXIcosIndex rootIndex= "A-0"; 
	int nRadius = 3; 
	PYXFFTDiamondFilter dFilt(index,nRadius);
	dFilt.setInput(spBinaryCov); 
	dFilt.performFFT(); 
	
	TEST_ASSERT(index.getResolution() ==
		dFilt.m_spSpatialDomainData->getTile().getCellResolution()); 

	TEST_ASSERT(index.getResolution() == 
		dFilt.m_spFreqDomainData->getTile().getCellResolution()); 

	dFilt.setRadius(1); 

	TEST_ASSERT(1 == dFilt.m_nRadius); 
	TEST_ASSERT(dFilt.m_bIsDirty == true); 

	dFilt.performFFT(); 
	TEST_ASSERT(dFilt.m_bIsDirty == false); */
	
}

/*!
Default Constructor. Throws an exception if radius is greater then 4. 
*/
PYXFFTDiamondFilter::PYXFFTDiamondFilter(const PYXIcosIndex& index, 
										 int nRadius):
	m_index(index),  
	m_bIsDirty(true)
{
		assignRadius(nRadius);
		m_spTransform = PYXFourierTransformDiamond::create(m_index, nRadius);
}


void PYXFFTDiamondFilter::assignRadius(int nRadius) 
{
	if (nRadius > 4 || nRadius <=0) 
	{
		PYXTHROW(PYXException,"Invalid Radius");
	}
	m_nRadius = nRadius;
}

/*!
Destructor.
*/
PYXFFTDiamondFilter::~PYXFFTDiamondFilter()
{
}

/*!
Overrides the setInput in the base class, sets the inputs 
in the base class, and sets the input so that an FFT can be
performed on the input coverage.

\param spCoverage	The input coverage to perform an FFT on.
*/
void PYXFFTDiamondFilter::setInput(PYXPointer<PYXCoverage> spCoverage) 
{
	assert(spCoverage.get() != 0 && "Invalid parameter.");
	assert(m_spTransform.get() !=0 && "No transform object.");
	PYXFilter::setInput(spCoverage);
	m_spInput = spCoverage;
	m_spTransform->setInput(spCoverage);
	m_bIsDirty = true; 
}

/*!
Performs the FFT on the input coverage. This is accomplished by 
wrapping the the work that is required to perform the FFT and generate 
the spatial domain and frequency domain data sources into one method call.
Running the FFT is a timely operation therefore only when data that impacts 
the direct results of the FFT is changed is there a reason to recompute 
the actual the FFT itself.
*/
void PYXFFTDiamondFilter::performFFT()
{
	assert(m_spTransform.get() !=0 && "No transform object.");
	if (m_bIsDirty)
	{
		m_spTransform->getLatticePts();
		m_spTransform->createSpatialData();	
		m_spTransform->spatialToFreq();
		m_spFreqDomainData = m_spTransform->createFreqDataSource();

		m_spFrequencyGeometry = m_spFreqDomainData->getTile().clone();

		m_bIsDirty = false;
	}
}

/*!
Sets the index at the centre of an area that the FFT is to be
computed on.

\param index	The index at the centre of the diamond which the FFT is 
				computed on.
*/
void PYXFFTDiamondFilter::setIndex(const PYXIcosIndex& index) 
{
	assert(!index.isNull() && "Invalid Parameter"); 
	assert(m_spTransform.get() !=0 && "No transform object.");
	
	m_spTransform->setIndex(index);
	m_index = index;
	m_bIsDirty = true;
	
}

/*!
Sets the radius of the dimaond which the FFT is computed on. The radius 
is the number of cells from the centre index until a vertex on in the 
horizontal directions.

\param	nRadius the radius of the diamond.
*/
void PYXFFTDiamondFilter::setRadius(int nRadius)
{
	assert(m_spTransform.get() !=0 && "No transform object.");
	assignRadius(nRadius);
	m_spTransform->setRadius(nRadius);
	m_bIsDirty = true;
}

/*!
Get the coverage value at the specified index. Ensures that the requested
index is the same resolution as the index that went into the tile. As the
values in the FFT are stored as doubles we need to convert them back to chars
for visualization. Check to see if the requested index is inside the spatial
geometry that was created when the FFT was performed. The image that is
visualized is a grey scale image.

A Null value will be returned if:
	- The FFT hasn't be performed.
	- The index doesn't intersect the geometry
	(The index doesn't fall inside the dimaond)
	- The value at that index is Null.
			
\param index	The index that a PYXValue is requested at.

\param nFieldIndex	The attribute index.

\return A PYXValue containing the value at the requested index.
*/

PYXValue PYXFFTDiamondFilter::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex) const
{
	assert(!index.isNull() && "Invalid Parameter"); 

	if (!m_bIsDirty)
	{
		PYXIcosIndex indexCopy(index);
		
		// adjust resolutions as required.
		indexCopy.setResolution(index.getResolution()); 
		
		if (m_spSpatialGeometry->intersects(const_cast<const PYXCell&>(PYXCell(index))))
		{
			PYXValue result = m_spSpatialDomainData->getValue(indexCopy,
				nFieldIndex);

			if (!result.isNull())
			{
				int nVal = 0; 
				uint8_t data[3] = { 0, 0, 0 };

				// Create a PYXValue to contain the value from the tile.
				PYXValue rtnValue = PYXValue::create(PYXValue::knUInt8, &data, 3, 0);
				nVal = result.getInt(0);
				rtnValue.setInt(0, nVal);
				rtnValue.setInt(1, nVal);
				rtnValue.setInt(2, nVal);
				return rtnValue;
			}
		}
	}
	return PYXValue(); 
}
/*!
Launches the lua viewer for viewing the dft. 
The code to generate the path to the lua viewer 
should come from a call to AppServices::getAppPath.
However since these values are intialized in a location 
that is currently unavailable until uilities moves to a 
DLL we are stuck with this hack.
*/
void PYXFFTDiamondFilter::viewOutput()
{
	//TODO: Uncommment this when hex view is available again.
	/*assert(m_spTransform.get() != 0 && "No transform object.");
	char szAppPath[MAX_PATH] = "";
	GetModuleFileName(0, szAppPath, MAX_PATH);
	std::string appPath(szAppPath); 
	appPath = appPath.substr(0,appPath.find_last_of("\\"));
	std::string command = appPath + "\\doris.exe -f hex-viewer3.lua"; 
	std::string param1 = m_spTransform->getRawOutputFilePath(); 
	std::string param2 = m_spTransform->getProcessedOutputFilePath();
	std::string execCommand;
	execCommand = command + ' ' + param1 + ' ' + param2;

	char runCommand[1024];
	strcpy(runCommand, execCommand.c_str());
	char direc[1024]; 
	strcpy(direc, appPath.c_str()); 

	STARTUPINFO sInfo;
	ZeroMemory(&sInfo,sizeof(sInfo));
	PROCESS_INFORMATION pInfo;
	ZeroMemory(&pInfo,sizeof(pInfo)); 
	sInfo.cb = sizeof(sInfo); 
	sInfo.dwFlags = STARTF_USESTDHANDLES;
	sInfo.hStdInput = NULL; 
	sInfo.hStdOutput = NULL; 
	sInfo.hStdError = NULL; 
	int nRtnVal = -1;
	
	nRtnVal = CreateProcess(NULL,runCommand,NULL,NULL,FALSE,0,NULL,direc,&sInfo,&pInfo); 

	DWORD err = -1000; 
	if (nRtnVal == 0) 
	{
		err = GetLastError();
	}*/
}
