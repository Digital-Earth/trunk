/******************************************************************************
module_sampling.cpp

begin		: 2007-02-19
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "module_sampling.h"

#include "bicubic_sampler.h"
#include "bicubic_sampler_with_null.h"
#include "bilinear_sampler.h"
#include "clamped_sampler.h"
#include "clamped_sampler_with_null.h"
#include "nearest_neighbour_sampler.h"
#include "xy_coverage_translator.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BicubicSamplerWithNull),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BicubicSampler),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BilinearSampler),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ClampedSampler),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ClampedSamplerWithNull),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(NearestNeighbourSampler),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(XYCoverageTranslatorProcess),
PYXCOM_END_CLASS_OBJECT_TABLE
