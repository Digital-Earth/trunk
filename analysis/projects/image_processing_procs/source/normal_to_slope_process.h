#ifndef NORMAL_TO_SLOPE_H
#define NORMAL_TO_SLOPE_H
/******************************************************************************
normal_to_rgb_process.h

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/coord_3d.h"

// standard includes
#include <cassert>
#include <vector>

/*!
This class acts as a process to convert surface normal values to rgb. In order to work properly
every field on the input must be a ContextNormal type.
*/
//! Greyscale to RGB coverage process.
class MODULE_IMAGE_PROCESSING_PROCS_DECL NormalToSlopeProcess : public ProcessImpl<NormalToSlopeProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

private:
	//! Controls the number of threads that will be used to run the calculation.
	static const int N_THREADS = 4;

public:

	//! Constructor
	NormalToSlopeProcess();

protected:
	//! Destructor
	virtual ~NormalToSlopeProcess();

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

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes in this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;	

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


	//! extract slope and direction and light source direction
	void extractSlopeAndDirectionAndLightDirection(
		const PYXCoord3DDouble & xyz,
		const PYXCoord3DDouble & normal,
		double * slope,
		double * direction) const;

	void calculatePartialTile (int firstIndex, int lastIndex,
							   const PYXIcosIndex & index,
							   PYXValueTile* spInputTile,
						       PYXValueTile* spOutputTile,
							   int nFieldIndex,int nRes) const;

	//! Convert a normal value to an RGB value.
	PYXValue convert(const PYXIcosIndex & index, const PYXValue& valIn, int nFieldIndex) const;

private:

	//! The input coverage.
	boost::intrusive_ptr<ICoverage> m_spCov;

	enum OutputType
	{
		knSlope,
		knAspect
	};

	OutputType m_outputType;
};

#endif // guard
