/******************************************************************************
 * $Id: cpl_http.cpp 29152 2015-05-04 11:45:44Z rouault $
 *
 * Project:  libcurl based HTTP client
 * Purpose:  libcurl based HTTP client
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2006, Frank Warmerdam
 * Copyright (c) 2008-2013, Even Rouault <even dot rouault at mines-paris dot org>
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

#include <map>
#include "cpl_http.h"
#include "cpl_multiproc.h"

#ifdef HAVE_CURL
#  include <curl/curl.h>

void CPLHTTPSetOptions(CURL *http_handle, char** papszOptions);

/* CURLINFO_RESPONSE_CODE was known as CURLINFO_HTTP_CODE in libcurl 7.10.7 and earlier */
#if LIBCURL_VERSION_NUM < 0x070a07
#define CURLINFO_RESPONSE_CODE CURLINFO_HTTP_CODE
#endif

#endif

CPL_CVSID("$Id: cpl_http.cpp 29152 2015-05-04 11:45:44Z rouault $");

// list of named persistent http sessions 

#ifdef HAVE_CURL
static std::map<CPLString,CURL*>* poSessionMap = NULL;
static CPLMutex *hSessionMapMutex = NULL;
#endif

/************************************************************************/
/*                            CPLWriteFct()                             */
/*                                                                      */
/*      Append incoming text to our collection buffer, reallocating     */
/*      it larger as needed.                                            */
/************************************************************************/

#ifdef HAVE_CURL
static size_t 
CPLWriteFct(void *buffer, size_t size, size_t nmemb, void *reqInfo)

{
    CPLHTTPResult *psResult = (CPLHTTPResult *) reqInfo;
    int  nNewSize;

    nNewSize = psResult->nDataLen + nmemb*size + 1;
    if( nNewSize > psResult->nDataAlloc )
    {
        psResult->nDataAlloc = (int) (nNewSize * 1.25 + 100);
        GByte* pabyNewData = (GByte *) VSIRealloc(psResult->pabyData,
                                                  psResult->nDataAlloc);
        if( pabyNewData == NULL )
        {
            VSIFree(psResult->pabyData);
            psResult->pabyData = NULL;
            psResult->pszErrBuf = CPLStrdup(CPLString().Printf("Out of memory allocating %d bytes for HTTP data buffer.", psResult->nDataAlloc));
            psResult->nDataAlloc = psResult->nDataLen = 0;

            return 0;
        }
        psResult->pabyData = pabyNewData;
    }

    memcpy( psResult->pabyData + psResult->nDataLen, buffer,
            nmemb * size );

    psResult->nDataLen += nmemb * size;
    psResult->pabyData[psResult->nDataLen] = 0;

    return nmemb;
}

/************************************************************************/
/*                           CPLHdrWriteFct()                           */
/************************************************************************/
static size_t CPLHdrWriteFct(void *buffer, size_t size, size_t nmemb, void *reqInfo)
{
    CPLHTTPResult *psResult = (CPLHTTPResult *) reqInfo;
    // copy the buffer to a char* and initialize with zeros (zero terminate as well)
    char* pszHdr = (char*)CPLCalloc(nmemb + 1, size);
    CPLPrintString(pszHdr, (char *)buffer, nmemb * size);
    char *pszKey = NULL;
    const char *pszValue = CPLParseNameValue(pszHdr, &pszKey );
    psResult->papszHeaders = CSLSetNameValue(psResult->papszHeaders, pszKey, pszValue);
    CPLFree(pszHdr);
    CPLFree(pszKey);
    return nmemb; 
}

#endif /* def HAVE_CURL */

/************************************************************************/
/*                           CPLHTTPFetch()                             */
/************************************************************************/

