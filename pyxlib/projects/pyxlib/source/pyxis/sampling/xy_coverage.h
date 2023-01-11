#ifndef PYXIS__SAMPLING__XY_COVERAGE_H
#define PYXIS__SAMPLING__XY_COVERAGE_H
/******************************************************************************
xy_coverage.h

begin		: 2006-03-13
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/procs/srs.h"
#include "pyxis/utility/rect_2d.h"
#include "pyxis/utility/thread_pool.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

// forward declarations
class PYXSpatialReferenceSystem;
class PYXXYIterator;
class PYXTile;

/*! A helper class for XY coverages that will allow the coverage to do
    some pre work to be able to optimize the getting of values from the coverage.
	For instance, if every call to get a coverage value needs to get the 
	same three bands of the coverage, the work can be done ahead of time
	and stored in this class.
*/
class PYXLIB_DECL XYCoverageValueGetter
{
public:
	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue) const 
	{
		return false;
	}
};


class PYXLIB_DECL XYAsyncValueConsumer
{
public:
	virtual void onRequestCompleted(const PYXIcosIndex & index,
									const PYXCoord2DDouble & nativeCoord,
									bool * hasValues,
									PYXValue * values,
									int width,int height) const = 0;
};

class PYXLIB_DECL XYAsyncValueGetter : public PYXObject
{
public:
	//add requests for each cell in a given tile.
	virtual void addAsyncRequests(const PYXTile & tile) = 0;

	//add a request with a given PYXIndex
	virtual void addAsyncRequest(const PYXIcosIndex & index) = 0;

	//waits until all asyncRequests were completed.
	//once the wait has been triggered, its is not possible to add new requests
	//return true when all requests were performed successfully. false is there were error
	virtual bool join() = 0;
};

