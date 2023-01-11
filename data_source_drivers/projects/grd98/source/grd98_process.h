#ifndef GRD98_PROCESS_H
#define GRD98_PROCESS_H
/******************************************************************************
grd98_process.h

begin		: 2005-04-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_grd98.h"

// local includes
#include "grd98_header_reader.h"

// pyxlib includes
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/pipe/process.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/utility/memory_manager.h"

// boost includes
#include <boost/filesystem/path.hpp>

// standard includes
#include <iosfwd>
#include <vector>

/*!
Contains logic for loading the GEODAS GRID DATA FORMAT. The native coordinates
for the GRD98 data source are geodetic seconds.
*/
//! A process for GRD98 elevation.
class MODULE_GRD98_DECL GRD98Process : public ProcessImpl<GRD98Process>, public XYCoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	GRD98Process();

	//! Destructor
	~GRD98Process();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

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

	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const;

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

	//! Returns a coordinate converter used to create a geometry. (ownership retained)
	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const
	{
		return &m_coordConverter;
	}

	/*!
	Get the spatial precision of the data.

	\return The spatial precision in metres or -1 if unknown.
	*/
	virtual double STDMETHODCALLTYPE getSpatialPrecision() const;

public:

	//! Add the definition for the data set's content
	static void addContentDefinition(PYXPointer<PYXTableDefinition> pTableDefn);

	//! Called by the memory manager when memory is low.
	void freeMemory();

	//! Open the data file.
	bool open(const std::string& strFileName);

	//! Get the number of points in the file.
	double getPointCount() const;

private:

	//! Disable copy constructor.
	GRD98Process(const GRD98Process&);

	//! Disable copy assignment.
	void operator=(const GRD98Process&);

	//! Get the width of the data file in geodetic seconds of longitude.
	int getWidth() const;

	//! Get the height of the data file in geodetic seconds of latitude.
	int getHeight() const;

	//! Read an integer value from the file.
	int readValueInt(unsigned int nOffset) const;

	//! Read a double value from the file.
	double readValueDouble(unsigned int nOffset) const;

	//! Calculate the file offset for a given point.
	unsigned int calcFileOffset(const PYXCoord2DInt& native) const;

	//!	Get the resolution of the data source.
	int getLonResolution() const;

	//!	Get the resolution of the data source.
	int getLatResolution() const;

	//! Allocate the data buffer and read data from the file.
	void loadDataBuffer(unsigned int nOffset) const;

	//! Open the data file for reading.
	void openFileForRead(std::ifstream& in) const;

	//! Initialize the meta data objects.
	void initMetaData();

private:

	// Structure to hold time and block for sorting for freeing memory.
	struct BLOCKTIMESTRUCT
	{
		unsigned int nTime;
		unsigned int nBlock;
	};

private:

	//! GRD98 file extension.
	static const std::string kstrFileExt;

private:

	//! Compare function for sort.
	static bool compareGRD98(	GRD98Process::BLOCKTIMESTRUCT& arg1,
								GRD98Process::BLOCKTIMESTRUCT& arg2);

private:

	//! The memory resource
	mutable std::vector<MemoryResource*> m_vecMemoryResource;

	//! Time stamp for memory resources.
	mutable std::vector<unsigned int> m_vecLastAccessedTime;

	//! Last block of memory that was accessed.
	mutable unsigned int m_nLastBlock;

	//! Internal counter used to set last accessed time for a memory block.
	mutable unsigned int m_nAccessCounter;

	//!	Internal vector used for sorting access times.
	std::vector<BLOCKTIMESTRUCT> m_vecTimes;

	//! The GRD98 header reader.
	GRD98HeaderReader m_header;

	//! The coordinate converter.
	WGS84CoordConverter m_coordConverter;

	//! Stream for reading file.
	mutable std::ifstream m_in;

	//!	Number of bytes of data in the file.
	unsigned int m_nFileSize;

	//! Number of blocks in the file.
	unsigned int m_nNumBlocks; 
};

#endif // guard
