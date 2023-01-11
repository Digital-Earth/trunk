#ifndef SAMPLER_BASE_H
#define SAMPLER_BASE_H
/******************************************************************************
sampler_base.h

begin		: 2007-03-12
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_sampling.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/region/xy_bounds_region.h"
#include "pyxis/data/value_tile.h"

//! A base class for samplers.
class MODULE_SAMPLING_DECL SamplerBase : public CoverageBase
{

public: // ICoverage

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		return getXYCoverage()->getCoverageDefinition();
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return getXYCoverage()->getCoverageDefinition();
	}

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex = 0	) const;

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(const PYXIcosIndex& index,
														int nFieldIndex = 0	) const;

	virtual void STDMETHODCALLTYPE geometryHint(PYXPointer<PYXGeometry> spGeom);

	virtual void STDMETHODCALLTYPE endGeometryHint(PYXPointer<PYXGeometry> spGeom);

private:

	//! The xy coverage being sampled.
	virtual boost::intrusive_ptr<IXYCoverage> getXYCoverage() const = 0;

	/*!
	Fill in a PYXValue with the coverage value for the specified PYXIS index.  The sampler should 
	implement this form of getCoverageValue and let the other form use this form as it is 
	implemented in the SamplerBase code.

	\param	index		The PYXIS index.
	\param  pValue      an pointer to the PYXValue to be filled in
	\param	nFieldIndex	The field index.

	\return	True if a data value was retrieved, false if there was no value to return.
	*/
	//! Fill in a PYXValue with the coverage value at the specified index.
	virtual bool getCoverageValue(const PYXIcosIndex& index,
									PYXValue* pValue,
									int nFieldIndex) const = 0;

	virtual bool generateCoverageValue(const PYXIcosIndex & index,
										const PYXCoord2DDouble & nativeCoord,
										bool * hasValues,
										PYXValue * values,
										int width,int height,
										PYXValue * resultValue) const = 0;

	virtual int getSamplingMatrixSize() const = 0;

protected:
	PYXPointer<PYXXYBoundsRegion> getXYRegion() const {
		if (!m_spRegion)
			createGeometry();

		return m_spRegion;
	}

private:
	mutable PYXPointer<PYXXYBoundsRegion> m_spRegion;

	void createGeometry() const;

	class ValueTileConsumer : public XYAsyncValueConsumer
	{
	private:
		PYXValue m_nullVal;
		PYXValue m_compatibleValue;
		boost::scoped_array<PYXValue> m_values;
		PYXPointer<PYXValueTile> m_spValueTile;
		const SamplerBase & m_sampler;
		mutable boost::recursive_mutex m_mutex;

	public:
		virtual void onRequestCompleted(const PYXIcosIndex & index,
										const PYXCoord2DDouble & nativeCoord,
										bool * hasValues,
										PYXValue * values,
										int width,int height) const
		{
			PYXValue finalVal(m_compatibleValue);

			if (hasValues != 0 && m_sampler.generateCoverageValue(index,nativeCoord,hasValues,values,width,height,&finalVal))
			{
				//optimization: move the finalValue into m_values - avoid assignment
				//m_values[PYXIcosMath::calcCellPosition(m_spValueTile->getTile().getRootIndex(), index)] = finalVal;
				swap(m_values[PYXIcosMath::calcCellPosition(m_spValueTile->getTile().getRootIndex(), index)],finalVal);
			}
			else 
			{
				//do nothing..
			}
		}

		ValueTileConsumer (const SamplerBase & sampler,const PYXPointer<PYXValueTile> & spValueTile) : m_spValueTile(spValueTile), m_sampler(sampler)
		{
			m_compatibleValue = m_spValueTile->getTypeCompatibleValue(0);
			m_values.reset(new PYXValue[m_spValueTile->getNumberOfCells()]);
		}

		void copyValuesToValueTile()
		{
			int count = m_spValueTile->getNumberOfCells();
			for(int i=0;i<count;i++)
			{
				m_spValueTile->setValue(i,0,m_values[i]);
			}
		}
	};
	

	friend class ClampedSampler;
	friend class ClampedSamplerWithNull;
};

#endif // guard