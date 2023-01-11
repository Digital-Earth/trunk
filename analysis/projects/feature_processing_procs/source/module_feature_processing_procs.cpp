/******************************************************************************
module_feature_processing_procs.cpp

begin		: 2008-01-23
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "attribute_query.h"
#include "equals.h"
#include "feature_collection_calculator.h"
#include "resolution_filter.h"
#include "styled_feature_collection_process.h"
#include "intersects.h"
#include "intersection.h"
#include "module_feature_processing_procs.h"
#include "point_aggregator_process.h"
#include "union.h"
#include "bitmap_process.h"
#include "bitmap_crop_process.h"
#include "bitmap_grid_process.h"
#include "icon_style_feature_collection_process.h"
#include "line_style_feature_collection_process.h"
#include "area_style_feature_collection_process.h"
#include "behaviour_feature_collection_process.h"
#include "features_summary.h"
#include "features_summary_filter.h"
#include "styled_features_summary.h"
#include "concat_features.h"
#include "constant_feature_calculator.h"
#include "count_feature_calculator.h"
#include "modify_feature_properties_process.h"
#include "aggregate_feature_calculator.h"
#include "aggregate_coverage_calculator.h"
#include "coverage_histogram_calculator.h"
#include "select_feature_by_id.h"
#include "geotag_record_collection.h"
#include "first_not_null_geometry_provider.h"
#include "feature_collection_geometry_provider.h"
#include "point_geometry_provider.h"


PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(AttributeQuery),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(Equals),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureCollectionCalculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureCollectionResolutionFilter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(StyledFeatureCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(Intersection),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(Intersects),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PointAggregatorProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(Union),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BitmapProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BitmapCropProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BitmapGridProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(IconStyleFeatureCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(LineStyleFeatureCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(AreaStyleFeatureCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(BehaviourFeatureCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeaturesSummary),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeaturesSummaryAttributeRangeFilter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeaturesSummaryGeometryFilter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(StyledFeaturesSummaryProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ConcatFeatures),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ConstantFieldCalculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CountFeatureCalculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ModifyFeaturePropertiesProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(AggregateFeatureCalculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CoverageHistogramCalculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(AggregateCoverageCalculator),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(SelectFeatureByIdProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GeotagRecordCollection),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureCollectionGeometryProvider),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FirstNotNullGeometryProvider),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PointGeometryProvider)

PYXCOM_END_CLASS_OBJECT_TABLE
