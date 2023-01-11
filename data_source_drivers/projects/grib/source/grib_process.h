#ifndef GRIB_DATA_SOURCE_H
#define GRIB_DATA_SOURCE_H
/******************************************************************************
grib_data_source.h

begin		: 2006-03-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "grib.h"
#include "grib_coord_converter.h"
#include "grib_dataset_reader.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/procs/path.h"
#include "pyxis/pipe/process.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>

// local forward declarations
class ProjectionMethod;
class PYXGeometry;

/*!
Provides access to a single record within a GRIdded Binary (GRIB) data file.
The grib files usually hold weather data and often have many images with discrete
binary data. 
*/
//! Provides data that is held within GRIB files.
class GRIB_DECL GRIBProcess : public ProcessImpl<GRIBProcess>, public XYCoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	GRIBProcess() :
		m_spDefn(PYXTableDefinition::create()),
		m_nRecord(-1)
	{
		m_stepSize.setX(1.0);
		m_stepSize.setY(1.0);
	}

	//! Test method
	static void test();

	//! Set the record number for the process
	void setRecord(int nRecord);

	//! Get the record number for the process.
	int getRecord() const {return m_nRecord;}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( GRIBProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IXYCoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IXYCoverage*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL();

public: // IXYCoverage

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue) const;

	virtual void STDMETHODCALLTYPE getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
													 PYXValue* pValues,
													 int sizeX,
													 int sizeY) const;

	/*!
	Specify the spatial reference for the data source. Call this method to set
	the spatial reference if after the data source is opened
	hasSpatialReference() returns false.

	\param	spSRS	The spatial reference system.
	*/
	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(
		boost::intrusive_ptr<ISRS> spSRS	) {assert(false && "not implemented");}

	//! Returns a coordinate converter used to create a geometry. (ownership retained)
	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const
	{
		return &m_coordConverter;
	}

	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const;

	/*!
	Get the spatial precision of the data.

	\return The spatial precision in metres or -1 if unknown.
	*/
	virtual double STDMETHODCALLTYPE getSpatialPrecision() const;

	/*!
	Get the dataset reader used by this process for gaining access to records.
	\return The dataset reader used by this process.
	*/
	const GRIBDatasetReader& getDataSetReader() const
	{
		return m_gribReader;
	}

private:

	//! Calculate the spatial precision for geographic lat/lon coordinates.
	double calcSpatialPrecisionGeographical(HorizontalDatum* pDatum) const;

	//! Get the value at the specified native raster coordinate.
    bool getValue(const PYXCoord2DInt& raster, PYXValue* pValue) const;

	//! Initialize the metadata.
	void initMetaData();

	//! open the data set with a third party reader.
	void readFile(const boost::filesystem::path& path, int nRecord);

private:

	//! The reader for the grib record.
	GRIBDatasetReader m_gribReader;

	//! The record index of the image within its grib file.
	int m_nRecord;

	//! The coordinate converter.
	GRIBCoordConverter m_coordConverter;

	//! The path this process is to open.
	boost::intrusive_ptr<IPath> m_spPath;

	friend GRIB_DECL bool operator ==(const GRIBProcess& lhs, const GRIBProcess& rhs);
};

//! The equality operator.
bool GRIB_DECL operator ==(const GRIBProcess& lhs, const GRIBProcess& rhs);

//! The inequality operator.
bool GRIB_DECL operator !=(const GRIBProcess& lhs, const GRIBProcess& rhs);

#endif