#ifndef PYXILIZATION_STYLE_MAPPER_H
#define PYXILIZATION_STYLE_MAPPER_H

/******************************************************************************
pyxilization_style_mapper.h

begin		: 2007-02-11
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyx_geometry.h"
#include "string_utils.h"
#include "style_mapper.h"
#include "pyx_cell.h"
#include "pyx_feature.h"

class PyxilizationFeature;

// boost includes
#include <boost/shared_ptr.hpp>

// standard includes
#include <cassert>
#include <map>
#include <vector>



class PyxilizationStyleMapper: public StyleMapper
{
	public:

		FeatureStyle::FeatureStyleConstPtr mapFeature(boost::shared_ptr<const PYXFeature> spFeature) const;

		PyxilizationStyleMapper(){}

		//! lets the style mapper know what resolution is being displayed right now
		void static setDisplayResolution(int nDisplayResolution)
		{
			m_nDisplayResolution = nDisplayResolution;
		}

		//! find out what resolution is being displayed
		int static getDisplayResolution(void)
		{
			return m_nDisplayResolution;
		}

		static int m_nDisplayResolution;
};

#endif