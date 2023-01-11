/******************************************************************************
clamped_sampler.cpp

begin		: 2010-Jan-14
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "clamped_sampler.h"
//#include "bilinear_sampler.h"
#include "bicubic_sampler.h"
#include "nearest_neighbour_sampler.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/pyxcom.h"

// standard includes
#include <boost/cast.hpp>

// {03A11842-7EE9-400e-9CA1-A4942E99E8FE}
PYXCOM_DEFINE_CLSID(ClampedSampler, 
0x3a11842, 0x7ee9, 0x400e, 0x9c, 0xa1, 0xa4, 0x94, 0x2e, 0x99, 0xe8, 0xfe);

PYXCOM_CLASS_INTERFACES(ClampedSampler, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ClampedSampler, "Standard Sampler", 
					"Converts xy data into PYXIS cells, with smooth interpolation for oversampled data.", "Sampling",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IXYCoverage::iid, 1, 1, "Input Coverage", "The XY coverage that is being sampled.")
IPROCESS_SPEC_END


ClampedSampler::ClampedSampler() :
		m_pCoordConverter(0), m_nativeResolution( -1)
{
	m_strID = "Clamped Sampler: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
}



////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in
\param	nFieldIndex	The field index.

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool ClampedSampler::getCoverageValue(	const PYXIcosIndex& index,
												PYXValue* pValue,
												int nFieldIndex	) const
{
	return getActualSampler( index.getResolution())->getCoverageValue( 
		index,pValue,nFieldIndex);
}



/*!
A threaded data retrieval for a tile of data.

\param	index		The root index of the tile.
\param	nRes		The resolution of the tile.
\param	nFieldIndex	The field index.

\return	The values. May return null if no values.
*/
PYXPointer<PYXValueTile> ClampedSampler::getFieldTile(const PYXIcosIndex& index,
												   int nRes,
												   int nFieldIndex) const
{
	return getActualSampler( nRes)->getFieldTile( 
		index,nRes,nFieldIndex);
}

bool ClampedSampler::generateCoverageValue(const PYXIcosIndex & index,
										const PYXCoord2DDouble & nativeCoord,
										bool * hasValues,
										PYXValue * values,
										int width,int height,
										PYXValue * resultValue) const
{
	PYXTHROW(PYXException,"Should never be called");
}

int ClampedSampler::getSamplingMatrixSize() const
{
	PYXTHROW(PYXException,"Should never be called");
}

// The following data table is copied from http://www.pyxisinnovation.com/pyxwiki/index.php?title=PYXIS_Resolutions
struct ResolutionData {
	int Resolution;
	double HexagonArea; // (m^2) 	 
	long long CentroidCellCount;  	 
	long long VertexCellCount;
	long long TotalGridCellCount;
	double	CLS; // (m)
} ResolutionTable[] = {
//{ 0, N/A, 12, 0, 12, N/A}, 
{ 1, 15939499735869.3, 12, 20, 32, 4434026.26}, 
{ 2, 5313166578623.11, 32, 60, 92, 2587366.56}, 
{ 3, 1771055526207.7, 92, 180, 272, 1499049.08}, 
{ 4, 590351842069.23, 272, 540, 812, 866481.01}, 
{ 5, 196783947356.41, 812, 1620, 2432, 500456.23}, 
{ 6, 65594649118.8, 2432, 4860, 7292, 288975.71}, 
{ 7, 21864883039.6, 7292, 14580, 21872, 166847.36}, 
{ 8, 7288294346.53, 21872, 43740, 65612, 96330.74}, 
{ 9, 2429431448.84, 65612, 131220, 196832, 55616.85}, 
{ 10, 809810482.95, 196832, 393660, 590492, 32110.45}, 
{ 11, 269936827.65, 590492, 1180980, 1771472, 18538.99}, 
{ 12, 89978942.55, 1771472, 3542940, 5314412, 10703.49}, 
{ 13, 29992980.85, 5314412, 10628820, 15943232, 6179.66}, 
{ 14, 9997660.28, 15943232, 31886460, 47829692, 3567.83}, 
{ 15, 3332553.43, 47829692, 95659380, 143489072, 2059.89}, 
{ 16, 1110851.14, 143489072, 286978140, 430467212, 1189.28}, 
{ 17, 370283.71, 430467212, 860934420, 1291401632, 686.63}, 
{ 18, 123427.9, 1291401632, 2582803260, 3874204892, 396.43}, 
{ 19, 41142.63, 3874204892, 7748409780, 11622614672, 228.88}, 
{ 20, 13714.21, 11622614672, 23245229340, 34867844012, 132.14}, 
{ 21, 4571.4, 34867844012, 69735688020, 104603532032, 76.29}, 
{ 22, 1523.8, 104603532032, 209207064060, 313810596092, 44.05}, 
{ 23, 507.93, 313810596092, 627621192180, 941431788272, 25.43}, 
{ 24, 169.31, 941431788272, 1882863576540, 2824295364812, 14.68}, 
{ 25, 56.44, 2824295364812, 5648590729620, 8472886094432, 8.48}, 
{ 26, 18.81, 8472886094432, 16945772188860, 25418658283292, 4.89}, 
{ 27, 6.27, 25418658283292, 50837316566580, 76255974849872, 2.83}, 
{ 28, 2.09, 76255974849872, 152511949699740, 228767924549612, 1.63}, 
{ 29, 0.7, 228767924549612, 457535849099220, 686303773648832, .94}, 
{ 30, 0.232, 686000000000000, 1370000000000000, 2060000000000000, .544}, 
{ 31, 0.0774, 2060000000000000, 4120000000000000, 6180000000000000, .314}, 
{ 32, 0.0258, 6180000000000000, 12400000000000000, 18500000000000000, .181}, 
{ 33, 0.0086, 18500000000000000, 37100000000000000, 55600000000000000, .105}, 
{ 34, 0.00287, 55600000000000000, 111000000000000000, 167000000000000000, .0604}, 
{ 35, 0.000956, 167000000000000000, 334000000000000000, 500000000000000000, .0349}, 
{ 36, 0.000319, 500000000000000000, 1000000000000000000, 1500000000000000000, .0201}, 
{ 37, 0.000106, 1500000000000000000, 3000000000000000000, 4500000000000000000, .0116}, 
{ 38, 0.0000354, 4500000000000000000, 9010000000000000000, 13500000000000000000, .00671}, 
// The following values are for reference only.... they contain constants that are too big for C++
//{ 39, 0.0000118, 13500000000000000000, 27000000000000000000, 40500000000000000000, .00388}, 
//{ 40, 0.00000393, 40500000000000000000, 81100000000000000000, 122000000000000000000, .00224}, 
//{ 41, 0.00000131, 122000000000000000000, 243000000000000000000, 365000000000000000000, .00129}, 
//{ 42, 0.000000437, 365000000000000000000, 729000000000000000000, 1090000000000000000000, .000746}, 
//{ 43, 0.000000146, 1090000000000000000000, 2190000000000000000000, 3280000000000000000000, .000431}, 
//{ 44, 0.0000000486, 3280000000000000000000, 6570000000000000000000, 9850000000000000000000, .000249}, 
//{ 45, 0.0000000162, 9850000000000000000000, 19700000000000000000000, 29500000000000000000000, .000144}, 
//{ 46, 0.0000000054, 29500000000000000000000, 59100000000000000000000, 88600000000000000000000, .0000829}, 
//{ 47, 0.0000000018, 88600000000000000000000, 177000000000000000000000, 266000000000000000000000, .0000479}, 
//{ 48, 0.0000000006, 266000000000000000000000, 532000000000000000000000, 798000000000000000000000, .0000276}, 
//{ 49, 0.0000000002, 798000000000000000000000, 1600000000000000000000000, 2390000000000000000000000, .000016}, 
//{ 50, 0.0000000001, 2390000000000000000000000, 4790000000000000000000000, 7180000000000000000000000, .00000921}, 
};

