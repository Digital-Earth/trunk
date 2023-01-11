/******************************************************************************
grib_dataset_reader.cpp

begin		: 2006-02-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define GRIB_SOURCE
#include "grib_dataset_reader.h"

// local includes

// pyxis library includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/feature.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/rgb.h"
#include "pyxis/utility/value.h"

// standard includes
#include <iostream>

// GRIB includes
#include "wgrib.h"

// standard includes
#include <cassert>

/*!
Constructor.
*/
GRIBDatasetReader::GRIBDatasetReader() :
	m_fNullValue(9.999e20)
{
}

/*!
Destructor deletes the GRIBDataset object.
*/
GRIBDatasetReader::~GRIBDatasetReader()
{
}

/*!
Open the dataset.

\param path		URI of the data source.
\param nRecord	Number of image (starting at 1) within the file to initially open.
*/
bool GRIBDatasetReader::open(const boost::filesystem::path& path, int nRecord)
{
	TRACE_INFO("Reading record '" << nRecord << "' of GRIB file '" << FileUtils::pathToString(path) << "'.");

	// Remove any data currently associated with the reader.
	m_gribRecord.getDataVector().clear();
	m_spMemoryToken.reset();

	m_gribRecord.setFileName(FileUtils::pathToString(path));
	m_gribRecord.setRecordNumber(nRecord);

	// Read data for our record.
	return openFile();
}

/*!
Get the value for the given raster coordinates.

\param raster	The raster coordinates
\param  pValue      an pointer to the PYXValue to be filled in

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool GRIBDatasetReader::getValue(const PYXCoord2DInt& raster, PYXValue* pValue) const
{
	/*
	Data buffer is empty (memory manager asked us to free memory
	re-read data from file.
	*/
	if (m_gribRecord.getDataVector().empty())
	{
		if (!openFile())
		{
			PYXTHROW(
				PYXDataException,
				"Unable to re-open grib data source '" 
				<< m_gribRecord.getFileName() << "'.");
		}
	}

	int nX = raster.x();
	int nY = raster.y();

	// determine if the value is within the bounds
	if (	nX < 0 || m_gribRecord.getBandXSize() <= nX || 
			nY < 0 || m_gribRecord.getBandYSize() <= nY	)
	{
		// out of bounds
		return false;
	}

	int nIndex = 0;

	// If data is stored from bottom to top (the norm).
	if (m_gribRecord.isBottomUp())
	{
		// Data stored from bottom to top.  Get left start of line.
		nIndex = nY * m_gribRecord.getBandXSize() + nX;
	}
	else
	{
		// Data stored from top to bottom.  Get left start of line.
		nIndex = ((m_gribRecord.getBandYSize() - 1) - nY) * m_gribRecord.getBandXSize() + nX;
	}

	// Return data from array (or null if appropriate)
	double fValue = m_gribRecord.getDataVector()[nIndex];
	if (fValue != m_fNullValue)
	{
		pValue->setDouble(fValue);
		return true;
	}
	else
	{
		return false;
	}
}

/*!
Memory manager calls this function to free memory when we are running low.
*/
void GRIBDatasetReader::freeMemory()
{
	std::vector<double> vecEmpty;

	// Only to remove allocated memory from the vector.
	m_gribRecord.getDataVector().swap(vecEmpty);
	m_spMemoryToken.reset();
}

/*!
Open an existing record
*/
bool GRIBDatasetReader::openFile() const
{
	int nResult;
	try
	{
		// Open file and get bounds, grid size and min/max values.
		nResult = openGRIB(&m_gribRecord);
	}
	catch(...)
	{
		fflush(stderr);
		return false;
	}

	// If we were able to open the file - request memory token.
	if (0 < nResult)
	{
		int nMemorySize =	m_gribRecord.getBandXSize() *
							m_gribRecord.getBandYSize() * 
							sizeof(double);
		m_spMemoryToken.reset(MemoryManager::getInstance()->requestToken(nMemorySize));

		if (m_spMemoryToken.get() != 0)
		{
			return true;
		}
		else
		{
			TRACE_ERROR("Unable to allocate memory for GRIB record.");
		}
	}

	TRACE_ERROR("Unable to open GRIB data source '" << m_gribRecord.getFileName() << "'.");
	return false;
}
