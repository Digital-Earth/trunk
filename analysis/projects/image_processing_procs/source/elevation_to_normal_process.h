#ifndef ELEVATION_TO_NORMAL_PORCESS_H
#define ELEVATION_TO_NORMAL_PORCESS_H
/******************************************************************************
elevation_to_normal_process.h

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_image_processing_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/derm/snyder_projection.h"

// standard includes
#include <cassert>
#include <vector>

/*!
This class acts as a process to convert elevation values into a normal vector values in 3D space
Every field on the input must be a Elevation type.
*/
//! Elevation to Normal coverage process.
class MODULE_IMAGE_PROCESSING_PROCS_DECL ElevationToNormalProcess : public ProcessImpl<ElevationToNormalProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

private:
	//! Controls the number of threads that will be used to run the calculation.
	static const int N_THREADS = 4;

public:

	//! Constructor
	ElevationToNormalProcess();

protected:
	//! Destructor
	virtual ~ElevationToNormalProcess();

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

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(		const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex = 0	) const;

public:

	static void test();

private:

	virtual void createGeometry() const;
	
	void calculatePartialTile (int firstIndex, int lastIndex,
							   const PYXIcosIndex & index,
							   PYXValueTile* spInputTile,
						       PYXValueTile* spOutputTile,
							   int nFieldIndex,int nRes) const;

	inline bool safeFetchValue(PYXValue * pValue,const PYXIcosIndex & root,const PYXIcosIndex & index,const PYXPointer<PYXValueTile> & valueTile,int nFieldIndex) const;

#ifdef FAST_NORMAL

	PYXValue findNormalFast(const PYXIcosIndex & root,
							const PYXCoord3DDouble & aDelta, const PYXValue& aElevation,
							const PYXCoord3DDouble & bDelta, const PYXValue& bElevation,
							const PYXCoord3DDouble & cDelta, const PYXValue& cElevation) const;

	//! Helper function that calculate the relative 3D delta of the tile
	void calcDelta(const PYXIcosIndex & root,PYXCoord3DDouble & aDelta,PYXCoord3DDouble & bDelta,PYXCoord3DDouble & cDelta) const;

#endif

	PYXValue findNormal(const PYXIcosIndex & a,const PYXValue& aElevation,
						const PYXIcosIndex & b,const PYXValue& bElevation,
						const PYXIcosIndex & c,const PYXValue& cElevation
						) const;

	
	
private:

	//! pointer to current synder pojection
	const SnyderProjection * m_pSnyder;

	//! The input coverage.
	boost::intrusive_ptr<ICoverage> m_spCov;	

	mutable boost::recursive_mutex m_inputCovMutex;
};

#endif // guard
