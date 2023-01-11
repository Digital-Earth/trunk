/******************************************************************************
pyx_gdal_driver.cpp

begin		: 2005-04-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_GDAL_SOURCE
#include "pyx_gdal_driver.h"

// local includes
#include "gdal_data_source.h"

// pyxis library includes
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"

// GDAL includes
#include "cpl_multiproc.h"

// {0FC41236-D4C0-4e41-8497-18A9D9ED5859}
PYXCOM_DEFINE_CLSID(PYXGDALDriver, 
0xfc41236, 0xd4c0, 0x4e41, 0x84, 0x97, 0x18, 0xa9, 0xd9, 0xed, 0x58, 0x59);
PYXCOM_CLASS_INTERFACES(PYXGDALDriver, IDataDriver::iid, IUnknown::iid);

namespace
{

//! Static object to manage resources.
struct StaticResourceManager
{

	StaticResourceManager()
	{
		PYXGDALDriver::registerAllDrivers();
	}

	~StaticResourceManager()
	{
		PYXGDALDriver::destroyDriverManager();
	}

} staticResourceManager;

}

//! The name of the driver
static const std::string kstrDriverName = "GDAL";

//! Tester class
TesterUnit<PYXGDALDriver> gTester;

//! Test method
void PYXGDALDriver::test()
{
}

//! Constructor.
PYXGDALDriver::PYXGDALDriver()
{
}

//! Destructor
PYXGDALDriver::~PYXGDALDriver()
{
}

/*!
Get the name of the driver.

\return	The name of the driver.
*/
std::string PYXGDALDriver::getName() const
{
	return kstrDriverName;
}

/*!
Open a GDAL data source.

There are two ways that failure can occur.  It is not a valid GDAL data set,
and it is a valid GDAL data set but it had some problem opening it.

1. If it is not a valid GDAL data set, then the method will return an empty shared pointer.
2. If it is valid but failed to open it, then an exception will be thrown.
3. If it is valid and was successfully opened, then a shared pointer to the data source will be returned.

\param	strDataSourceName	The name of the data source.

\return	A shared pointer to the data source or an empty shared pointer if the
		data source could not be opened.
*/
PYXPointer<PYXDataSource>
PYXGDALDriver::openForRead(const std::string& strDataSourceName) const
{
	// create the data source
	PYXPointer<GDALDataSource> spDataSource(GDALDataSource::create());

	// open the data source  
	if (!spDataSource->open(strDataSourceName))
	{
		// if it returned false try again specifying the ADRG driver format for OGDI.
		std::string strADRG("gltp:/adrg/");
		strADRG.append(strDataSourceName);

		// Change all '\' to '/'
		size_t nPos = 0;
		do
		{
			nPos = strADRG.find("\\", nPos);
			if (nPos < -1)
			{
				strADRG[nPos]='/';
			}
		} while (nPos != -1);

		// If not a directory (we are a file) strip off file name portion.
		if (!FileUtils::isDir(strADRG))
		{
			nPos = strADRG.rfind("/");
		
			/*
			Trim off file name portion.

			ADRG files have to be opened based on directory, 
			not individual file name.
			*/
			if (nPos != std::string::npos)
			{
				strADRG[nPos+1] = 0;
			}
		}

		// If we still can't open the file, exit.
		if (!spDataSource->open(strADRG))
		{
			return PYXPointer<PYXDataSource>();
		}
	}

	return spDataSource;
}

/************************************************************************/
/*                        GDALSetBounds()                        */
/************************************************************************/

void 
GDALSetBounds( GDALDatasetH hDataset,
			  IDataDriver::FileInfo* pFileInfo,
               double fTLX, double fTLY,
			   double fBRX, double fBRY)

{
    double	dfGeoTLX, dfGeoTLY;
    double	dfGeoBRX, dfGeoBRY;
    const char  *pszProjection;
    double	adfGeoTransform[6];
    OGRCoordinateTransformationH hTransform = NULL;

/* -------------------------------------------------------------------- */
/*      Transform the point into georeferenced coordinates.             */
/* -------------------------------------------------------------------- */
    if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None )
    {
        pszProjection = GDALGetProjectionRef(hDataset);

        dfGeoTLX = adfGeoTransform[0] + adfGeoTransform[1] * fTLX
            + adfGeoTransform[2] * fTLY;
        dfGeoTLY = adfGeoTransform[3] + adfGeoTransform[4] * fTLX
            + adfGeoTransform[5] * fTLY;

        dfGeoBRX = adfGeoTransform[0] + adfGeoTransform[1] * fBRX
            + adfGeoTransform[2] * fBRY;
        dfGeoBRY = adfGeoTransform[3] + adfGeoTransform[4] * fBRX
            + adfGeoTransform[5] * fBRY;

	}
    else
    {
		pFileInfo->m_bounds.setXMin(fTLX);
		pFileInfo->m_bounds.setYMin(fTLY);
		pFileInfo->m_bounds.setXMax(fBRX);
		pFileInfo->m_bounds.setYMax(fBRY);

        return;
    }