/*!
*/
//! Provides access to coverage data through xy coordinates.
struct PYXLIB_DECL IXYCoverage : public IFeatureCollection
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const = 0;

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() = 0;

	//! Get an optimized coverage value getter to use if one is going to retrieve a number of points.
	/*!  This is an optimization that is available to XYCoverage classes.  The general idea is to
	be able to make any decisions that are going to be the same for each call to GetCoverageValue().
	As one example, the GDAL coverage can return straight values or values looked up in a Colour Look Up
	Table.  So, instead of coding this check in every call to GetCoverageValue, the getCoverageValueGetter()
	can return one getter for look up tables, and one for non-lookup.  As a reference, the GetCoverageValue()
	function is called close to a million times in putting up the first screen of data in WorldView.
	IMPORTANT NOTE:  The return value XYCoverageValueGetter* from this function is a straight pointer that
	is owned by the XYCoverage.  The pointer should never be freed, and the pointer will only be valid as long
	as the XYCoverage is also valid.

	\return	A pointer to a class which can be used to get data points from the coverage.
	*/

	virtual XYCoverageValueGetter* STDMETHODCALLTYPE getCoverageValueGetter() const = 0;


	//! Get an optimized goverage value getter to use to retrive number of points.
	/*! This optimization is used to retrive a number of points in async manner.
	The caller should use XYAsyncValueGetter::addRequest(PYXIndex) to add requests to perform.
	once all requests were added, the user should call XYAsyncValueGetter::wait to gerentee that 
	all requestes were completed.

	the given XYAsyncValueConsumer object will be called when a request is completed.

	the given matrixWidth and matrixHeight is been used to get a matrix of values centered on the 
	requested PYXIndex (used by Bilinear and Bicubic samplers)

	\retrun XYAsyncValueGetter object to add request into.
	*/
	virtual PYXPointer<XYAsyncValueGetter> STDMETHODCALLTYPE getAsyncCoverageValueGetter(
			const XYAsyncValueConsumer & consumer,
			int matrixWidth,
			int matrixHeight
		) const = 0;

	/*!
	Get the coverage value at the specified native coordinates.
	This call MUST be thread-safe.

	\param	native		The native coordinates.
	\param  pValue      an pointer to the PYXValue to be filled in

	\return	True if a data value was retrieved, false if there was no value to return.
	*/
	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue) const = 0;

	/*!
	Get the field values around the specified native coordinates.
	The origin of the matrix of returned values will be at the grid point 
	in the mesh that is lower or equal to the nativeCentre point and adjusted 
	left (or down) by trunc((size - 1)/2).

	If the values to be returned fall outside of the current data set, then 
	the edges of the data set will be duplicated and returned.

	\param	nativeCentre	The native coordinate.
	\param  pValues         an array of PYXValue to be filled in with field values
							that are centered on the point requested.
	\param  sizeX           width of PYXValue array
	\param  sizeY           height of PYXValue array
	*/
	virtual void STDMETHODCALLTYPE getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
													 PYXValue* pValues,
													 int sizeX,
													 int sizeY) const = 0;

	/*!
	Call this method after the data source is opened to determine if the data
	source contains a spatial reference. If not, a spatial reference must be
	supplied by calling setSpatialReference() before getFeatureIterator() is
	called.
	
	\return	true if the data source has a spatial reference, otherwise false
	*/
	virtual bool STDMETHODCALLTYPE hasSpatialReferenceSystem() const = 0;

	/*!
	Specify the spatial reference for the data source. Call this method to set
	the spatial reference if after the data source is opened
	hasSpatialReference() returns false.

	\param	spSRS	The spatial reference system.
	*/
	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS) = 0;

	//! Returns a coordinate converter used to create a geometry. (ownership retained)
	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const = 0;

	/*!
	Get the spatial precision of the data.

	\return The spatial precision in metres or -1 if unknown.
	*/
	virtual double STDMETHODCALLTYPE getSpatialPrecision() const = 0;

	/*!
	Get the bounds of the coverage in native coordinate.

	\return	The bounds as a rectangle.
	*/
	virtual const PYXRect2DDouble& STDMETHODCALLTYPE getBounds() const = 0;

	//! Get the distance between data points in this coverage
	virtual PYXCoord2DDouble STDMETHODCALLTYPE getStepSize() const = 0;

	//! Get a location inside a raster pixel based on native coordinate
	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const = 0;

	/*! 
	Hint to the coverage that we are about to load a tile of data.  The implementation
	may choose to ignore this call and a default do nothing implementation is provided 
	in XYCoverageBase.  The intent is for the coverage to be able to prefetch an area
	of data so that cell by cell access can be sped up.

	\param	tile	The tile that is about to be read.
	*/
	virtual void STDMETHODCALLTYPE tileLoadHint (const PYXTile& tile) const = 0;

	/*! 
	Hint to the coverage that we are done loading a tile of data.  The implementation
	may choose to ignore this call and a default do nothing implementation is provided 
	in XYCoverageBase.  The intent is for the coverage to be able to free memory that was
	allocated during tileLoadHint()

	\param	tile	The tile that is finished being read.
	*/
	virtual void STDMETHODCALLTYPE tileLoadDoneHint (const PYXTile& tile) const = 0;

};

#define IXYCOVERAGE_IMPL_PROXY(proxy) \
public: \
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const \
	{ \
		return (proxy).getCoverageDefinition(); \
	} \
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() \
	{ \
		return (proxy).getCoverageDefinition(); \
	} \
	virtual XYCoverageValueGetter* STDMETHODCALLTYPE getCoverageValueGetter() const \
	{ \
		return (proxy).getCoverageValueGetter(); \
	} \
	virtual PYXPointer<XYAsyncValueGetter> STDMETHODCALLTYPE getAsyncCoverageValueGetter( \
			const XYAsyncValueConsumer & consumer, \
			int matrixWidth, \
			int matrixHeight \
		) const \
	{ \
		return (proxy).getAsyncCoverageValueGetter(consumer,matrixWidth,matrixHeight); \
	} \
	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,PYXValue* pValue) const \
	{ \
		return (proxy).getCoverageValue(native,pValue); \
	} \
	virtual void STDMETHODCALLTYPE getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,PYXValue* pValues,int sizeX,int sizeY) const \
	{ \
		return (proxy).getMatrixOfValues(nativeCentre,pValues,sizeX,sizeY); \
	} \
	virtual bool STDMETHODCALLTYPE hasSpatialReferenceSystem() const \
	{ \
		return (proxy).hasSpatialReferenceSystem(); \
	} \
	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS) \
	{ \
		return (proxy).setSpatialReferenceSystem(spSRS); \
	} \
	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const \
	{ \
		return (proxy).getCoordConverter(); \
	} \
	virtual double STDMETHODCALLTYPE getSpatialPrecision() const \
	{ \
		return (proxy).getSpatialPrecision(); \
	} \
	virtual const PYXRect2DDouble& STDMETHODCALLTYPE getBounds() const \
	{ \
		return (proxy).getBounds(); \
	} \
	virtual PYXCoord2DDouble STDMETHODCALLTYPE getStepSize() const \
	{ \
		return (proxy).getStepSize(); \
	} \
	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const \
	{ \
		return (proxy).nativeToRasterSubPixel(native); \
	} \
	virtual void STDMETHODCALLTYPE tileLoadHint (const PYXTile& tile) const \
	{ \
		return (proxy).tileLoadHint(tile); \
	} \
	virtual void STDMETHODCALLTYPE tileLoadDoneHint (const PYXTile& tile) const \
	{ \
		return (proxy).tileLoadDoneHint(tile); \
	}




