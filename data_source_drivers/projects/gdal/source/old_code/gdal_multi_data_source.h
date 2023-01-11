#ifndef GDAL_MULTI_DATA_SOURCE_H
#define GDAL_MULTI_DATA_SOURCE_H
/******************************************************************************
gdal_multi_data_source.h

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_gdal.h"
#include "coord_converter_impl.h"
#include "pyx_rtree.h"

// pyxlib includes
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"
#include "pyxis/utility/coord_2d.h"

// standard includes
#include <map>
#include <memory>

// local forward declarations
class GDALDataSource;
class PYXSpatialReferenceSystem;

/*!
The GDALMultiDataSource contains logic for loading multiple similar GIS 
files and returning data values for given cells.  It utilizes the PYXDataSource
interface.  The GDAL library is used to read the files. GDALMultiDataSource works
in the native coordinates of the GDAL files that it s reading.
*/
//! Reads a collection of similar GIS Data Files utilizing the GDAL library.
class GDALMultiDataSource : public PYXXYCoverage
{

public:

	//! Test method
	static void test();

	//! Create method
	static PYXPointer<GDALMultiDataSource> create()
	{
		return PYXNEW(GDALMultiDataSource);
	}

	//! Destructor.
	virtual ~GDALMultiDataSource();

	/*!
	Open the data files located in the directory(s) specified.
	\return	Pointer to first GDALDataSet in multiset.
	*/
	virtual bool open(	const std::string& strDir,
						const std::string& strFileExt);

	//! Return true if we have a spatial reference system.
	bool hasSpatialReferenceSystem() const;

	//! Set the spatial reference system.
	void setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS);

	//! Get the spatial precision of the data in metres.
	virtual double getSpatialPrecision() const;

	//! Get the number of files opened by this reader.
	int getNumFiles() const {return static_cast<int>(m_mapDataSource.size());}

	//! Get the bounds of the data set in native coordinate.
	virtual const PYXRect2DDouble& getBounds() const {return m_bounds;}

	//! Get the distance between data points in this coverage
	virtual const PYXCoord2DDouble getStepSize() const;

	//! Get the coverage value at the specified native coordinates (thread-safe).
	virtual PYXValue getCoverageValue(
		const PYXCoord2DDouble& native,
		int nFieldIndex = 0	) const;

	//! Get matrix of field values around the specified native coordinate.
	virtual void getMatrixOfAttrValues(	const PYXCoord2DDouble& nativeCentre,
										PYXValue* pValues,
										int sizeX,
										int sizeY,
										int nFieldIndex = 0) const;

	//! Returns a coordinate converter used to create a geometry. (ownership retained)
	virtual const CoordConverterImpl* getCoordConverter() const;

private:

	//! Hide constructor
	GDALMultiDataSource();

	//! Disable copy constructor.
	GDALMultiDataSource(const GDALMultiDataSource&);

	//! Disable copy assignment.
	void operator=(const GDALMultiDataSource&);

	//! Open the data files located in the specified directory.
	int openFiles(const std::string& strDir, const std::string& strFileExt);

	//! Open the file.
	bool openFile(const std::string& strFileName);

	//! Get the reader based on the native coordinate.
	const GDALDataSource* getDataSource(const PYXCoord2DDouble& native) const;

	//! Map of GDALDataSources.
	typedef std::map<PYXCoord2DDouble, PYXPointer<GDALDataSource> > GDALDataSourceMap;

	/*!
	Map of GDALReaders for this data source sorted by increasing longitude then
	latitude.
	*/
	mutable GDALDataSourceMap m_mapDataSource;

	//! The bounds in native coordinates
	PYXRect2DDouble m_bounds;

	//! The rtree to store file coordintates.
	mutable PYXrTree m_rTree;
};

#endif	// GDAL_MULTI_DATA_SOURCE_H
