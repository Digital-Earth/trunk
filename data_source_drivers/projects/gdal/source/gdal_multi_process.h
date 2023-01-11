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
#include "exceptions.h"
#include "pyxis/pipe/process.h"
#include "coord_converter_impl.h"
#include "pyxis/utility/coord_2d.h"
#include "pyx_rtree.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"
#include "gdal_xy_coverage.h"
#include "pyxis/geometry/tile.h"

// standard includes
#include <map>
#include <memory>

// local forward declarations
class GDALXYCoverage;
class PYXSpatialReferenceSystem;

/*!
The GDALMultiProcess contains logic for loading multiple similar GIS 
files and returning data values for given cells.  It utilizes the PYXDataSource
interface.  The GDAL library is used to read the files. GDALMultiProcess works
in the native coordinates of the GDAL files that it's reading.
*/
//! Reads a collection of similar GIS Data Files utilizing the GDAL library.
class MODULE_GDAL_DECL GDALMultiProcess : public ProcessImpl<GDALMultiProcess>, public XYCoverageBase
{
	PYXCOM_DECLARE_CLASS();

	public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
	IUNKNOWN_QI_END
	
	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL();

public:

	IPROCESS_GETSPEC_IMPL();

	
	//! Get the output type of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IXYCoverage*>(this);
	}

	//! Get the output type of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IXYCoverage*>(this);
	}

	//! Test method
	static void test();

	//! Constructor.
	GDALMultiProcess();

	//! Destructor.
	virtual ~GDALMultiProcess();

	//! Return true if we have a spatial reference system.
	virtual bool STDMETHODCALLTYPE hasSpatialReferenceSystem() const;

	//! Set the spatial reference system.
	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS);

	//! Get the spatial precision of the data in metres.
	virtual double  STDMETHODCALLTYPE getSpatialPrecision() const;

	//! Get the bounds of the data set in native coordinate.
	virtual const PYXRect2DDouble& STDMETHODCALLTYPE getBounds() const {return m_bounds;}

	//! Get the distance between data points in this coverage
	virtual PYXCoord2DDouble STDMETHODCALLTYPE getStepSize() const;

	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const;

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue) const;

	virtual void STDMETHODCALLTYPE getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
													 PYXValue* pValues,
													 int sizeX,
													 int sizeY) const;

	//! Returns a coordinate converter used to create a geometry. (ownership retained)
	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const;

	virtual void STDMETHODCALLTYPE tileLoadHint (const PYXTile& tile) const;

	virtual void STDMETHODCALLTYPE tileLoadDoneHint (const PYXTile& tile) const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

private:
	
	friend class GDALXYCoverage;

	//! Disable copy constructor.
	GDALMultiProcess(const GDALMultiProcess&);

	//! Disable copy assignment.
	void operator=(const GDALMultiProcess&);

	//! Open the file.
	bool openFile(const std::string& strFileName);

	//! Get the reader based on the native coordinate.
	boost::intrusive_ptr<IXYCoverage> getDataSource(const PYXCoord2DDouble& native) const;

	void getValue(PYXCoord2DDouble nativeGetAt, PYXValue* pValue) const;

	/*!
	Open the data files located in the directory(s) specified.
	\return	Pointer to first GDALDataSet in multiset.
	*/
	virtual bool open(const std::vector<boost::filesystem::path>& filePaths);

	//! Helper method to get all the files in a specified directory.
	void getFiles(const boost::filesystem::path& directory, std::vector<boost::filesystem::path>& filePaths) const;
	
	//! Initialize all the data needed by this process once.
	void initDriverData(boost::intrusive_ptr<IXYCoverage> spXYCov);

	//! An intrusive pointer to the last accessed datasource.
	mutable boost::intrusive_ptr<IXYCoverage>  m_spLastAccessed;

	//! An intrusive pointer to the spatial reference system.
	boost::intrusive_ptr<ISRS> m_spSRS;

	//! Intrusive pointer ot the co-ordinate converter.
	mutable boost::intrusive_ptr<ICoordConverter> m_spCoordConv;

	//! The bounds in native coordinates
	PYXRect2DDouble m_bounds;

	//! Flag to indicate if the process has had it's data initalized.
	bool m_bDataInit;

	//! The rtree to store file coordintates.
	mutable PYXrTree m_rTree;

	//! Mutex for thread safety.
	mutable boost::recursive_mutex m_Mutex;

};

#endif	// GDAL_MULTI_DATA_SOURCE_H
