#include "force_include.h" 
#include "gdal_pam.h"
#include <fstream>
#include <string>
#include <exception>
#include "boost/asio.hpp"
#include "pyxlib_instance.h" 
#include "pyxis/derm/index.h"
#include "pyxis/data/coverage.h" 
#include "pyxis/procs/cache.h" 
#include "pyxis/derm/snyder_projection.h" 
#include "pyxis/pipe/pipe_manager.h" 
using namespace std;
CPL_C_START
__declspec(dllexport) void GDALRegister_PYXIS(void);
CPL_C_END
class PYXISDataset:public GDALPamDataset {
    friend class PYXISRasterBand;
    int         bGeoTransformValid;
    double      adfGeoTransform[6];
    int         pyxisResolution;
    boost::intrusive_ptr<ICoverage> spCov;
    boost::intrusive_ptr<IProcess> spProc;
    static int count;
public:
    PYXISDataset();
    ~PYXISDataset();
    static GDALDataset  *Open(GDALOpenInfo *);
    CPLErr              GetGeoTransform(double*);
    virtual const char  *GetProjectionRef(void);
}; int PYXISDataset::count=0;
class PYXISRasterBand:public GDALPamRasterBand {
public:
    friend PYXISDataset;
    PYXISRasterBand(int,PYXISDataset *);
	~PYXISRasterBand();
    virtual CPLErr IReadBlock(int,int,void *);
	char *pBandCache;
};
PYXISRasterBand::PYXISRasterBand(int nBand,PYXISDataset *poDS){
    this->poDS=poDS;
    this->nBand=nBand;
    eDataType=GDT_Byte;
    nBlockXSize=poDS->nRasterXSize;
    nBlockYSize=1;
	pBandCache=NULL;
}
PYXISRasterBand::~PYXISRasterBand() {
    try {
		if (pBandCache!=NULL) CPLFree(pBandCache);
    } catch(...){}
}
CPLErr PYXISRasterBand::IReadBlock(int nBlockXOff,int nBlockYOff,void *pData){
	try {
		if (nBand!=1)
		{
			memcpy(pData,pBandCache,sizeof(char)*nBlockXSize);
		}
		else
		{
			PYXISDataset *pyxDS=(PYXISDataset *)poDS;
			PYXISRasterBand * poBand2 = (PYXISRasterBand *)pyxDS->GetRasterBand(2);
			PYXISRasterBand * poBand3 = (PYXISRasterBand *)pyxDS->GetRasterBand(3);
			if (poBand2->pBandCache!=NULL) CPLFree(poBand2->pBandCache);
			if (poBand3->pBandCache!=NULL) CPLFree(poBand3->pBandCache);
			poBand2->pBandCache = (char *) CPLMalloc(nBlockXSize);
			poBand3->pBandCache = (char *) CPLMalloc(nBlockXSize);
			const SnyderProjection* pSnyder=SnyderProjection::getInstance();
			double lat=pyxDS->adfGeoTransform[3]-(nBlockYOff*pyxDS->adfGeoTransform[5]);
			for (int x=nBlockXOff;x<pyxDS->nRasterXSize;x++) {
				CoordLatLon ll;
				double lon=pyxDS->adfGeoTransform[0]+(x*pyxDS->adfGeoTransform[1]);
				ll.setInDegrees(lat,lon);
				PYXIcosIndex i;
				pSnyder->nativeToPYXIS(ll,&i,pyxDS->pyxisResolution);
				PYXValue rgb=pyxDS->spCov->getCoverageValue(i);
				if(rgb.isNull()){
					((char *)pData)[x]=255;
					((char *)poBand2->pBandCache )[x]=255;
					((char *)poBand3->pBandCache )[x]=255;
				}
				else {
					((char *)pData)[x]=rgb.getUInt8(0);
					((char *)poBand2->pBandCache )[x]=rgb.getUInt8(1);
					((char *)poBand3->pBandCache )[x]=rgb.getUInt8(2);
				}
			}
		}
		return CE_None;
	} catch(...){return CE_Failure;}
}
PYXISDataset::PYXISDataset(){
    bGeoTransformValid=FALSE;
}
PYXISDataset::~PYXISDataset() {
    try {
        if (--PYXISDataset::count==0) PYXLibInstance::uninitialize();
    } catch(...){}
}
const char *PYXISDataset::GetProjectionRef(){return "GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.0174532925199433]]";}
CPLErr PYXISDataset::GetGeoTransform(double * padfTransform) {
    try {
        if(bGeoTransformValid) {
            memcpy(padfTransform,adfGeoTransform,sizeof(adfGeoTransform[0]) * 6);
            return CE_None;
        }
    } catch(...){}
    return CE_Failure;
}
GDALDataset *PYXISDataset::Open(GDALOpenInfo * poOpenInfo) {
    PYXISDataset *poDS;
    try {
		std::string strFilePath(poOpenInfo->pszFilename);
		if (strFilePath.find(".pgd")==string::npos) {return NULL;} // Must return null, for when GDAL is searching for the needed driver.
        fstream pgdFile(poOpenInfo->pszFilename,ios::in);
        if (!pgdFile.is_open()) {throw "pgd file error";}
        string str,str2;getline(pgdFile,str);getline(pgdFile,str2);
        if (PYXISDataset::count++==0) PYXLibInstance::initialize("pyxisgdaldriver",str.c_str(),str2.c_str());
        getline(pgdFile,str2);
        poDS=new PYXISDataset();
        getline(pgdFile,str);poDS->nRasterXSize=atoi(str.c_str()); 
        getline(pgdFile,str);poDS->nRasterYSize=atoi(str.c_str());
        getline(pgdFile,str);poDS->pyxisResolution=atoi(str.c_str());
        getline(pgdFile,str);poDS->adfGeoTransform[0]=atof(str.c_str());
        getline(pgdFile,str);poDS->adfGeoTransform[3]=atof(str.c_str());
        getline(pgdFile,str);double LRLon=atof(str.c_str());
        poDS->adfGeoTransform[1]=fabs((LRLon-poDS->adfGeoTransform[0])/poDS->nRasterXSize);
        getline(pgdFile,str);double LRLat=atof(str.c_str());
        poDS->adfGeoTransform[5]=fabs((LRLat-poDS->adfGeoTransform[3])/poDS->nRasterYSize);
        poDS->adfGeoTransform[2]=poDS->adfGeoTransform[4]=0;
        pgdFile.close();
        poDS->bGeoTransformValid=TRUE;
        poDS->eAccess=GA_ReadOnly;
        poDS->spProc=PipeManager::readPipelineFromFile(str2);
        poDS->spProc->QueryInterface(ICoverage::iid,(void**) &(poDS->spCov));
		if (!poDS->spCov){throw "pgd input coverage did not resolve";}
		boost::intrusive_ptr<ICache> spCache;
        poDS->spProc->QueryInterface(ICache::iid,(void**) &(spCache));
		if (spCache)
		{
			spCache->setGreedyCache(true);
		}
        poDS->nBands=3;
        for(int iBand=1;iBand<=poDS->nBands;iBand++) {
            poDS->SetBand(iBand,new PYXISRasterBand(iBand,poDS));
        }
    }
    catch(char *ex){poDS = NULL;}
    return poDS;
}
void GDALRegister_PYXIS() {
    if(GDALGetDriverByName("PYXIS")==NULL) {
        GDALDriver *poDriver=new GDALDriver();
        poDriver->SetDescription("PYXIS");
        poDriver->SetMetadataItem(GDAL_DMD_LONGNAME,"PYXIS Innovation Inc");
        poDriver->SetMetadataItem(GDAL_DMD_HELPTOPIC,"frmt_PYXIS.html");
        poDriver->SetMetadataItem(GDAL_DMD_EXTENSION,"pgd");
        poDriver->pfnOpen=PYXISDataset::Open;
        GetGDALDriverManager()->RegisterDriver(poDriver);
    }
}