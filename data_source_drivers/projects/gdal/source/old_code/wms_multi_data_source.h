#ifndef WMS_MULTI_DATA_SOURCE_H
#define WMS_MULTI_DATA_SOURCE_H
/******************************************************************************
wms_multi_data_source.h

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "coord_converter_impl.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/exception.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"
#include "named_pipe.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

// standard includes
#include <map>
#include <memory>
#include <windows.h>

// local forward declarations
class WMSDataSource;
class PYXSpatialReferenceSystem;

//! Name of the plugin.  Displayed to user when selecting a DLL
typedef int (*DLLMAKENEWINSTANCE)();
typedef void (*DLLDELETEINSTANCE)(int nInstance);
typedef void (*DLLGETPIPENAME)(int nInstance, char* szBuffer, int nMaxSize);
//TODO: Code clean up after changest tested. CHOWELL
//class pyxSemaphore
//{
//public:
//
//	pyxSemaphore()
//		: m_hSemaphore(INVALID_HANDLE_VALUE)
//	{
//		m_hSemaphore = CreateSemaphore( 0, 0, 9999, 0);
//	};
//
//	~pyxSemaphore()
//	{
//		if (m_hSemaphore != INVALID_HANDLE_VALUE)
//		{
//			CloseHandle(m_hSemaphore);
//		}
//	}
//
//	void wait()
//	{
//		WaitForSingleObject(m_hSemaphore, INFINITE);
//	}
//
//	void signal()
//	{
//		LONG nPrevCount;
//		ReleaseSemaphore(m_hSemaphore, 1, &nPrevCount);
//	}
//
//	HANDLE m_hSemaphore;
//};

/*!
The WMSMultiDataSource contains logic for loading multiple similar GIS 
files and returning data values for given cells.  It utilizes the PYXDataSource
interface.  The GDAL library is used to read the files. WMSMultiDataSource works
in the native coordinates of the GDAL files that it s reading.
*/
//! Reads a collection of similar GIS Data Files utilizing the GDAL library.
class WMSMultiDataSource : public PYXXYCoverage
{

public:

	//! X and Y dimension in pixles of all images to download from the server.
	static const int knImagePixels;

	//! Test method
	static void test();

	static void getInvalidGeometry(std::map<std::string, std::vector<PYXPointer<PYXGeometry> > >& mapGeometry);

	static void addInvalidGeometry(std::string& strURI, PYXPointer<PYXGeometry> spGeometry);

	//! Mutex to serialize concurrent access by multiple threads
	static boost::recursive_mutex m_staticMutex;

	static std::map<std::string, std::vector<PYXPointer<PYXGeometry> > > m_geometryMap;

	//! Constructor.
	WMSMultiDataSource();

	//! Constructor.
	WMSMultiDataSource(WMSMultiDataSource* pDataSource);

	//! Copy constructor.
	WMSMultiDataSource(const WMSMultiDataSource&);

	//! Destructor.
	virtual ~WMSMultiDataSource();

	/*!
	Open the data files located in the directory(s) specified.
	\return	Pointer to first GDALDataSet in multiset.
	*/
	virtual bool open(	const std::string& strDir,
						const std::string& strFileExt);

	//! Get an iterator to all the points in the coverage.
	virtual PYXPointer<PYXXYIterator> getXYIterator() const;

	//! Return true if we have a spatial reference system.
	bool hasSpatialReferenceSystem() const;

	//! Set the spatial reference system.
	void setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS);

	//! Get the spatial precision of the data in metres.
	virtual double getSpatialPrecision() const;

	//! Get the number of files opened by this reader.
	int getNumFiles() const {return static_cast<int>(m_mapDataSource.size());}

	//! Get the geometry
	virtual PYXPointer<const PYXGeometry> getGeometry() const;

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

	//! The code that is run when this class is instantiated as a thread.
	void operator()();

	//! Set resolution of data to be returned in subsequent requests.
	virtual void setRequestedDataResolution(int nResolution);

	//! initialize metadata.
	void initMetaData();

		/*!
	For data sources that can have multiple resolutions of data (WMS for now)
	give the minumum available resolution we can return.

	\return minimum resolution this data source can return. -1 if not implemented.
	*/
	virtual int getMinimumAvailableResolution() { return m_nMinLOD * 3; };

	/*!
	For data sources that can have multiple resolutions of data (WMS for now)
	give the maximum available resolution we can return.

	\return maximum resolution this data source can return. -1 if not implemented.
	*/
	virtual int getMaximumAvailableResolution() { return m_nMaxLOD * 3; };


