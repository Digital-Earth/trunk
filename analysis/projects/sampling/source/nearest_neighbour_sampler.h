#ifndef NEAREST_NEIGHBOUR_SAMPLER_H
#define NEAREST_NEIGHBOUR_SAMPLER_H
/******************************************************************************
nearest_neighbour_sampler.h

begin		: 2006-03-21
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_sampling.h"

// local includes
#include "sampler_base.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

/*!
*/
//! A process for nearest neighbour sampling coverage.
class MODULE_SAMPLING_DECL NearestNeighbourSampler : public ProcessImpl<NearestNeighbourSampler>, public SamplerBase
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}


	IProcess::eInitStatus initImpl();

public: // implementation

	NearestNeighbourSampler() :
		m_pCoordConverter(0)
	{
		m_strID = "Nearest Neighbour Sampler: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	}
		
private:

	//! The coverage being sampled.
	boost::intrusive_ptr<IXYCoverage> m_spXYCov;

	//! The xy coverage being sampled is the first input of the first parameter.
	virtual boost::intrusive_ptr<IXYCoverage> getXYCoverage() const
	{
		if (!m_spXYCov.get())
		{
			getParameter(0)->getValue(0)->getOutput()->QueryInterface(IXYCoverage::iid, (void**) &m_spXYCov);
		}

		return m_spXYCov; 
	}

	virtual bool getCoverageValue(const PYXIcosIndex& index,
									PYXValue* pValue,
									int nFieldIndex) const;

	virtual bool generateCoverageValue(const PYXIcosIndex & index,
										const PYXCoord2DDouble & nativeCoord,
										bool * hasValues,
										PYXValue * values,
										int width,int height,
										PYXValue * resultValue) const;

	virtual int getSamplingMatrixSize() const;

	//! Cache the coord converter.
	mutable ICoordConverter* m_pCoordConverter;

	//! Get the coord converter.
	const ICoordConverter* getCoordConverter() const
	{
		if (m_pCoordConverter == 0)
		{
			m_pCoordConverter = const_cast<ICoordConverter*>(getXYCoverage()->getCoordConverter());
		}
		return m_pCoordConverter; 
	}

};

#endif // guard