/**
 * \brief Fetch a document from an url and return in a string.
 *
 * @param pszURL valid URL recognized by underlying download library (libcurl)
 * @param papszOptions option list as a NULL-terminated array of strings. May be NULL.
 *                     The following options are handled :
 * <ul>
 * <li>TIMEOUT=val, where val is in seconds</li>
 * <li>HEADERS=val, where val is an extra header to use when getting a web page.
 *                  For example "Accept: application/x-ogcwkt"
 * <li>HTTPAUTH=[BASIC/NTLM/GSSNEGOTIATE/ANY] to specify an authentication scheme to use.
 * <li>USERPWD=userid:password to specify a user and password for authentication
 * <li>POSTFIELDS=val, where val is a nul-terminated string to be passed to the server
 *                     with a POST request.
 * <li>PROXY=val, to make requests go through a proxy server, where val is of the
 *                form proxy.server.com:port_number
 * <li>PROXYUSERPWD=val, where val is of the form username:password
 * <li>PROXYAUTH=[BASIC/NTLM/DIGEST/ANY] to specify an proxy authentication scheme to use.
 * <li>NETRC=[YES/NO] to enable or disable use of $HOME/.netrc, default YES.
 * <li>CUSTOMREQUEST=val, where val is GET, PUT, POST, DELETE, etc.. (GDAL >= 1.9.0)
 * <li>COOKIE=val, where val is formatted as COOKIE1=VALUE1; COOKIE2=VALUE2; ...
 * <li>MAX_RETRY=val, where val is the maximum number of retry attempts if a 503 or
 *               504 HTTP error occurs. Default is 0. (GDAL >= 2.0)
 * <li>RETRY_DELAY=val, where val is the number of seconds between retry attempts.
 *                 Default is 30. (GDAL >= 2.0)
 * </ul>
 *
 * Alternatively, if not defined in the papszOptions arguments, the PROXY,  
 * PROXYUSERPWD, PROXYAUTH, NETRC, MAX_RETRY and RETRY_DELAY values are searched in the configuration 
 * options named GDAL_HTTP_PROXY, GDAL_HTTP_PROXYUSERPWD, GDAL_PROXY_AUTH, 
 * GDAL_HTTP_NETRC, GDAL_HTTP_MAX_RETRY and GDAL_HTTP_RETRY_DELAY.
 *
 * @return a CPLHTTPResult* structure that must be freed by 
 * CPLHTTPDestroyResult(), or NULL if libcurl support is disabled
 */
CPLHTTPResult *CPLHTTPFetch( const char *pszURL, char **papszOptions )

