#ifndef PYXIS__DATA__DATA_DRIVER_H
#define PYXIS__DATA__DATA_DRIVER_H
/******************************************************************************
data_driver.h

begin		: 2006-11-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/rect_2d.h"

// standard includes
#include <string>
#include <vector>

// forward declarations
class PYXDataSource;

/*!
TODO update this description
PYXDriver is the abstract base class for drivers that read data files in PYXIS
and non-PYXIS format. This class is responsible for registering itself with the
PYXDriverRegistrar.
*/
//! Abstract base class for drivers that read GIS data files.
struct PYXLIB_DECL IDataDriver : public IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	enum eLayerType { knCoverage, knFeature };
	enum eFileType { GDAL_MULTI, DTED_MULTI, GRD98, DTED, GDAL, GRIB, OGR, PYXIS, PDB, KML, TEXT, PIPELINE };
	enum eFileInterpretation { knNone, knRaster, knElevation, knVector };

	struct FileInfo
	{
		std::string	m_strURI;
		std::string m_strDriverName;
		PYXCoord2DInt m_nRasterSize;
		PYXRect2DDouble	m_bounds;
		int m_nLayerCount;
		int m_nNumBands;
		eLayerType m_nLayerType;
		eFileType m_nFileType;
		eFileInterpretation m_nFileInterpretation;

		struct BandInfo
		{
			std::string	m_strBandType;
			std::string m_strColorInterpretation;
			std::string m_strBandDesc;
			bool m_bHasNoDataValue;
			double m_fNoDataValue;
			int m_nNumOverviews;
			std::vector<PYXCoord2DInt> m_vecOverviewSize;
			int m_nNumColorTableEntries;
			std::string m_strColorTableInterpretation;
			PYXRect2DDouble	m_bounds;
			bool m_bHaveBounds;
		};

		std::vector<BandInfo> m_vecBandInfo;

		struct LayerInfo
		{
			std::string m_strLayerName;
			std::string m_strGeometryType;
			int m_nFeatureCount;
			PYXRect2DDouble	m_bounds;
		};
		
		std::vector<LayerInfo> m_vecLayerInfo;
	};

public:

	virtual std::string STDMETHODCALLTYPE getName() const = 0;

	//! Open a data source
	/*!
	Open a data source.

	\param	strDataSourceName	The name of the data source. [TODO Is this a file name?]

	\return	Shared pointer to the data source or an empty shared pointer if the
			data source could not be opened.
	*/
	virtual PYXPointer<PYXDataSource> STDMETHODCALLTYPE openForRead(
		const std::string& strDataSourceName	) const = 0;

	/*!
	Fill supplied FileInfo structure with information about the specific file.

	\param	strURI	URI of file to check.
	\param	pFileInfo	Pointer to file structure to fill with info on this file.
	\param  pbExit	Pointer to optional bool set to true if the function should exit immediately.

	\return true if able to obtain info on file, otherwise false (i.e. file not valid for this driver).
	*/
	virtual bool STDMETHODCALLTYPE getFileInfo(const std::string& strURI, IDataDriver::FileInfo* pFileInfo, bool* pbExit = 0) = 0;

};

#endif // guard
