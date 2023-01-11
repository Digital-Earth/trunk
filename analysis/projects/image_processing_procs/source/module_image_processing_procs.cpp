/******************************************************************************
module_image_processing_procs.cpp

begin		: 2007-06-26
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "module_image_processing_procs.h"

#include "band_pass_filter.h"
#include "blur_process.h"
#include "calculator_process.h"
#include "channel_selector_process.h"
#include "coverage_mask_process.h"
#include "coverage_geometry_mask_process.h"
#include "greyscale_to_rgb_process.h"
#include "hillshade_process.h"
#include "multi_res_spatial_analysis_process.h"
#include "zoom_in_process.h"
#include "zoom_out_process.h"
#include "elevation_to_normal_process.h"
#include "normal_to_rgb_process.h"
#include "normal_to_slope_process.h"
#include "coverage_transfrom_process.h"
#include "watershed_process.h"
#include "rgb_to_value_process.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BandPassFilter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BlurProcess), // TODO: Drop
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(Calculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ChannelSelectorProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CoverageMaskProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CoverageGeometryMaskProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GreyscaleToRGBProcess), // TODO: Drop
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(HillShader),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(MultiResSpatialAnalysisProcess), // TODO: Drop
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PYXZoomInProcess), // TODO: Drop
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PYXZoomOutProcess), // TODO: Drop
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(NormalToRGBProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ElevationToNormalProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(NormalToSlopeProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CoverageValuesTransformProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(RgbToValueProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(WatershedProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(WatershedFlowProcess)
PYXCOM_END_CLASS_OBJECT_TABLE