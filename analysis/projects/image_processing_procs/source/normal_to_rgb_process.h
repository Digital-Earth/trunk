#ifndef NORMAL_TO_RGB_PROCESS_H
#define NORMAL_TO_RGB_PROCESS_H
/******************************************************************************
normal_to_rgb_process.h

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "normal_to_rgb_process.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/coord_3d.h"

// standard includes
#include <cassert>
#include <vector>

//! Helper class to covert a position/angle to an rgb values
class Palette : public PYXObject
{
public:
	virtual void convert(double position,uint8_t * rgb) = 0;
};

/*!
This class acts as a process to convert surface normal values to rgb. In order to work properly
every field on the input must be a ContextNormal type.
*/
//! Greyscale to RGB coverage process.
class MODULE_IMAGE_PROCESSING_PROCS_DECL NormalToRGBProcess : public ProcessImpl<NormalToRGBProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

private:
	//! Controls the number of threads that will be used to run the calculation.
	static const int N_THREADS = 4;

public:

	//! Constructor
	NormalToRGBProcess();

protected:
	//! Destructor
	virtual ~NormalToRGBProcess();

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
		double * direction, 
		PYXCoord3DDouble * lightDirection) const;

	//! extract light factors
	void cacluateLightFactors();

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

	//! the palette to use
	std::string m_paletteName;

	//! pointer to the right palette obhect
	PYXPointer<Palette> m_palette;

	//! vertical angle of the light source
	int m_lightAngle;

	//! azimuth of the light source
	int m_lightAzimuth;

	enum eShadingMode
	{
		knShadeSlope,
		knShadeDirection,
		knShadeSlopeAndDirection,
		knHillShade
	};

	eShadingMode m_shadingMode;

	//speed up light source finding
	double m_lightSourceEastFactor;
	double m_lightSourceNorthFactor;
	double m_lightSourceForwadFactor;
	double m_lightSourceUpFactor;
};


//! Grayscale palette
class GrayscalePalette : public Palette
{
public:
	GrayscalePalette() {};
	static PYXPointer<GrayscalePalette> create() { return PYXNEW(GrayscalePalette); }
 	~GrayscalePalette() {};
	virtual void convert(double position,uint8_t * rgb);
};

//! HSV palette
class HSVPalette : public Palette
{
public:
	HSVPalette() {};
	static PYXPointer<HSVPalette> create() { return PYXNEW(HSVPalette); }
	~HSVPalette() {};
	virtual void convert(double position,uint8_t * rgb);
};

//! Green to red palette
class GreenToRedPalette : public Palette
{
public:
	GreenToRedPalette() {};
	static PYXPointer<GreenToRedPalette> create() { return PYXNEW(GreenToRedPalette); }
	~GreenToRedPalette() {};
	virtual void convert(double position,uint8_t * rgb);
};

#endif // guard