{
    if( strncmp(pszURL, "/vsimem/", strlen("/vsimem/")) == 0 &&
        /* Disabled by default for potential security issues */
        CSLTestBoolean(CPLGetConfigOption("CPL_CURL_ENABLE_VSIMEM", "FALSE")) )
    {
        CPLString osURL(pszURL);
        const char* pszPost = CSLFetchNameValue( papszOptions, "POSTFIELDS" );
        if( pszPost != NULL ) /* Hack: we append post content to filename */
        {
            osURL += "&POSTFIELDS=";
            osURL += pszPost;
        }
        vsi_l_offset nLength = 0;
        CPLHTTPResult* psResult = (CPLHTTPResult* )CPLCalloc(1, sizeof(CPLHTTPResult));
        GByte* pabyData = VSIGetMemFileBuffer( osURL, &nLength, FALSE );
        if( pabyData == NULL )
        {
            CPLDebug("HTTP", "Cannot find %s", osURL.c_str());
            psResult->nStatus = 1;
            psResult->pszErrBuf = CPLStrdup(CPLSPrintf("HTTP error code : %d", 404));
            CPLError( CE_Failure, CPLE_AppDefined, "%s", psResult->pszErrBuf );
        }
        else if( nLength != 0 )
        {
            psResult->nDataLen = (size_t)nLength;
            psResult->pabyData = (GByte*) CPLMalloc((size_t)nLength + 1);
            memcpy(psResult->pabyData, pabyData, (size_t)nLength);
            psResult->pabyData[(size_t)nLength] = 0;
        }

        if( psResult->pabyData != NULL &&
            strncmp((const char*)psResult->pabyData, "Content-Type: ",
                    strlen("Content-Type: ")) == 0 )
        {
            const char* pszContentType = (const char*)psResult->pabyData + strlen("Content-type: ");
            const char* pszEOL = strchr(pszContentType, '\r');
            if( pszEOL )
                pszEOL = strchr(pszContentType, '\n');
            if( pszEOL )
            {
                int nLength = pszEOL - pszContentType;
                psResult->pszContentType = (char*)CPLMalloc(nLength + 1);
                memcpy(psResult->pszContentType, pszContentType, nLength);
                psResult->pszContentType[nLength] = 0;
            }
        }

        return psResult;
    }

#ifndef HAVE_CURL
    (void) papszOptions;
    (void) pszURL;

    CPLError( CE_Failure, CPLE_NotSupported,
              "GDAL/OGR not compiled with libcurl support, remote requests not supported." );
    return NULL;
#else

/* -------------------------------------------------------------------- */
/*      Are we using a persistent named session?  If so, search for     */
/*      or create it.                                                   */
/*                                                                      */
/*      Currently this code does not attempt to protect against         */
/*      multiple threads asking for the same named session.  If that    */
/*      occurs it will be in use in multiple threads at once which      */
/*      might have bad consequences depending on what guarantees        */
/*      libcurl gives - which I have not investigated.                  */
/* -------------------------------------------------------------------- */
    CURL *http_handle = NULL;

    const char *pszPersistent = CSLFetchNameValue( papszOptions, "PERSISTENT" );
    const char *pszClosePersistent = CSLFetchNameValue( papszOptions, "CLOSE_PERSISTENT" );
    if (pszPersistent)
    {
        CPLString osSessionName = pszPersistent;
        CPLMutexHolder oHolder( &hSessionMapMutex );

        if( poSessionMap == NULL )
            poSessionMap = new std::map<CPLString,CURL*>;
        if( poSessionMap->count( osSessionName ) == 0 )
        {
            (*poSessionMap)[osSessionName] = curl_easy_init();
            CPLDebug( "HTTP", "Establish persistent session named '%s'.",
                      osSessionName.c_str() );
        }

        http_handle = (*poSessionMap)[osSessionName];
    }
/* -------------------------------------------------------------------- */
/*      Are we requested to close a persistent named session?          */
/* -------------------------------------------------------------------- */
    else if (pszClosePersistent)
    {
        CPLString osSessionName = pszClosePersistent;
        CPLMutexHolder oHolder( &hSessionMapMutex );

        if( poSessionMap )
        {
            std::map<CPLString,CURL*>::iterator oIter = poSessionMap->find( osSessionName );
            if( oIter != poSessionMap->end() )
            {
                curl_easy_cleanup(oIter->second);
                poSessionMap->erase(oIter);
                if( poSessionMap->size() == 0 )
                {
                    delete poSessionMap;
                    poSessionMap = NULL;
                }
                CPLDebug( "HTTP", "Ended persistent session named '%s'.",
                        osSessionName.c_str() );
            }
            else
            {
                CPLDebug( "HTTP", "Could not find persistent session named '%s'.",
                        osSessionName.c_str() );
            }
        }

        return NULL;
    }
    else
        http_handle = curl_easy_init();

/* -------------------------------------------------------------------- */
/*      Setup the request.                                              */
/* -------------------------------------------------------------------- */
    char szCurlErrBuf[CURL_ERROR_SIZE+1];
    CPLHTTPResult *psResult;
    struct curl_slist *headers=NULL; 

    const char* pszArobase = strchr(pszURL, '@');
    const char* pszSlash = strchr(pszURL, '/');
    const char* pszColon = (pszSlash) ? strchr(pszSlash, ':') : NULL;
    if (pszArobase != NULL && pszColon != NULL && pszArobase - pszColon > 0)
    {
        /* http://user:password@www.example.com */
        char* pszSanitizedURL = CPLStrdup(pszURL);
        pszSanitizedURL[pszColon-pszURL] = 0;
        CPLDebug( "HTTP", "Fetch(%s:#password#%s)", pszSanitizedURL, pszArobase );
        CPLFree(pszSanitizedURL);
    }
    else
    {
        CPLDebug( "HTTP", "Fetch(%s)", pszURL );
    }

    psResult = (CPLHTTPResult *) CPLCalloc(1,sizeof(CPLHTTPResult));

    curl_easy_setopt(http_handle, CURLOPT_URL, pszURL );

    CPLHTTPSetOptions(http_handle, papszOptions);

    // turn off SSL verification, accept all servers with ssl
    curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, FALSE);

    /* Set Headers.*/
    const char *pszHeaders = CSLFetchNameValue( papszOptions, "HEADERS" );
    if( pszHeaders != NULL ) {
        CPLDebug ("HTTP", "These HTTP headers were set: %s", pszHeaders);
        headers = curl_slist_append(headers, pszHeaders);
        curl_easy_setopt(http_handle, CURLOPT_HTTPHEADER, headers);
    }

    // are we making a head request
    const char* pszNoBody = NULL;
    if ((pszNoBody = CSLFetchNameValue( papszOptions, "NO_BODY" )) != NULL)
    {
        if (CSLTestBoolean(pszNoBody)) 
        {
            CPLDebug ("HTTP", "HEAD Request: %s", pszURL);
            curl_easy_setopt(http_handle, CURLOPT_NOBODY, 1L);           
        }
    }

    // capture response headers
    curl_easy_setopt(http_handle, CURLOPT_HEADERDATA, psResult);
    curl_easy_setopt(http_handle, CURLOPT_HEADERFUNCTION, CPLHdrWriteFct);
 
    curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, psResult );
    curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, CPLWriteFct );

    szCurlErrBuf[0] = '\0';

    curl_easy_setopt(http_handle, CURLOPT_ERRORBUFFER, szCurlErrBuf );

    static int bHasCheckVersion = FALSE;
    static int bSupportGZip = FALSE;
    if (!bHasCheckVersion)
    {
        bSupportGZip = strstr(curl_version(), "zlib/") != NULL;
        bHasCheckVersion = TRUE;
    }
    int bGZipRequested = FALSE;
    if (bSupportGZip && CSLTestBoolean(CPLGetConfigOption("CPL_CURL_GZIP", "YES")))
    {
        bGZipRequested = TRUE;
        curl_easy_setopt(http_handle, CURLOPT_ENCODING, "gzip");
    }

