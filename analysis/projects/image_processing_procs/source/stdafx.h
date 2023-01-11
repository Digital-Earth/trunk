#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "module_image_processing_procs.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/data/coverage_base.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/data_processor.h"
#include "pyxis/procs/string.h"
#include "pyxis/utility/string_utils.h"