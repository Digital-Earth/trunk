/******************************************************************************
 * $Id: gdal_frmts.h 13124 2007-11-28 00:08:49Z pvachon $
 *
 * Project:  GDAL
 * Purpose:  Prototypes for all format specific driver initializations.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2001, Frank Warmerdam
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

#ifndef GDAL_FRMTS_H_INCLUDED
#define GDAL_FRMTS_H_INCLUDED

#include "cpl_port.h"

CPL_C_START
void CPL_DLL GDALRegister_GDB(void);
void CPL_DLL GDALRegister_GTiff(void);
void CPL_DLL GDALRegister_GXF(void);
void CPL_DLL GDALRegister_OGDI(void);
void CPL_DLL GDALRegister_HFA(void);
void CPL_DLL GDALRegister_AAIGrid(void);
void CPL_DLL GDALRegister_AIGrid(void);
void CPL_DLL GDALRegister_AIGrid2(void);
void CPL_DLL GDALRegister_CEOS(void);
void CPL_DLL GDALRegister_SAR_CEOS(void);
void CPL_DLL GDALRegister_SDTS(void);
void CPL_DLL GDALRegister_ELAS(void);
void CPL_DLL GDALRegister_EHdr(void);
void CPL_DLL GDALRegister_GenBin(void);
void CPL_DLL GDALRegister_PAux(void);
void CPL_DLL GDALRegister_ENVI(void);
void CPL_DLL GDALRegister_DOQ1(void);
void CPL_DLL GDALRegister_DOQ2(void);
void CPL_DLL GDALRegister_DTED(void);
void CPL_DLL GDALRegister_MFF(void);
void CPL_DLL GDALRegister_HKV(void);
void CPL_DLL GDALRegister_PNG(void);
void CPL_DLL GDALRegister_JPEG(void);
void CPL_DLL GDALRegister_JPEG2000(void);
void CPL_DLL GDALRegister_JP2KAK(void);
void CPL_DLL GDALRegister_MEM(void);
void CPL_DLL GDALRegister_JDEM(void);
void CPL_DLL GDALRegister_GRASS(void);
void CPL_DLL GDALRegister_PNM(void);
void CPL_DLL GDALRegister_GIF(void);
void CPL_DLL GDALRegister_Envisat(void);
void CPL_DLL GDALRegister_FITS(void);
void CPL_DLL GDALRegister_ECW(void);
void CPL_DLL GDALRegister_JP2ECW(void);
void CPL_DLL GDALRegister_FujiBAS(void);
void CPL_DLL GDALRegister_FIT(void);
void CPL_DLL GDALRegister_VRT(void);
void CPL_DLL GDALRegister_USGSDEM(void);
void CPL_DLL GDALRegister_FAST(void);
void CPL_DLL GDALRegister_HDF4(void);
void CPL_DLL GDALRegister_HDF4Image(void);
void CPL_DLL GDALRegister_L1B(void);
void CPL_DLL GDALRegister_LDF(void);
void CPL_DLL GDALRegister_BSB(void);
void CPL_DLL GDALRegister_XPM(void);
void CPL_DLL GDALRegister_BMP(void);
void CPL_DLL GDALRegister_GSC(void);
void CPL_DLL GDALRegister_NITF(void);
void CPL_DLL GDALRegister_RPFTOC(void);
void CPL_DLL GDALRegister_MrSID(void);
void CPL_DLL GDALRegister_PCIDSK(void);
void CPL_DLL GDALRegister_BT(void);
void CPL_DLL GDALRegister_DODS(void);
void CPL_DLL GDALRegister_GMT(void);
void CPL_DLL GDALRegister_netCDF(void);
void CPL_DLL GDALRegister_LAN(void);
void CPL_DLL GDALRegister_CPG(void);
void CPL_DLL GDALRegister_AirSAR(void);
void CPL_DLL GDALRegister_RS2(void);
void CPL_DLL GDALRegister_ILWIS(void);
void CPL_DLL GDALRegister_PCRaster(void);
void CPL_DLL GDALRegister_IDA(void);
void CPL_DLL GDALRegister_NDF(void);
void CPL_DLL GDALRegister_RMF(void);
void CPL_DLL GDALRegister_HDF5(void);
void CPL_DLL GDALRegister_HDF5Image(void);
void CPL_DLL GDALRegister_MSGN(void);
void CPL_DLL GDALRegister_RIK(void);
void CPL_DLL GDALRegister_Leveller(void);
void CPL_DLL GDALRegister_SGI(void);
void CPL_DLL GDALRegister_SRTMHGT(void);
void CPL_DLL GDALRegister_DIPEx(void);
void CPL_DLL GDALRegister_ISIS3(void);
void CPL_DLL GDALRegister_ISIS2(void);
void CPL_DLL GDALRegister_PDS(void);
void CPL_DLL GDALRegister_IDRISI(void);
void CPL_DLL GDALRegister_Terragen(void);
void CPL_DLL GDALRegister_WCS(void);
void CPL_DLL GDALRegister_WMS(void);
void CPL_DLL GDALRegister_HTTP(void);
void CPL_DLL GDALRegister_SDE(void);
void CPL_DLL GDALRegister_GSAG(void);
void CPL_DLL GDALRegister_GSBG(void);
void CPL_DLL GDALRegister_GS7BG(void);
void CPL_DLL GDALRegister_GRIB(void);
void CPL_DLL GDALRegister_INGR(void);
void CPL_DLL GDALRegister_ERS(void);
void CPL_DLL GDALRegister_PALSARJaxa(void);
void CPL_DLL GDALRegister_DIMAP();
void CPL_DLL GDALRegister_GFF(void);
void CPL_DLL GDALRegister_COSAR(void);
void CPL_DLL GDALRegister_TSX(void);
void CPL_DLL GDALRegister_ADRG(void);
void CPL_DLL GDALRegister_COASP(void);
void CPL_DLL GDALRegister_PYXIS(void);
CPL_C_END

#endif /* ndef GDAL_FRMTS_H_INCLUDED */