/* -------------------------------------------------------------------- */
/*      Setup transformation to lat/long.                               */
/* -------------------------------------------------------------------- */
    if( pszProjection != NULL && strlen(pszProjection) > 0 )
    {
        OGRSpatialReferenceH hProj, hLatLong = NULL;

        hProj = OSRNewSpatialReference( pszProjection );
        if( hProj != NULL )
            hLatLong = OSRCloneGeogCS( hProj );

        if( hLatLong != NULL )
        {
            CPLPushErrorHandler( CPLQuietErrorHandler );
            hTransform = OCTNewCoordinateTransformation( hProj, hLatLong );
            CPLPopErrorHandler();
            
            OSRDestroySpatialReference( hLatLong );
        }

        if( hProj != NULL )
            OSRDestroySpatialReference( hProj );
    }

/* -------------------------------------------------------------------- */
/*      Transform to latlong and report.                                */
/* -------------------------------------------------------------------- */
	bool bHaveTL = false;
	bool bHaveBR = false;
    if( hTransform != NULL)
	{
		if (OCTTransform(hTransform,1,&dfGeoTLX,&dfGeoTLY,NULL) )
		{
			bHaveTL = true;
			pFileInfo->m_bounds.setXMin(dfGeoTLX);
			pFileInfo->m_bounds.setYMin(dfGeoTLY);
		}

		if (OCTTransform(hTransform,1,&dfGeoBRX,&dfGeoBRY,NULL) )
		{
			bHaveBR = true;
			pFileInfo->m_bounds.setXMax(dfGeoBRX);
			pFileInfo->m_bounds.setYMax(dfGeoBRY);
		}

		OCTDestroyCoordinateTransformation( hTransform );
    }

	if ( !bHaveTL || !bHaveBR)
	{
		pFileInfo->m_bounds.setXMin(dfGeoTLX);
		pFileInfo->m_bounds.setYMin(dfGeoTLY);
		pFileInfo->m_bounds.setXMax(dfGeoBRX);
		pFileInfo->m_bounds.setYMax(dfGeoBRY);
	}
}