private:

	//! Disable copy assignment.
	void operator=(const WMSMultiDataSource&);

	//! Request the data source that contained the specified point at the specified LOD.
	void requestDataSource(const PYXCoord2DDouble& native, int nLod, char szPriority) const;

	//! Create a GDAL data source but do not open it.
	bool openFile(const std::string& strFileName,
				  int nLod, 
				  int nLonMin, int nLatMin,
				  int nLonMax, int nLatMax,
				  bool bInvalidate = true) const;

	//! Get the reader based on the native coordinate.
	const WMSDataSource* getDataSource(const PYXCoord2DDouble& native) const;

	//! Get value from data source.
	PYXValue getDSValue(const WMSDataSource* pDS, const PYXCoord2DDouble& native) const;


private:

	//! Map of WMSDataSources.
	typedef std::map<PYXCoord2DInt, PYXPointer<WMSDataSource> > WMSDataSourceMap;

	/*!
	Map of individual files for this data source at request LOD. By south west lon/lat.
	*/
	mutable WMSDataSourceMap m_mapDataSource;

	/*!
	Map of individual files for this data source at lower than request LOD - speed optimization. By south west lon/lat.
	*/
	mutable WMSDataSourceMap m_mapLowDataSource;

	//! The bounds in native coordinates
	PYXRect2DDouble m_bounds;

	//! LOD of data currently contained in data source.
	mutable int m_nCurrentLOD;

	//! Name of host to retreive data from.
	std::string m_strHost;

	//! Name of layer of data to retreive.
	std::string m_strLayer;

	//! Host path added to host name for requests for data
	std::string m_strHostPath;

	//! Style of data being looked for.
	std::string m_strStyle;

	//! Format of image downloaded from server.
	std::string m_strFormat;

	//! Description of this data source.
	std::string m_strDescription;

	//! Terminating character for pipe communication.
	std::string m_strTerminator;

	//! Minimum LOD available from this server.
	int m_nMinLOD;

	//! Maximum LOD available from this server.
	int m_nMaxLOD;

	//! The last LOD set using the "setRequestedDataResolution" function.
	mutable int m_nLod;

	//! Coordinate converter.
	CoordConverterImpl	m_coordConverter;

	//! Bounds of the last selected data source - speed optimization.
	mutable PYXRect2DDouble	m_lastDSBounds;

	//! Pointer to last accessed WMS tile.
	mutable WMSDataSource* m_pLastDS;

	//! Grid scale factor - used to calculate top left corner of grid a coordinate falls in.
	mutable double m_fGridFactor;

	//! The thread that updates data tiles.
	std::auto_ptr<boost::thread> m_spPipeThread;

	//! Pointer to instance of class used by thread.
	WMSMultiDataSource* m_pThreadDataSource;

	//! Set to true to terminate thread (then send message to pipe to close it).
	bool m_bTerminate;

	//! Counter (incremented each time a WMSMultiDataSource is opened) used in pipe name.
	static int m_nCounter;

	//! Name of communication pipe.
	std::string m_strPipeName;

	//! true if we are an elevation data source.
	bool m_bElevationDataSource;

	HMODULE m_hPlugIn;

	// Get create instance function.
	DLLMAKENEWINSTANCE m_pDLLMakeNewInstance;

	// Get delete instance function.
	DLLDELETEINSTANCE m_pDLLDeleteInstance;

	// Get pipe name function.
	DLLGETPIPENAME m_pDLLGetPipeName;

	int m_nInstance;
	std::string m_strServerPipeName;

	//! Semaphore used to trigger thread to exit on terminate.
	//pyxSemaphore m_processPipeMessagesSemaphore;

	//! Pipe used for connection to server.
	PYXNamedPipe m_serverPipe;

	//! Boolean set when currently downloading files are to be removed.
	mutable bool m_bClearDownloads;

private:

	/*!
	Iterator iterates over points specified in the GDAL files. It provides
	methods for getting the point latitude and longitude and getting the point
	value.
	*/
	//! Iterates over points in the GDAL list of files.
	class WMSMultiIterator : public PYXXYIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<WMSMultiIterator> create(
			WMSDataSourceMap::const_iterator begin,
			WMSDataSourceMap::const_iterator end	)
		{
			return PYXNEW(WMSMultiIterator, begin, end);
		}

		//! Constructor
		WMSMultiIterator(	WMSDataSourceMap::const_iterator begin,
							WMSDataSourceMap::const_iterator end	);

		//! Destructor
		virtual ~WMSMultiIterator();

		//! Move to the next point.
		virtual void next();

		//! See if we have covered all the points.
		virtual bool end() const {return (m_it == m_end);}

		//! Get the coordinates for the current point.
		virtual PYXCoord2DDouble getPoint() const;

		//! Get the value of the current cell.
		virtual PYXValue getFieldValue(int nFieldIndex = 0) const;

	private:

		//! Current iteration
		WMSDataSourceMap::const_iterator m_it;

		//! End of iteration
		WMSDataSourceMap::const_iterator m_end;

		//! Current GDALReader iterator
		PYXPointer<PYXXYIterator> m_spIterator;

	};
};

#endif	// WMS_MULTI_DATA_SOURCE_H
