#ifndef WMS_DATA_SOURCE_H
#define WMS_DATA_SOURCE_H
/******************************************************************************
wms_data_source.h

begin		: 2004-06-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "coord_converter_impl.h"
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"
#include "coord_converter_impl.h"
#include "gdal_dataset_reader.h"
#include "gdal_iterator.h"
#include "gdal_metadata.h"

#include "pyxis/sampling/spatial_reference_system.h"

#include <boost/shared_ptr.hpp>
// GDAL includes
#include "ogr_spatialref.h"

// standard includes

// local forward declarations
class ProjectionMethod;
class PYXGeometry;
//TODO: Code clean up after changest tested. CHOWELL
/*!
The WMSDataSource contains logic for loading a GIS file through GDAL (Geospatial
Data Abstraction Layer). The GDAL reader works in the native geospatial
coordinates of its GIS file. These may be projected (e.g. UTM) or un-projected
(e.g. WGS84).
*/
//! Reads a GIS file through GDAL
class WMSDataSource : public PYXXYCoverage
{
public:

	//! Test method.
	static void test();

	static PYXPointer<WMSDataSource> create()
	{
		return PYXNEW(WMSDataSource);
	}

	//! Constructor.
	WMSDataSource();

	//! Destructor.
	virtual ~WMSDataSource();

	//! Open the data file.
	virtual bool open(	
		const std::string& strFileName, 
		bool bCreateGeometry = true	);

	//! Get an iterator to all the points in the data source.
	virtual PYXPointer<PYXXYIterator> getXYIterator() const;

	//! Return true if the reader has a spatial reference system.
	virtual bool hasSpatialReferenceSystem() const { return m_bHasSRS; }

	//! Set spatial reference system for the reader.
	virtual void setSpatialReferenceSystem(boost::shared_ptr<PYXSpatialReferenceSystem> spSRS);

	//! Get the spatial precision of the data in metres.
	virtual double getSpatialPrecision() const;

	//! Get the bounds of the data set in native coordinate.
	virtual const PYXRect2DDouble& getBounds() const {return m_bounds;}

	//! Get the bounds of the data set in native coordinate.
	const void setBounds(PYXRect2DDouble& bounds) {m_bounds = bounds;}

	//! Get the distance between data points in this coverage
	virtual const PYXCoord2DDouble getStepSize() const;

	//! Get the value at the specified native raster coordinate.
    PYXValue getValue(const PYXCoord2DInt& raster) const;

	//! Get the coverage value at the specified native coordinate (thread-safe).
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
	virtual const CoordConverterImpl* getCoordConverter() const { return &m_coordConverter; }

	//! Get pointer to the GDALDataset inside the reader.
	GDALDataset* getDataset() { return m_pDataset; }

	//! Returns a coordinate converter used to create a geometry. (ownership retained)
	CoordConverterImpl* getCoordConverter() { return &m_coordConverter; }

	//! Return open state (1 = open, 2 = downloading, 3 = unavailable)
	int getOpenState() { return m_nOpenState; }

	//! Set open state (1 = open, 2 = downloading, 3 = unavailable)
	void setOpenState(int nOpenState) { m_nOpenState = nOpenState; }

private:

	//! Disable copy constructor.
	WMSDataSource(const WMSDataSource&);

	//! Disable copy assignment.
	void operator=(const WMSDataSource&);

	//! Continue reading file after open and SRS has been set.
	void readFile();

	//! Get a pointer to the band iterator.
	GDALIterator* getDataIterator() const {return m_spDataIterator.get();}

	//! Calculate the spatial precision for projected coordinates.
	double calcSpatialPrecisionProjected() const;

	//! Calculate the spatial precision for geographic lat/lon coordinates.
	virtual double calcSpatialPrecisionGeographical(HorizontalDatum* pDatum) const;

	//! Initialize the MetaData object.
	void initMetaData();

	/*!
	Get the GDAL metadata object.

	\return	The metadata object (ownership retained)
	*/
	//! Return the GDAL metadata object.
	const GDALMetaData* getMetaDataGDAL() const {return &m_metaDataGDAL;}

	//! Perform a table lookup.
	PYXValue lookupValue(int nIndex) const;

private:

	//! The Metadata object used to determine geospatial information.
	GDALMetaData m_metaDataGDAL;

	//! The GDALDataset Reader, it is used to return the data
	GDALDatasetReader m_datasetReader;

	//! The spatial accuracy in metres
	double m_fSpatialAccuracy;

	//! The spatial precision in metres
	double m_fSpatialPrecision;

	//! The band iterator.
	PYXPointer<GDALIterator> m_spDataIterator;

	//! The bounds in native coordinates
	PYXRect2DDouble m_bounds;

	//! Pointer to the internal GDALDataset
	GDALDataset* m_pDataset;

	//! The coordinate converter.
	CoordConverterImpl m_coordConverter;

	//! Boolean to determine if the data source has a SRS.
	bool m_bHasSRS;

	//! Local copy of boolean to determine if geometry is created from setSpatialReferenceSystem.
	bool m_bCreateGeometry;

	//! Open state of data source  (1 = open, 2 = downloading, 3 = unavailable)
	int m_nOpenState;

private:

	/*!
	GDALDataSourceIterator iterates over points specified in the GIS file. It may
	return the same PYXIS index multiple times with different values if a cell
	encloses multiple data points.
	*/
	//! Iterates over the points in the file
	class WMSDataSourceIterator : public PYXXYIterator
	{

	public:

		//! Dynamic creator
		static PYXPointer<WMSDataSourceIterator> create(const WMSDataSource& reader)
		{
			return PYXNEW(WMSDataSourceIterator, reader);
		}

		//! Constructor.
		WMSDataSourceIterator(const WMSDataSource& reader);

		//! Destructor.
		virtual ~WMSDataSourceIterator() {;}

		//! Move to the next point.
		virtual void next();

		//! See if all the points have been covered.
		virtual bool end() const;

		//! Get the coordinates for the current point.
		virtual PYXCoord2DDouble getPoint() const;

		//! Get the value of the current cell.
		virtual PYXValue getFieldValue(int nFieldIndex = 0) const;

	private:

		// Disable copy constructor.
		WMSDataSourceIterator(const WMSDataSourceIterator&);

		// Disable copy assignment.
		void operator=(const WMSDataSourceIterator&);
		
	private:

		//! The reader.
		const WMSDataSource& m_reader;

		//! The band iterator.  The pointee is not owned by this pointer.
		GDALIterator* m_pDataIterator;
	};

private:

	friend class WMSDataSourceIterator;
	friend class WMSMultiDataSource;
};

#endif	// GDAL_DATA_SOURCE_H
