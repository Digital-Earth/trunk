/******************************************************************************
gdalinfodll.cpp

begin		: 2009-07-29
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition
// windows includes
#include <tchar.h>
#include <windows.h>
#pragma warning(pop)

#include <string>
#include <vector>

//--
//-- GDAL API includes
//--
#include "gdal.h"
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: class needs to have dll-interface to be used by clients of class
#include "gdal_priv.h"
#pragma warning(pop)
#include "cpl_string.h"

#include "gdalinfodll.h"

//---------------------------------------------------------------------
//--
//-- functions in the following class are used to enable access to the
//-- GDAL api.
//--
//---------------------------------------------------------------------

class GdalInfo : public IGdalInfo
{
public:
	GdalInfo()
	{
	}

	~GdalInfo()
	{
	}

	virtual void STDMETHODCALLTYPE Init( void ) const
	{
		GDALAllRegister();
	}

	virtual void STDMETHODCALLTYPE ShutDown( void ) const 
	{
		//--
		//-- initially thought it would be a good idea to do a GDAL cleanup,
		//-- but this causes errors with WorldView if it uses the GDAL library also.
		//--
		//GDALDestroyDriverManager();
	}

	virtual std::string STDMETHODCALLTYPE VersionName( void ) const
	{
		return std::string(GDALVersionInfo("VERSION_NAME"));
	}

	virtual std::string STDMETHODCALLTYPE VersionNumber( void ) const
	{
		return std::string(GDALVersionInfo("VERSION_NUM"));
	}

	virtual bool STDMETHODCALLTYPE driverSupported( std::string driverName ) const 
	{
		return GDALGetDriverByName(driverName.c_str()) != (GDALDriverH) 0;
	}

	//-- 
	//-- returns true if resident GDAL library supports Oracle.
	//--
	virtual bool STDMETHODCALLTYPE oracleSupported( void ) const 
	{
		return driverSupported("georaster");
	}

	//--
	//-- open a datasource, and return all subdataset metatdata with out 
	//-- processing.  used primarily for debuging. testing, and verification.
	//--
	virtual std::vector<std::string> STDMETHODCALLTYPE getSubDataSetsRaw(std::string arg) const
	{
		std::vector<std::string> subDataSets;

		GDALDatasetH hDataSet = GDALOpen( arg.c_str(), GA_ReadOnly );
		if( hDataSet )
		{
			char** pMetadata = GDALGetMetadata( hDataSet, "SUBDATASETS" );
			if( CSLCount(pMetadata) > 0 )
			{
				for( int nIdx = 0; pMetadata[nIdx] != NULL; nIdx++ )
				{
					subDataSets.push_back(pMetadata[nIdx]);
				}
			}

			GDALClose( hDataSet );
		}

		return subDataSets;
	}

	//--
	//-- open a datasource, and traverse the metadata for subdatasets, 
	//-- parsing out the name and description information for each
	//-- matching subdataset returned by the original query.
	//--
	virtual std::vector<GdalSubSetNode*> STDMETHODCALLTYPE getSubDataSetsNodes(std::string arg) const
	{
		std::vector<GdalSubSetNode*> subDataSets;

		GDALDatasetH hDataSet = GDALOpen( arg.c_str(), GA_ReadOnly );
		if( hDataSet )
		{
			char** pMetadata = GDALGetMetadata( hDataSet, "SUBDATASETS" );
			if( CSLCount(pMetadata) > 0 )
			{
				GdalSubSetNode* node = (GdalSubSetNode*) 0;

				for( int nIdx = 0; pMetadata[nIdx] != NULL; nIdx++ )
				{
					std::string metadata = pMetadata[nIdx];
					int nPos = metadata.find('=');
	
					std::string lhs = metadata.substr(0,nPos);
					std::string rhs = metadata.substr(nPos+1);

					nPos = lhs.find_last_of('_');
					lhs = lhs.substr(nPos+1);

					if( lhs == "NAME" )
					{
						node = new GdalSubSetNode();
						node->m_sName = rhs;
					}
					else if( lhs == "DESC" )
					{
						node->m_sDesc = rhs;
						subDataSets.push_back(node);
						node =  (GdalSubSetNode*) 0;
					}
				}
			}

			GDALClose( hDataSet );
		}

		return subDataSets;
	}

	//--
	//-- return list of all oracle raster datasets.  
	//-- requires three types of API calls into database.
	//--
	//-- a. first call returns list of spatial tables.
	//-- b. second call returns list of georaster columns in a table, needs to be called
	//--    once for every spatial table.
	//-- c. third call returns list of raster tables and ids, for a spatial table and 
	//--    georaster column pair.  needs to be called for every table/column pair.
	//--
	//-- worse case, three API calls for every raster datasource available.
	//--
	virtual std::vector<GdalSubSetNode*> STDMETHODCALLTYPE getAllOracleRasterDataSets( std::string user, std::string password, std::string sid ) const
	{
		std::vector<GdalSubSetNode*> nodeList;

		std::string connectString = "georaster:" + user + "," + password + "," + sid;

		std::vector<GdalSubSetNode*> tableList = getSubDataSetsNodes(connectString);				
		std::vector<GdalSubSetNode*>::iterator tableIter;
		for( tableIter = tableList.begin(); tableIter != tableList.end(); tableIter++)
		{
			GdalSubSetNode* tableNode = *tableIter;
			if( tableNode )
			{
				std::vector<GdalSubSetNode*> columnList = getSubDataSetsNodes(tableNode->m_sName);				
				std::vector<GdalSubSetNode*>::iterator columnIter;
				for( columnIter = columnList.begin(); columnIter != columnList.end(); columnIter++)
				{
					GdalSubSetNode* columnNode = *columnIter;
					if( columnNode )
					{
						std::vector<GdalSubSetNode*> rasterList = getSubDataSetsNodes(columnNode->m_sName);				
						std::vector<GdalSubSetNode*>::iterator rasterIter;
						for( rasterIter = rasterList.begin(); rasterIter != rasterList.end(); rasterIter++)
						{
							GdalSubSetNode* rasterNode = *rasterIter;
							if( rasterNode )
							{
								nodeList.push_back(rasterNode);
							}
						}
					}
				}
				releaseSubDataSetsNodes(columnList);
			}
		}	
		releaseSubDataSetsNodes(tableList);
	
		return nodeList;
	}

	//-- 
	//-- release nodes previously created by functions in this DLL.
	//--
	virtual void STDMETHODCALLTYPE releaseSubDataSetsNodes(std::vector<GdalSubSetNode*> nodeList) const
	{
		std::vector<GdalSubSetNode*>::iterator iter;
		for(iter = nodeList.begin(); iter != nodeList.end(); iter++)
		{
			GdalSubSetNode* node = *iter;
			if( node )
			{
				delete node;
			}
		}	
		nodeList.clear();
	}

	//--
	//-- build VRT file by calling "CreateCopy" method from target datasource type's driver 
	//--
	virtual void STDMETHODCALLTYPE buildVrt( std::string filename, std::string connectString ) const
	{
		GDALDataset* pSrcDataSet = (GDALDataset*)GDALOpen( connectString.c_str(), GA_ReadOnly );
		GDALDriver* pDriver = GetGDALDriverManager()->GetDriverByName("VRT");
		
		if( pSrcDataSet != 0 && pDriver != 0 )
		{
			pDriver->CreateCopy(filename.c_str(),pSrcDataSet,1,0,0,0);
		}

		if( pSrcDataSet != 0 )
		{
			GDALClose((GDALDatasetH)pSrcDataSet);
		}
	}

};

//---------------------------------------------------------------------
//--
//-- magic here.  declare a single static object, and a function that 
//-- returns the address of that object.  the function is exposed and
//-- exported from the dll.  outside of the dll, the object's pointer,
//-- as returned by the function, can be used to access all the  
//-- functions exposed by the interface structure.
//--
//---------------------------------------------------------------------

static GdalInfo g_GdalInfoNode;

extern "C" __declspec(dllexport) IGdalInfo *GetGdalInfo(void)
{
	return &g_GdalInfoNode;
}