/* -------------------------------------------------------------------- */
/*      If 502, 503 or 504 status code retry this HTTP call until max        */
/*      retry has been rearched                                         */
/* -------------------------------------------------------------------- */
    const char *pszRetryDelay = CSLFetchNameValue( papszOptions, "RETRY_DELAY" );
    if( pszRetryDelay == NULL )
        pszRetryDelay = CPLGetConfigOption( "GDAL_HTTP_RETRY_DELAY", "30" );
    const char *pszMaxRetries = CSLFetchNameValue( papszOptions, "MAX_RETRY" );
    if( pszMaxRetries == NULL )
        pszMaxRetries = CPLGetConfigOption( "GDAL_HTTP_MAX_RETRY", "0" );
    int nRetryDelaySecs = atoi(pszRetryDelay);
    int nMaxRetries = atoi(pszMaxRetries);
    int nRetryCount = 0;
    bool bRequestRetry;

    do
    {
        bRequestRetry = FALSE;

/* -------------------------------------------------------------------- */
/*      Execute the request, waiting for results.                       */
/* -------------------------------------------------------------------- */
        psResult->nStatus = (int) curl_easy_perform( http_handle );

/* -------------------------------------------------------------------- */
/*      Fetch content-type if possible.                                 */
/* -------------------------------------------------------------------- */
        psResult->pszContentType = NULL;
        curl_easy_getinfo( http_handle, CURLINFO_CONTENT_TYPE,
                           &(psResult->pszContentType) );
        if( psResult->pszContentType != NULL )
            psResult->pszContentType = CPLStrdup(psResult->pszContentType);

/* -------------------------------------------------------------------- */
/*      Have we encountered some sort of error?                         */
/* -------------------------------------------------------------------- */
        if( strlen(szCurlErrBuf) > 0 )
        {
            int bSkipError = FALSE;

            /* Some servers such as http://115.113.193.14/cgi-bin/world/qgis_mapserv.fcgi?VERSION=1.1.1&SERVICE=WMS&REQUEST=GetCapabilities */
            /* invalidly return Content-Length as the uncompressed size, with makes curl to wait for more data */
            /* and time-out finally. If we got the expected data size, then we don't emit an error */
            /* but turn off GZip requests */
            if (bGZipRequested &&
                strstr(szCurlErrBuf, "transfer closed with") &&
                strstr(szCurlErrBuf, "bytes remaining to read"))
            {
                const char* pszContentLength =
                    CSLFetchNameValue(psResult->papszHeaders, "Content-Length");
                if (pszContentLength && psResult->nDataLen != 0 &&
                    atoi(pszContentLength) == psResult->nDataLen)
                {
                    const char* pszCurlGZIPOption = CPLGetConfigOption("CPL_CURL_GZIP", NULL);
                    if (pszCurlGZIPOption == NULL)
                    {
                        CPLSetConfigOption("CPL_CURL_GZIP", "NO");
                        CPLDebug("HTTP", "Disabling CPL_CURL_GZIP, because %s doesn't support it properly",
                                 pszURL);
                    }
                    psResult->nStatus = 0;
                    bSkipError = TRUE;
                }
            }
            if (!bSkipError)
            {
                psResult->pszErrBuf = CPLStrdup(szCurlErrBuf);
                CPLError( CE_Failure, CPLE_AppDefined,
                        "%s", szCurlErrBuf );
            }
        }
        else
        {
            /* HTTP errors do not trigger curl errors. But we need to */
            /* propagate them to the caller though */
            long response_code = 0;
            curl_easy_getinfo(http_handle, CURLINFO_RESPONSE_CODE, &response_code);

            if (response_code >= 400 && response_code < 600)
            {
                /* If HTTP 502, 503 or 504 gateway timeout error retry after a pause */
                if ((response_code >= 502 && response_code <= 504) && nRetryCount < nMaxRetries)
                {
                    CPLError(CE_Warning, CPLE_AppDefined,
                             "HTTP error code: %d - %s. Retrying again in %d secs",
                             (int)response_code, pszURL, nRetryDelaySecs);
                    CPLSleep(nRetryDelaySecs);
                    nRetryCount++;

                    CPLFree(psResult->pszContentType);
                    psResult->pszContentType = NULL;
                    CSLDestroy(psResult->papszHeaders);
                    psResult->papszHeaders = NULL;
                    CPLFree(psResult->pabyData);
                    psResult->pabyData = NULL;
                    psResult->nDataLen = 0;
                    psResult->nDataAlloc = 0;

                    bRequestRetry = TRUE;
                }
                else
                {
                    psResult->pszErrBuf = CPLStrdup(CPLSPrintf("HTTP error code : %d", (int)response_code));
                    CPLError( CE_Failure, CPLE_AppDefined, "%s", psResult->pszErrBuf );
                }
            }
        }
    }
    while (bRequestRetry);

    if (!pszPersistent)
        curl_easy_cleanup( http_handle );

    curl_slist_free_all(headers);

    return psResult;
