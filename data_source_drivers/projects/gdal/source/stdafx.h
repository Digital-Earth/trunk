#define MODULE_GDAL_SOURCE

// local includes
#include "exceptions.h"
#include "gdal_xy_coverage.h"
#include "gdal_pipe_builder.h"
#include "gdal_file_process.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/srs.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// GDAL includes
#include "cpl_string.h"
#include "gdal_priv.h"

// boost includes
#include <boost/filesystem/path.hpp>

// standard includes
#include <algorithm>
#include <cassert>
