/******************************************************************************
calculator_functions.cpp

begin		: 2013-12-20
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "calculator_functions.h"
#include "pyxis/derm/point_location.h"

#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE

double Dist_FunctionNode::DoEvaluate()
{
	auto p1 = PointLocation::fromWGS84(m_nodes[0]->Evaluate(), m_nodes[1]->Evaluate());
	auto p2 = PointLocation::fromWGS84(m_nodes[2]->Evaluate(),m_nodes[3]->Evaluate());
	return p1.distance(p2);
}