#ifndef SAMPLER_FACTORY_H
#define SAMPLER_FACTORY_H
/******************************************************************************
sampler_factory.h

begin		: 2006-03-21
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
//TODO: Come back to this!
// local includes
#include "pyxis/utility/app_services.h"
#include "nearest_neighbour_sampler.h"
#include "bicubic_sampler.h"
#include "bilinear_sampler.h"

// boost includes

// standard includes

// local forward declarations

/*!
PYXSamplerFactor creates an instance of the current sampling method. The
sampling method is specified int the properties file.
*/
//! Creates the currently selected sampler.
class PYXSamplerFactory
{

public:

	//! Create the appropriate sampler based on value in properties file.
	static PYXSampler* createObject()
	{
		const std::string kstrScope = "SamplerFactory";
		const std::string kstrKey = "SamplingMethod";
		const std::string kstrDefault = "NearestNeighbour";
		const std::string kstrDescription = "Method used to sample xy coverages into PYXIS (Available Methods: NearestNeighbour, Bicubic, Bilinear).";

		std::string strSampler = getAppProperty(
									kstrScope,
									kstrKey,
									kstrDefault,
									kstrDescription	);

		if (strSampler == "NearestNeighbour")
		{
			return new PYXNearestNeighbourSampler();
		}
		else if (strSampler == "Bicubic")
		{
			return new PYXBicubicSampler();
		}
		else if (strSampler == "Bilinear")
		{
			return new PYXBilinearSampler();
		}
		else
		{
			assert("Invalid sampler type stored in properties file.");
		}

		// use default if undefined
		return new PYXNearestNeighbourSampler();
	}

protected:

private:

	//! Disable constructor
	PYXSamplerFactory();
};

#endif	// PYX_SAMPLER_FACTORY_H