//! default implementation of XYCoverageValueGetter
class PYXLIB_DECL XYCoverageValueGetterDefault : public XYCoverageValueGetter
{
public:
	void setXYCoverageValue (IXYCoverage* pXYCoverage)
	{
		m_pXYCoverage = pXYCoverage;
	}

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue) const
	{
		return m_pXYCoverage->getCoverageValue(native, pValue);
	}

private:
	IXYCoverage* m_pXYCoverage;
};

class PYXLIB_DECL DefaultXYAsyncValueGetter : public XYAsyncValueGetter
{
private:
	struct LocalStorage
	{
		std::vector<PYXValue> values;
		std::vector<char> hasValues;
	};

	int m_width;
	int m_height;

	PYXTaskGroupWithLocalStorage<LocalStorage> m_requests;

	const IXYCoverage & m_coverage;	
	const XYAsyncValueConsumer & m_consumer;
	bool m_canAddRequests;

	boost::recursive_mutex m_mutex;

public:

	static PYXPointer<DefaultXYAsyncValueGetter> DefaultXYAsyncValueGetter::create(const IXYCoverage & coverage,const XYAsyncValueConsumer & consumer, int width, int height)
	{
		return PYXNEW(DefaultXYAsyncValueGetter,coverage,consumer,width,height);
	}

	DefaultXYAsyncValueGetter(const IXYCoverage & coverage,const XYAsyncValueConsumer & consumer, int width, int height)
		:	m_coverage(coverage),
			m_consumer(consumer),
			m_width(width),
			m_height(height),
			m_canAddRequests(true)
	{
		m_requests.initLocalStorage(boost::bind(&DefaultXYAsyncValueGetter::initLocalStorage,this,_1));
	}

	//add requests for each cell in a given tile.
	virtual void addAsyncRequests(const PYXTile & tile)
	{
		if (!m_canAddRequests)
		{
			PYXTHROW(PYXException,"can't add requests once wait was called");
		}

		m_requests.addTask(boost::bind(&DefaultXYAsyncValueGetter::doRequests,this,_1,tile));
	}

	//add a request with a given PYXIndex
	virtual void addAsyncRequest(const PYXIcosIndex & index)
	{
		if (!m_canAddRequests)
		{
			PYXTHROW(PYXException,"can't add requests once wait was called");
		}

		m_requests.addTask(boost::bind(&DefaultXYAsyncValueGetter::doRequest,this,_1,index));
	}

	//waits until all asyncRequests were completed.
	//once the wait has been triggered, its is not possible to add new requests
	//return true when all requests were performed successfully. false is there were error
	virtual bool join()
	{
		m_canAddRequests = false;

		m_requests.joinAll();

		return true;
	}

private:
	void initLocalStorage(LocalStorage & storage)
	{
		storage.values.resize(m_width*m_height);
		storage.hasValues.resize(m_width*m_height);
	}