#endif /* def HAVE_CURL */
}

#ifdef HAVE_CURL
/************************************************************************/
/*                         CPLHTTPSetOptions()                          */
/************************************************************************/

void CPLHTTPSetOptions(CURL *http_handle, char** papszOptions)
{
    if (CSLTestBoolean(CPLGetConfigOption("CPL_CURL_VERBOSE", "NO")))
        curl_easy_setopt(http_handle, CURLOPT_VERBOSE, 1);

    const char *pszHttpVersion = CSLFetchNameValue( papszOptions, "HTTP_VERSION");
    if( pszHttpVersion && strcmp(pszHttpVersion, "1.0") == 0 )
        curl_easy_setopt(http_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0 );

    /* Support control over HTTPAUTH */
    const char *pszHttpAuth = CSLFetchNameValue( papszOptions, "HTTPAUTH" );
    if( pszHttpAuth == NULL )
        pszHttpAuth = CPLGetConfigOption( "GDAL_HTTP_AUTH", NULL );
    if( pszHttpAuth == NULL )
        /* do nothing */;

    /* CURLOPT_HTTPAUTH is defined in curl 7.11.0 or newer */
#if LIBCURL_VERSION_NUM >= 0x70B00
    else if( EQUAL(pszHttpAuth,"BASIC") )
        curl_easy_setopt(http_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC );
    else if( EQUAL(pszHttpAuth,"NTLM") )
        curl_easy_setopt(http_handle, CURLOPT_HTTPAUTH, CURLAUTH_NTLM );
    else if( EQUAL(pszHttpAuth,"ANY") )
        curl_easy_setopt(http_handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY );
#ifdef CURLAUTH_GSSNEGOTIATE
    else if( EQUAL(pszHttpAuth,"NEGOTIATE") )
        curl_easy_setopt(http_handle, CURLOPT_HTTPAUTH, CURLAUTH_GSSNEGOTIATE );
#endif
    else
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Unsupported HTTPAUTH value '%s', ignored.", 
                  pszHttpAuth );
    }
#else
    else
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "HTTPAUTH option needs curl >= 7.11.0" );
    }
