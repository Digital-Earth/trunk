#ifndef GRIB_WGRIB_H
#define GRIB_WGRIB_H
/******************************************************************************
wgrib.h

begin		: 2006-03-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "grib.h"

// standard includes
#include <vector>
#include <string>

// forward declarations
class GribRecord;

/*!
Read the record whose file name and record number are in the pRecord item.

\param pRecord	Pointer to a GribRecord whose values will be updated with the read data.

\return Zero if record was read ok, negative if an error.
*/
int openGRIB(GribRecord* pRecord);

int getGRIBInfo(const std::string& strFileName, std::vector<GribRecord>& vecRecords);

double getUndefinedValue();

#endif // guard
