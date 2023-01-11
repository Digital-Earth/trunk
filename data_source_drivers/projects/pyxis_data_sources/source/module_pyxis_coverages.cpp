/******************************************************************************
module_pyxis_coverages.cpp

begin		: 2007-03-09
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE
#include "module_pyxis_coverages.h"

#include "checker_coverage.h"
#include "feature_collection_process.h"
#include "georeferenced_file.h"
#include "null_coverage.h"
#include "coverage_cache.h"
#include "pyxis_pipe_builder.h"
#include "viewpoint_process.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CheckerCoverage),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(FeatureCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GeoRerefencedFileProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(NullCoverage),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PYXCoverageCache),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(PyxisPipeBuilder),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ViewPointProcess)
PYXCOM_END_CLASS_OBJECT_TABLE