#endif

    /* Support use of .netrc - default enabled */
    const char *pszHttpNetrc = CSLFetchNameValue( papszOptions, "NETRC" );
    if( pszHttpNetrc == NULL )
        pszHttpNetrc = CPLGetConfigOption( "GDAL_HTTP_NETRC", "YES" );
    if( pszHttpNetrc == NULL || CSLTestBoolean(pszHttpNetrc) )
        curl_easy_setopt(http_handle, CURLOPT_NETRC, 1L);

    /* Support setting userid:password */
    const char *pszUserPwd = CSLFetchNameValue( papszOptions, "USERPWD" );
    if (pszUserPwd == NULL)
        pszUserPwd = CPLGetConfigOption("GDAL_HTTP_USERPWD", NULL);
    if( pszUserPwd != NULL )
        curl_easy_setopt(http_handle, CURLOPT_USERPWD, pszUserPwd );

    /* Set Proxy parameters */
    const char* pszProxy = CSLFetchNameValue( papszOptions, "PROXY" );
    if (pszProxy == NULL)
        pszProxy = CPLGetConfigOption("GDAL_HTTP_PROXY", NULL);
    if (pszProxy)
        curl_easy_setopt(http_handle,CURLOPT_PROXY,pszProxy);

    const char* pszProxyUserPwd = CSLFetchNameValue( papszOptions, "PROXYUSERPWD" );
    if (pszProxyUserPwd == NULL)
        pszProxyUserPwd = CPLGetConfigOption("GDAL_HTTP_PROXYUSERPWD", NULL);
    if (pszProxyUserPwd)
        curl_easy_setopt(http_handle,CURLOPT_PROXYUSERPWD,pszProxyUserPwd);

    /* Support control over PROXYAUTH */
    const char *pszProxyAuth = CSLFetchNameValue( papszOptions, "PROXYAUTH" );
    if( pszProxyAuth == NULL )
        pszProxyAuth = CPLGetConfigOption( "GDAL_PROXY_AUTH", NULL );
    if( pszProxyAuth == NULL )
        /* do nothing */;
    /* CURLOPT_PROXYAUTH is defined in curl 7.11.0 or newer */
#if LIBCURL_VERSION_NUM >= 0x70B00
    else if( EQUAL(pszProxyAuth,"BASIC") )
        curl_easy_setopt(http_handle, CURLOPT_PROXYAUTH, CURLAUTH_BASIC );
    else if( EQUAL(pszProxyAuth,"NTLM") )
        curl_easy_setopt(http_handle, CURLOPT_PROXYAUTH, CURLAUTH_NTLM );
    else if( EQUAL(pszProxyAuth,"DIGEST") )
        curl_easy_setopt(http_handle, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST );
    else if( EQUAL(pszProxyAuth,"ANY") )
        curl_easy_setopt(http_handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY );
    else
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Unsupported PROXYAUTH value '%s', ignored.", 
                  pszProxyAuth );
    }
#else
    else
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "PROXYAUTH option needs curl >= 7.11.0" );
    }
#endif

    /* Enable following redirections.  Requires libcurl 7.10.1 at least */
// START-PYXIS
//    curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1 );
//    curl_easy_setopt(http_handle, CURLOPT_MAXREDIRS, 10 );
    curl_easy_setopt(http_handle, CURLOPT_FOLLOWLOCATION, 1L );
    curl_easy_setopt(http_handle, CURLOPT_MAXREDIRS, 10L );
// END-PYXIS
    
    /* Set timeout.*/
    const char *pszTimeout = CSLFetchNameValue( papszOptions, "TIMEOUT" );
    if (pszTimeout == NULL)
        pszTimeout = CPLGetConfigOption("GDAL_HTTP_TIMEOUT", NULL);
    if( pszTimeout != NULL )
// START-PYXIS
//        curl_easy_setopt(http_handle, CURLOPT_TIMEOUT, atoi(pszTimeout) );
        curl_easy_setopt(http_handle, CURLOPT_TIMEOUT, (long) atoi(pszTimeout) );
// END-PYXIS

    /* Disable some SSL verification */
    const char *pszUnsafeSSL = CSLFetchNameValue( papszOptions, "UNSAFESSL" );
    if (pszUnsafeSSL == NULL)
        pszUnsafeSSL = CPLGetConfigOption("GDAL_HTTP_UNSAFESSL", NULL);
    if (pszUnsafeSSL != NULL && CSLTestBoolean(pszUnsafeSSL))
    {
        curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    /* Set Referer */
    const char *pszReferer = CSLFetchNameValue(papszOptions, "REFERER");
    if (pszReferer != NULL)
        curl_easy_setopt(http_handle, CURLOPT_REFERER, pszReferer);

    /* Set User-Agent */
    const char *pszUserAgent = CSLFetchNameValue(papszOptions, "USERAGENT");
    if (pszUserAgent == NULL)
        pszUserAgent = CPLGetConfigOption("GDAL_HTTP_USERAGENT", NULL);
    if (pszUserAgent != NULL)
        curl_easy_setopt(http_handle, CURLOPT_USERAGENT, pszUserAgent);

    /* NOSIGNAL should be set to true for timeout to work in multithread
     * environments on Unix, requires libcurl 7.10 or more recent.
     * (this force avoiding the use of signal handlers)
     */
#if LIBCURL_VERSION_NUM >= 0x070A00
// START-PYXIS
//    curl_easy_setopt(http_handle, CURLOPT_NOSIGNAL, 1 );
    curl_easy_setopt(http_handle, CURLOPT_NOSIGNAL, 1L );
// END-PYXIS
#endif

    /* Set POST mode */
    const char* pszPost = CSLFetchNameValue( papszOptions, "POSTFIELDS" );
    if( pszPost != NULL )
    {
        CPLDebug("HTTP", "These POSTFIELDS were sent:%.4000s", pszPost);
// START-PYXIS
//        curl_easy_setopt(http_handle, CURLOPT_POST, 1 );
        curl_easy_setopt(http_handle, CURLOPT_POST, 1L );
// END-PYXIS
        curl_easy_setopt(http_handle, CURLOPT_POSTFIELDS, pszPost );
    }

    const char* pszCustomRequest = CSLFetchNameValue( papszOptions, "CUSTOMREQUEST" );
    if( pszCustomRequest != NULL )
    {
        curl_easy_setopt(http_handle, CURLOPT_CUSTOMREQUEST, pszCustomRequest );
    }
    
    const char* pszCookie = CSLFetchNameValue(papszOptions, "COOKIE");
    if (pszCookie == NULL)
        pszCookie = CPLGetConfigOption("GDAL_HTTP_COOKIE", NULL);
    if (pszCookie != NULL)
        curl_easy_setopt(http_handle, CURLOPT_COOKIE, pszCookie);
}
#endif /* def HAVE_CURL */

