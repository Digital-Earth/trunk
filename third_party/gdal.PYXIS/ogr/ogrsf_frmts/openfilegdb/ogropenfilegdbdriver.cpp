/******************************************************************************
 * $Id: ogropenfilegdbdriver.cpp 27384 2014-05-24 12:28:12Z rouault $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Implements Open FileGDB OGR driver.
 * Author:   Even Rouault, <even dot rouault at mines-dash paris dot org>
 *
 ******************************************************************************
 * Copyright (c) 2014, Even Rouault <even dot rouault at mines-paris dot org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "ogr_openfilegdb.h"

CPL_CVSID("$Id");

// g++ -O2 -Wall -Wextra -g -shared -fPIC ogr/ogrsf_frmts/openfilegdb/*.cpp -o ogr_OpenFileGDB.so -Iport -Igcore -Iogr -Iogr/ogrsf_frmts -Iogr/ogrsf_frmts/mem -Iogr/ogrsf_frmts/openfilegdb -L. -lgdal

extern "C" void RegisterOGROpenFileGDB();


#define ENDS_WITH(str, strLen, end) \
    (strLen >= strlen(end) && EQUAL(str + strLen - strlen(end), end))

/************************************************************************/
/*                         OGROpenFileGDBDriverIdentify()               */
/************************************************************************/

