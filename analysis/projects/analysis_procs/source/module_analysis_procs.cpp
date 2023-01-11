/******************************************************************************
module_analysis_procs.cpp

begin		: 2007-04-26
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "module_analysis_procs.h"

#include "blender.h"
#include "channel_combiner.h"
#include "colourizer.h"
#include "feature_colourizer.h"
#include "feature_coverage_calculator.h"
#include "feature_rasterizer.h"
#include "first_non_null.h"
#include "resolution_filter.h"
#include "styled_feature_rasterizer.h"
#include "feature_field_rasterizer.h"
#include "feature_field_rasterizer2.h"
#include "styled_coverage.h"
#include "feature_collection_filter.h"
#include "feature_condition_calculator.h"



PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(Blender),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ChannelCombiner),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(Colourizer),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureColourizer),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureCoverageCalculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureRasterizer),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FirstNonNull),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ResolutionFilter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(StyledFeatureRasterizer),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(StyledCoverage),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureFieldRasterizer),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureFieldRasterizer2),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureCollectionFilter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureConditionCalculator)
PYXCOM_END_CLASS_OBJECT_TABLE
