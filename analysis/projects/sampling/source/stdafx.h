#define MODULE_SAMPLING_SOURCE

#include "bicubic_sampler.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>