	void doRequests(LocalStorage & storage,PYXTile tile)
	{
		for (PYXExhaustiveIterator it(tile.getRootIndex(), tile.getCellResolution()); !it.end(); it.next())
		{
			doRequest(storage,it.getIndex());
		}
	}

	void doRequest(LocalStorage & storage,PYXIcosIndex index)
	{
		PYXCoord2DDouble native;
		if (!m_coverage.getCoordConverter()->tryPyxisToNative(index,&native) ||
			!m_coverage.getBounds().inside(native))	
		{
			m_consumer.onRequestCompleted(index,native,0,0,0,0);
			return;
		}

		m_coverage.getMatrixOfValues(native,&(storage.values[0]),m_width,m_height);

		//the default getMatrixOfValues doesn't return if we realy get a value but set the values to null
		for(int x=0;x<m_width;x++)
		{
			for(int y=0;y<m_height;y++)
			{
				storage.hasValues[x*m_height+y] = !storage.values[x*m_height+y].isNull();
			}
		}

		m_consumer.onRequestCompleted(index,native,(bool*)&(storage.hasValues[0]),&(storage.values[0]),m_width,m_height);
	}
};


//! Helper class for implementing xy coverages.
class PYXLIB_DECL XYCoverageBase : public IXYCoverage
{
public:

	XYCoverageBase() :
		m_spCovDefn(PYXTableDefinition::create()),
		m_fSpatialAccuracy(-1.0),
		m_fSpatialPrecision(-1.0),
		m_bHasSRS(false)
	{
		m_getter.setXYCoverageValue(this);
	}

public: // IXYCoverage

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		return m_spCovDefn;
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return m_spCovDefn;
	}

	virtual XYCoverageValueGetter* STDMETHODCALLTYPE getCoverageValueGetter() const
	{
		return &m_getter;
	}

	virtual PYXPointer<XYAsyncValueGetter> STDMETHODCALLTYPE getAsyncCoverageValueGetter(
			const XYAsyncValueConsumer & consumer,
			int matrixWidth,
			int matrixHeight
		) const 
	{
		return DefaultXYAsyncValueGetter::create(*this,consumer,matrixWidth,matrixHeight);
	}

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue,
													int nResolution,
													int nFieldIndex = 0	) const
	{
		// Default returns that there is no data.
		return false;
	}

	virtual void STDMETHODCALLTYPE getMatrixOfValues(	const PYXCoord2DDouble& nativeCentre,
															PYXValue* pValues,
															int sizeX,
															int sizeY,
															int nResolution,
															int nFieldIndex = 0		) const
	{
		// Default does nothing
	}

	virtual bool STDMETHODCALLTYPE hasSpatialReferenceSystem() const
	{
		return m_bHasSRS;
	}

	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS)
	{
		// Default does nothing
	}

	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const
	{
		// Default does nothing
		return 0;
	}

	virtual double STDMETHODCALLTYPE getSpatialPrecision() const
	{
		return m_fSpatialPrecision;
	}

	virtual const PYXRect2DDouble& STDMETHODCALLTYPE getBounds() const
	{
		return m_bounds;
	}

	virtual PYXCoord2DDouble STDMETHODCALLTYPE getStepSize() const
	{
		return m_stepSize;
	}

	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

	virtual void STDMETHODCALLTYPE tileLoadHint (const PYXTile& tile) const
	{
		// Default does nothing
	}

	virtual void STDMETHODCALLTYPE tileLoadDoneHint (const PYXTile& tile) const
	{
		// Default does nothing
	}

protected:

	//! Mutex to serialize concurrent access by multiple threads.
	mutable boost::recursive_mutex m_mutex;

	//! Describes the fields in the coverage.
	PYXPointer<PYXTableDefinition> m_spCovDefn;

	//! The bounds in native coordinates.
	PYXRect2DDouble m_bounds;

	//! The step size in native coordinates.
	PYXCoord2DDouble m_stepSize;

	//! The spatial accuracy in metres.
	double m_fSpatialAccuracy;

	//! The spatial precision in metres.
	double m_fSpatialPrecision;

	//! Whether the data source has a SRS.
	bool m_bHasSRS;

	//! the value getter class.
	mutable XYCoverageValueGetterDefault m_getter;
};

#endif // guard