static int OGROpenFileGDBDriverIdentifyInternal( GDALOpenInfo* poOpenInfo,
                                         const char*& pszFilename )
{
#ifdef FOR_FUSIL
    CPLString osOrigFilename(pszFilename);
#endif

    // First check if we have to do any work.
    size_t nLen = strlen(pszFilename);
// START-PYXIS - Add support for directories ending in "_gdb"
//    if( ENDS_WITH(pszFilename, nLen, ".gdb") ||
//        ENDS_WITH(pszFilename, nLen, ".gdb/") )
    if( ENDS_WITH(pszFilename, nLen, ".gdb") ||
		ENDS_WITH(pszFilename, nLen, "_gdb") ||
		ENDS_WITH(pszFilename, nLen, ".gdb/") ||
		ENDS_WITH(pszFilename, nLen, "_gdb/")	)
// END-PYXIS
    {
        /* Check that the filename is really a directory, to avoid confusion with */
        /* Garmin MapSource - gdb format which can be a problem when the */
        /* driver is loaded as a plugin, and loaded before the GPSBabel driver */
        /* (http://trac.osgeo.org/osgeo4w/ticket/245) */
        if( strncmp(pszFilename, "/vsicurl/https://github.com/",
                    strlen("/vsicurl/https://github.com/")) == 0 ||
            !poOpenInfo->bStatOK ||
            !poOpenInfo->bIsDirectory )
        {
            /* In case we don't manage to list the directory, try to stat one file */
            VSIStatBufL stat;
            if( !(strncmp(pszFilename, "/vsicurl/", strlen("/vsicurl/")) == 0 &&
                  VSIStatL( CPLFormFilename(pszFilename, "a00000001", "gdbtable"), &stat ) == 0) )
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    /* We also accept zipped GDB */
    else if( ENDS_WITH(pszFilename, nLen, ".gdb.zip") ||
             ENDS_WITH(pszFilename, nLen, ".gdb.tar") ||
                /* Canvec GBs */
             (ENDS_WITH(pszFilename, nLen, ".zip") &&
              (strstr(pszFilename, "_gdb") != NULL ||
// START-PYXIS
               strstr(pszFilename, "_GDB") != NULL)) ||
             (ENDS_WITH(pszFilename, nLen, ".tar") &&
              (strstr(pszFilename, "_gdb") != NULL ||
// END-PYXIS
               strstr(pszFilename, "_GDB") != NULL))	)
    {
        return TRUE;
    }
    /* We also accept tables themselves */
    else if( ENDS_WITH(pszFilename, nLen, ".gdbtable") )
    {
        return TRUE;
    }
#ifdef FOR_FUSIL
    /* To be able to test fuzzer on any auxiliary files used (indexes, etc.) */
    else if( strlen(CPLGetBasename(pszFilename)) == 9 &&
             CPLGetBasename(pszFilename)[0] == 'a' )
    {
        pszFilename = CPLFormFilename(CPLGetPath(pszFilename),
                                      CPLGetBasename(pszFilename),
                                      "gdbtable");
        return TRUE;
    }
    else if( strlen(CPLGetBasename(CPLGetBasename(pszFilename))) == 9 &&
             CPLGetBasename(CPLGetBasename(pszFilename))[0] == 'a' )
    {
        pszFilename = CPLFormFilename(CPLGetPath(pszFilename),
                                      CPLGetBasename(CPLGetBasename(pszFilename)),
                                      "gdbtable");
        return TRUE;
    }
#endif
    else
    {
        return FALSE;
    }
}

static int OGROpenFileGDBDriverIdentify( GDALOpenInfo* poOpenInfo )
{
    const char* pszFilename = poOpenInfo->pszFilename;
    return OGROpenFileGDBDriverIdentifyInternal( poOpenInfo, pszFilename );
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

static GDALDataset* OGROpenFileGDBDriverOpen( GDALOpenInfo* poOpenInfo )

{
    if( poOpenInfo->eAccess == GA_Update )
        return NULL;
    const char* pszFilename = poOpenInfo->pszFilename;
#ifdef FOR_FUSIL
    CPLString osOrigFilename(pszFilename);
#endif
    if( OGROpenFileGDBDriverIdentifyInternal( poOpenInfo, pszFilename ) == FALSE )
        return NULL;

#ifdef FOR_FUSIL
    const char* pszSrcDir = CPLGetConfigOption("FUSIL_SRC_DIR", NULL);
    if( pszSrcDir != NULL && VSIStatL( osOrigFilename, &stat ) == 0 &&
        VSI_ISREG(stat.st_mode) )
    {
        /* Copy all files from FUSIL_SRC_DIR to directory of pszFilename */
        /* except pszFilename itself */
        CPLString osSave(pszFilename);
        char** papszFiles = VSIReadDir(pszSrcDir);
        for(int i=0; papszFiles[i] != NULL; i++)
        {
            if( strcmp(papszFiles[i], CPLGetFilename(osOrigFilename)) != 0 )
            {
                CPLCopyFile(CPLFormFilename(CPLGetPath(osOrigFilename), papszFiles[i], NULL),
                            CPLFormFilename(pszSrcDir, papszFiles[i], NULL));
            }
        }
        CSLDestroy(papszFiles);
        pszFilename = CPLFormFilename("", osSave.c_str(), NULL);
    }
#endif

    OGROpenFileGDBDataSource* poDS = new OGROpenFileGDBDataSource();
    if( poDS->Open( pszFilename ) )
    {
        return poDS;
    }
    else
    {
        delete poDS;
        return NULL;
    }
}

/***********************************************************************/
/*                       RegisterOGROpenFileGDB()                      */
/***********************************************************************/

void RegisterOGROpenFileGDB()

{
    if (! GDAL_CHECK_VERSION("OGR OpenFileGDB"))
        return;
    GDALDriver  *poDriver;

    if( GDALGetDriverByName( "OpenFileGDB" ) == NULL )
    {
        poDriver = new GDALDriver();

        poDriver->SetDescription( "OpenFileGDB" );
        poDriver->SetMetadataItem( GDAL_DCAP_VECTOR, "YES" );
        poDriver->SetMetadataItem( GDAL_DMD_LONGNAME,
                                   "ESRI FileGDB" );
        poDriver->SetMetadataItem( GDAL_DMD_EXTENSION, "gdb" );
        poDriver->SetMetadataItem( GDAL_DMD_HELPTOPIC,
                                   "drv_openfilegdb.html" );

        poDriver->SetMetadataItem( GDAL_DCAP_VIRTUALIO, "YES" );

        poDriver->pfnOpen = OGROpenFileGDBDriverOpen;
        poDriver->pfnIdentify = OGROpenFileGDBDriverIdentify;

        GetGDALDriverManager()->RegisterDriver( poDriver );
    }
}
