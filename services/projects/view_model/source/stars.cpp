
// Data from http://local.wasp.uwa.edu.au/~pbourke/modelling_rendering/starfield/
// Collated from Bright Star Catalogue, 5th Revised Ed. by Paul Bourke
// Further processed by mlepage

#include "stdafx.h"
#include "stars.h"

C4UB_V3F stardata[] =
{
#pragma warning(push)
#pragma warning(disable: 4305) // warning C4305: 'initializing' : truncation from 'double' to 'float'
#include "star_data.txt"
#pragma warning(pop)
};

int starcount = sizeof(stardata)/sizeof(stardata[0]);
