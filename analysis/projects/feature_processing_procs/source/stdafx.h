#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "attribute_query.h"
#include "exceptions.h"

#include "feature_collection_process.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/geometry/multi_geometry.h"
#include "pyxis/procs/default_feature.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/xml_transform.h"

// standard includes
#include <cassert>