/*! Helper function to construct a sampler.  Returns a pointer to SamplerBase, to permit quick calls. 
*/
boost::intrusive_ptr<SamplerBase> CreateSampler( CLSID samplerClassId, boost::intrusive_ptr<IProcess> input)
{
	// Create a sampler
	boost::intrusive_ptr<IProcess> spSamplerProc;
	PYXCOMCreateInstance( samplerClassId, 0, IProcess::iid, (void**) &spSamplerProc);
	if (spSamplerProc == 0)
	{
		PYXTHROW( PYXException, "Unable to create an instance of process " << samplerClassId << ".");
	}

	spSamplerProc->getParameter(0)->addValue( 
		boost::dynamic_pointer_cast<IProcess>(input));

	boost::intrusive_ptr<PYXCOM_IUnknown> spUnk = spSamplerProc->getOutput();
	boost::intrusive_ptr<ICoverage> samplerOutput = boost::dynamic_pointer_cast<ICoverage>( spUnk);

	boost::intrusive_ptr<SamplerBase> result = boost::dynamic_pointer_cast<SamplerBase>( samplerOutput);
	return result;
}

/*! Determine what resolution to use in the dataset.  We also initialize the samplers in use.
*/
int ClampedSampler::getNativeResolution() const
{
	if (m_nativeResolution == -1)
	{
		double precision = this->getXYCoverage()->getSpatialPrecision();
		// A precision of -1 means unknown.
		if (precision < 0)
		{
			// Always use the nearest neighbour sampler.  This should never happen.
			m_nativeResolution = 40;
		}
		else
		{			 
			// Look for the resolution with a higher CLS than the data precision.  We
			// actually look for one that is twice as large as the source data, to
			// ensure a smooth transition.
			for (int index = 0; 
				index < sizeof(ResolutionTable)/sizeof(ResolutionTable[0]); 
				++index)
			{
				if (ResolutionTable[index].CLS < (2.0 * precision))
				{
					m_nativeResolution = ResolutionTable[index].Resolution;
					break;
				}
			}

			// We fell off the edge of the table.  This is surprisingly precise data!
			if (m_nativeResolution == -1)
			{
				m_nativeResolution = 38; 
			}
		}

		TRACE_INFO( "Data file " << this->getProcName() << " " <<
			procRefToStr( ProcRef( (IProcess *) this)) << " is using resolution " << 
			m_nativeResolution << " to sample " << precision << "m data.");

		// Create our samplers.  We could also do this in an init function....
		m_spOverSampler = CreateSampler( 
			BicubicSampler::clsid, getParameter(0)->getValue(0));
		m_spUnderSampler = CreateSampler( 
			NearestNeighbourSampler::clsid, getParameter(0)->getValue(0));
	}
	return m_nativeResolution;
}
