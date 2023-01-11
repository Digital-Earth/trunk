/******************************************************************************
excel_feature.cpp

begin      : 15/11/2007 9:56:19 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define EXCEL_SOURCE
#include "excel_feature.h"

// local includes
#include "exceptions.h"

// pyxis library includes
#include "pyxis/derm/coord_converter.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/region/vector_point_region.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/coord_2d.h"

// standard includes
#include <cassert>
#include <memory>

//! Constructor
ExcelFeature::ExcelFeature(
	PYXPointer< PYXTableDefinition > spDefn,
	PYXPointer< PYXVectorPointRegion const > spRegion,
	PYXPointer< PYXGeometry > spGeometry,
	std::vector< PYXValue > const & values,
	size_t nID) :
m_spDefn(spDefn),
m_nID(nID),
m_strID(StringUtils::toString(nID)),
m_spRegion(spRegion),
m_spGeometry(spGeometry),
m_vecValue(values),
m_featureStyle()
{
	assert(this->m_spRegion);
	assert(this->m_spGeometry);
	
	if (this->m_spDefn->getFieldCount() != this->m_vecValue.size())
	{
		// it should never be constructed this way
		PYXTHROW(FeatureException, "Field count does not match the value count");
	}
}

PYXPointer< IRegion const > ExcelFeature::getRegion() const
{
	return m_spRegion;
}