/************************************************************************/
/*                           CPLHTTPEnabled()                           */
/************************************************************************/

/**
 * \brief Return if CPLHTTP services can be useful
 *
 * Those services depend on GDAL being build with libcurl support.
 *
 * @return TRUE if libcurl support is enabled
 */
int CPLHTTPEnabled()

{
#ifdef HAVE_CURL
    return TRUE;
#else
    return FALSE;
#endif
}

/************************************************************************/
/*                           CPLHTTPCleanup()                           */
/************************************************************************/

/**
 * \brief Cleanup function to call at application termination
 */
void CPLHTTPCleanup()

{
#ifdef HAVE_CURL
    if( !hSessionMapMutex )
        return;

    {
        CPLMutexHolder oHolder( &hSessionMapMutex );
        std::map<CPLString,CURL*>::iterator oIt;
        if( poSessionMap )
        {
            for( oIt=poSessionMap->begin(); oIt != poSessionMap->end(); oIt++ )
                curl_easy_cleanup( oIt->second );
            delete poSessionMap;
            poSessionMap = NULL;
        }
    }

    // not quite a safe sequence. 
    CPLDestroyMutex( hSessionMapMutex );
    hSessionMapMutex = NULL;
#endif
}

/************************************************************************/
/*                        CPLHTTPDestroyResult()                        */
/************************************************************************/

/**
 * \brief Clean the memory associated with the return value of CPLHTTPFetch()
 *
 * @param psResult pointer to the return value of CPLHTTPFetch()
 */
void CPLHTTPDestroyResult( CPLHTTPResult *psResult )

{
    if( psResult )
    {
        CPLFree( psResult->pabyData );
        CPLFree( psResult->pszErrBuf );
        CPLFree( psResult->pszContentType );
        CSLDestroy( psResult->papszHeaders );

        int i;
        for(i=0;i<psResult->nMimePartCount;i++)
        {
            CSLDestroy( psResult->pasMimePart[i].papszHeaders );
        }
        CPLFree(psResult->pasMimePart);
        
        CPLFree( psResult );
    }
}

/************************************************************************/
/*                     CPLHTTPParseMultipartMime()                      */
/************************************************************************/

/**
 * \brief Parses a a MIME multipart message
 *
 * This function will iterate over each part and put it in a separate
 * element of the pasMimePart array of the provided psResult structure.
 *
 * @param psResult pointer to the return value of CPLHTTPFetch()
 * @return TRUE if the message contains MIME multipart message.
 */
int CPLHTTPParseMultipartMime( CPLHTTPResult *psResult )