/*!
Fill supplied FileInfo structure with information about the specific file.

\param	strURI	URI of file to check.
\param	pFileInfo	Pointer to file instructure to fill with info on this file.
\param  pbExit	Pointer to optional boolean set to true when we should exit immediately.

\return true if able to obtain info on file, otherwise fase (ie. file not valid for this driver).
*/
bool PYXGDALDriver::getFileInfo(const std::string& strURI, IDataDriver::FileInfo* pFileInfo, bool* pbExit )
{
    GDALDatasetH hDataset;
    GDALRasterBandH hBand;
    int iBand;
    GDALDriverH hDriver;
    int bComputeMinMax = FALSE;
	int bSample = FALSE;
    int bShowGCPs = TRUE;
	int bShowMetadata = TRUE;
    int bStats = FALSE;
    const char *pszFilename = NULL;

	std::string strResult;

	std::string strTheURI(strURI);

	hDataset = GDALOpen( strTheURI.c_str(), GA_ReadOnly );
    
    if( hDataset == NULL && !(pbExit && *pbExit))
    {
		// if it returned false try again specifying the ADRG driver format for OGDI.
		std::string strADRG("gltp:/adrg/");
		strADRG.append(strURI);

		// Change all '\' to '/'
		size_t nPos = 0;
		do
		{
			nPos = strADRG.find("\\", nPos);
			if (nPos < -1)
			{
				strADRG[nPos]='/';
			}
		} while(nPos != -1);

		// If not a directory (we are a file) strip off file name portion.
		if (!FileUtils::isDir(strADRG))
		{
			nPos = strADRG.rfind("/");
		
			/*
			Trim off file name portion.

			ADRG files have to be opened based on directory, 
			not individual file name.
			*/
			if (nPos != std::string::npos)
			{
				strADRG[nPos] = 0;
			}
		}

		strTheURI = strADRG;

		// Try opening again
		try
		{
			if (!(pbExit && *pbExit))
			{
				hDataset = GDALOpen( strTheURI.c_str(), GA_ReadOnly );
			}
		}
		catch (...)
		{
			return false;
		}
    }
    
    if( hDataset == NULL )
	{
		return false;
	}

	// Assign file name.
	pFileInfo->m_strURI = strTheURI;

	// Assign file type.
	pFileInfo->m_nLayerType = IDataDriver::knCoverage;

/* -------------------------------------------------------------------- */
/*      Report general info.                                            */
/* -------------------------------------------------------------------- */
    hDriver = GDALGetDatasetDriver( hDataset );
    
	// File type;
	pFileInfo->m_nFileType = IDataDriver::GDAL;

	// Driver name.
	pFileInfo->m_strDriverName = GDALGetDriverLongName( hDriver );

	// Raster size.
	pFileInfo->m_nRasterSize.setX(GDALGetRasterXSize( hDataset ));
	pFileInfo->m_nRasterSize.setY(GDALGetRasterYSize( hDataset ));

/* -------------------------------------------------------------------- */
/*      Report corners.                                                 */
/* -------------------------------------------------------------------- */
	
	GDALSetBounds(hDataset, pFileInfo, 0.0, 0.0,
				  GDALGetRasterXSize(hDataset),
				  GDALGetRasterYSize(hDataset));

	pFileInfo->m_nNumBands = GDALGetRasterCount(hDataset);

/* ==================================================================== */
/*      Loop over bands.                                                */
/* ==================================================================== */
    for( iBand = 0; iBand < GDALGetRasterCount( hDataset ) && !(pbExit && *pbExit); ++iBand )
    {
        double      dfNoData;
        int         bGotNodata;
        int         nBlockXSize, nBlockYSize;
        GDALColorTableH	hTable;

        hBand = GDALGetRasterBand( hDataset, iBand+1 );

        GDALGetBlockSize( hBand, &nBlockXSize, &nBlockYSize );

		IDataDriver::FileInfo::BandInfo bandInfo;

		pFileInfo->m_nFileInterpretation = IDataDriver::knNone;

		bandInfo.m_bHaveBounds = false;

		bandInfo.m_strBandType = GDALGetDataTypeName(GDALGetRasterDataType(hBand));

		bandInfo.m_strColorInterpretation = GDALGetColorInterpretationName(GDALGetRasterColorInterpretation(hBand));

		if( GDALGetDescription( hBand ) != NULL 
            && strlen(GDALGetDescription( hBand )) > 0 )
		{
			bandInfo.m_strBandDesc = GDALGetDescription(hBand);
		}

		if (bandInfo.m_strColorInterpretation == "Undefined")
		{
			if (bandInfo.m_strBandType == "Int16" || bandInfo.m_strBandType == "Int32")
			{
				pFileInfo->m_nFileInterpretation = IDataDriver::knElevation;
			}
			if (bandInfo.m_strBandType == "Byte")
			{
				pFileInfo->m_nFileInterpretation = IDataDriver::knRaster;
			}
			if (bandInfo.m_strBandType == "Float32") // && !bandInfo.m_strBandDesc.empty())
			{
				pFileInfo->m_nFileInterpretation = IDataDriver::knElevation;
			}
			if (bandInfo.m_strBandType == "UInt16")
			{
				pFileInfo->m_nFileInterpretation = IDataDriver::knElevation;
			}
			if (bandInfo.m_strBandType == "CInt16")
			{
				pFileInfo->m_nFileInterpretation = IDataDriver::knRaster;
			}
		}
		else
		{
			if (bandInfo.m_strBandType == "Byte")
			{
				pFileInfo->m_nFileInterpretation = IDataDriver::knRaster;
			}
			if (bandInfo.m_strColorInterpretation == "Gray")
			{
				pFileInfo->m_nFileInterpretation = IDataDriver::knRaster;
			}
		}

        dfNoData = GDALGetRasterNoDataValue( hBand, &bGotNodata );
        if( bGotNodata )
        {
			bandInfo.m_bHasNoDataValue = true;
			bandInfo.m_fNoDataValue = dfNoData;
        }
		else
		{
			bandInfo.m_bHasNoDataValue = false;
			bandInfo.m_fNoDataValue = 0.0;
		}

		bandInfo.m_nNumOverviews = GDALGetOverviewCount(hBand);

        if( GDALGetOverviewCount(hBand) > 0 )
        {
            int		iOverview;

			for( iOverview = 0; 
                 iOverview < GDALGetOverviewCount(hBand);
                 iOverview++ )
            {
                GDALRasterBandH	hOverview;
				
				PYXCoord2DInt overviewSize;

                hOverview = GDALGetOverview( hBand, iOverview );
				overviewSize.setX(GDALGetRasterBandXSize( hOverview ));
				overviewSize.setY(GDALGetRasterBandYSize( hOverview ));
			
				bandInfo.m_vecOverviewSize.push_back(overviewSize);
			}
        }

        if( GDALGetRasterColorInterpretation(hBand) == GCI_PaletteIndex 
            && (hTable = GDALGetRasterColorTable( hBand )) != NULL )
        {
			bandInfo.m_strColorTableInterpretation = GDALGetPaletteInterpretationName(GDALGetPaletteInterpretation( hTable ));
			bandInfo.m_nNumColorTableEntries = GDALGetColorEntryCount( hTable );
        }
		else
		{
			bandInfo.m_nNumColorTableEntries = 0;
		}
    
		pFileInfo->m_vecBandInfo.push_back(bandInfo);
	}


    GDALClose( hDataset );

	return true;
}


void PYXGDALDriver::registerAllDrivers()
{
	GDALAllRegister();	
}

void PYXGDALDriver::destroyDriverManager()
{
	GDALDestroyDriverManager();
}
