#ifndef GRIB_DATASET_READER_H
#define GRIB_DATASET_READER_H
/******************************************************************************
grib_dataset_reader.h

begin		: 2006-03-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "grib.h"
#include "grib_record.h"

// pyxis library includes
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/memory_manager.h"

// standard includes
#include <vector>
#include <string>


// boost includes
#include <boost/filesystem/path.hpp>

// local forward declarations
class PYXValue;

/*!
The GRIBDatasetReader is used to read data from GRIB (Gridded Binary)
data files. The reader can only read from a single record within
the file at any one time. This reader currently only handles GRIB version 1.
*/
//! Reads raw raster weather raw data.
class GRIBDatasetReader : public MemoryConsumer
{

public:
	
	//! Constructor.
	GRIBDatasetReader();

	//! Destructor.
	~GRIBDatasetReader();

	//! Open the dataset.
	bool open(const boost::filesystem::path& path, int nRecord);

	//!	Get the data value for the given raster coordinates.
	bool getValue(const PYXCoord2DInt& raster, PYXValue* pValue) const;

	//! Called my memory manager when requesting memory to be freeded.
	virtual void freeMemory();

	//! Return the record associated with the reader.
	const GribRecord& getRecord() const {return m_gribRecord;}

	//! Return the description of the record within the GRIB file.
	const std::string getDescription() const {return m_gribRecord.getDescription();}
    
private:

	//! Disable copy constructor
	GRIBDatasetReader(const GRIBDatasetReader&);

	//! Disable copy assignment.
	void operator=(const GRIBDatasetReader&);

	//! Determine the data information.
	void determineDataInfo();

	//! Verify that all bands in the data set are compatible
	void verifyCompatibleBands();

	//! Open the data file.
	bool openFile() const;

private:

	//! Structure to hold our grib record data.
	mutable GribRecord m_gribRecord;

	//! The value that is returned when no data is present.
	double m_fNullValue;

	//! Token for memory manager.
	mutable std::auto_ptr<MemoryToken> m_spMemoryToken;
};

#endif
