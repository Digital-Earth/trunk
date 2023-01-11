#pragma once
/******************************************************************************
clamped_sampler_with_null.h

begin		: 2015-nov-9
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_sampling.h"

// local includes
#include "sampler_base.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

/*! 
 * The clamped sampler is a sampling process that pays attention to the native 
 * data resolution.  It behaves differently for data at or above the native resolution 
 * (it decimates) than it does for higher resolution requests (which it interpolates 
 * through a bicubic sampler.)
 *
 * See http://euclid:9000/WorldView/ticket/2353
*/
//! A process for clamped sampling.
class MODULE_SAMPLING_DECL ClampedSamplerWithNull : public ProcessImpl<ClampedSamplerWithNull>, public SamplerBase
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

public: // implementation

	ClampedSamplerWithNull();

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex = 0	) const;
private:

	//! Native resolution of the input.  0 is unknown, -1 is uninitialized.
	mutable int m_nativeResolution;

	int getNativeResolution() const;

	//! The sampler to use when we are over sampling (resolution > native)
	mutable boost::intrusive_ptr<SamplerBase> m_spOverSampler;

	//! The sampler to use when we are under sampling (resolution <= native)
	mutable boost::intrusive_ptr<SamplerBase> m_spUnderSampler;

	//! Returns the sampler responsible for the given resolution.
	boost::intrusive_ptr<SamplerBase> getActualSampler( int nResolution) const
	{
		if (m_nativeResolution == -1)
		{
			m_nativeResolution = getNativeResolution();
		}

		if (nResolution <= m_nativeResolution)
		{
			return m_spUnderSampler;
		}
		return m_spOverSampler;
		
	}

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