{
/* -------------------------------------------------------------------- */
/*      Is it already done?                                             */
/* -------------------------------------------------------------------- */
    if( psResult->nMimePartCount > 0 )
        return TRUE;

/* -------------------------------------------------------------------- */
/*      Find the boundary setting in the content type.                  */
/* -------------------------------------------------------------------- */
    const char *pszBound = NULL;

    if( psResult->pszContentType != NULL )
        pszBound = strstr(psResult->pszContentType,"boundary=");

// START-PYXIS
	if( pszBound == NULL )
	{
		//this is a hack when the ContentType is multi-line libcurl doesn't parse the second line.
		//so we try to find it inside the data (which should not work - but servers seem to do it).
		pszBound = strstr((char*)psResult->pabyData,"boundary=");
	}
// END-PYXIS

	if( pszBound == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Unable to parse multi-part mime, no boundary setting." );
        return FALSE;
    }

    CPLString osBoundary;
// START-PYXIS
//    char **papszTokens = 
//        CSLTokenizeStringComplex( pszBound + 9, "\n ;", 
//                                  TRUE, FALSE );
    char **papszTokens = 
        CSLTokenizeStringComplex( pszBound + 9, "\r\n ;", 
                                  TRUE, FALSE );
// END-PYXIS

    if( CSLCount(papszTokens) == 0 || strlen(papszTokens[0]) == 0 )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Unable to parse multi-part mime, boundary not parsable." );
        CSLDestroy( papszTokens );
        return FALSE;
    }
    
    osBoundary = "--";
    osBoundary += papszTokens[0];
    CSLDestroy( papszTokens );

/* -------------------------------------------------------------------- */
/*      Find the start of the first chunk.                              */
/* -------------------------------------------------------------------- */
    char *pszNext;
    pszNext = (char *) 
        strstr((const char *) psResult->pabyData,osBoundary.c_str());
    
    if( pszNext == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "No parts found." );
        return FALSE;
    }

    pszNext += strlen(osBoundary);
    while( *pszNext != '\n' && *pszNext != '\r' && *pszNext != '\0' )
        pszNext++;
    if( *pszNext == '\r' )
        pszNext++;
    if( *pszNext == '\n' )
        pszNext++;

/* -------------------------------------------------------------------- */
/*      Loop over parts...                                              */
/* -------------------------------------------------------------------- */
    while( TRUE )
    {
        psResult->nMimePartCount++;
        psResult->pasMimePart = (CPLMimePart *)
            CPLRealloc(psResult->pasMimePart,
                       sizeof(CPLMimePart) * psResult->nMimePartCount );

        CPLMimePart *psPart = psResult->pasMimePart+psResult->nMimePartCount-1;

        memset( psPart, 0, sizeof(CPLMimePart) );

/* -------------------------------------------------------------------- */
/*      Collect headers.                                                */
/* -------------------------------------------------------------------- */
        while( *pszNext != '\n' && *pszNext != '\r' && *pszNext != '\0' )
        {
            char *pszEOL = strstr(pszNext,"\n");

            if( pszEOL == NULL )
            {
                CPLError(CE_Failure, CPLE_AppDefined,
                         "Error while parsing multipart content (at line %d)", __LINE__);
                return FALSE;
            }

            *pszEOL = '\0';
            int bRestoreAntislashR = FALSE;
            if (pszEOL - pszNext > 1 && pszEOL[-1] == '\r')
            {
                bRestoreAntislashR = TRUE;
                pszEOL[-1] = '\0';
            }
            psPart->papszHeaders = 
                CSLAddString( psPart->papszHeaders, pszNext );
            if (bRestoreAntislashR)
                pszEOL[-1] = '\r';
            *pszEOL = '\n';
            
            pszNext = pszEOL + 1;
        }

        if( *pszNext == '\r' )
            pszNext++;
        if( *pszNext == '\n' )
            pszNext++;
            
/* -------------------------------------------------------------------- */
/*      Work out the data block size.                                   */
/* -------------------------------------------------------------------- */
        psPart->pabyData = (GByte *) pszNext;

        int nBytesAvail = psResult->nDataLen - 
            (pszNext - (const char *) psResult->pabyData);

        while( nBytesAvail > 0
               && (*pszNext != '-' 
                   || strncmp(pszNext,osBoundary,strlen(osBoundary)) != 0) )
        {
            pszNext++;
            nBytesAvail--;
        }
        
        if( nBytesAvail == 0 )
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                        "Error while parsing multipart content (at line %d)", __LINE__);
            return FALSE;
        }

        psPart->nDataLen = pszNext - (const char *) psPart->pabyData;
        pszNext += strlen(osBoundary);

        if( strncmp(pszNext,"--",2) == 0 )
        {
            break;
        }

        if( *pszNext == '\r' )
            pszNext++;
        if( *pszNext == '\n' )
            pszNext++;
        else
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                        "Error while parsing multipart content (at line %d)", __LINE__);
            return FALSE;
        }
    }

    return TRUE;
}
