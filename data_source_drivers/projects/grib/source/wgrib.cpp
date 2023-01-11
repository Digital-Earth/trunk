#include "stdafx.h"
#define GRIB_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <float.h>

#include <vector>

#include "grib_record.h"
#include "wgrib_defines.c"
#include "wgrib_tables.c"

#pragma warning(disable: 4244) // ignore size conversion errors
#pragma warning(disable: 4267) // ignore size conversion errors
#pragma warning(disable: 4996) // ignore security warnings for sprintf

int readRecord(
	GribRecord* pRecord,
	FILE* input,
	long int& pos,
	long int& len_grib,
	unsigned char** pBuffer,
	long int& buffer_size,
	bool bReadData	);

unsigned char *seek_grib(FILE *file, long *pos, long *len_grib, 
        unsigned char *buffer, unsigned int buf_len);

int read_grib(FILE *file, long pos, long len_grib, unsigned char *buffer);

double ibm2flt(unsigned char *ibm);
 
void BDS_unpack(std::vector<double>& vecData, unsigned char *bds, unsigned char *bitmap,
        int n_bits, int n, double ref, double scale);

double int_power(double x, int y);

void levels(char*, int, int, int);
 
void PDStimes(char* szTimeDesc, int time_range, int p1, int p2, int time_unit);

int GDS_grid(unsigned char *gds, unsigned char *bds, int *nx, int *ny, 
             long int *nxny);

int PDS_date(unsigned char *pds, int option, int verf_time);

int verf_time(unsigned char *pds, int *year, int *month, int *day, int *hour);

char *k5toa(unsigned char *pds);
char *k5_comments(unsigned char *pds);
int setup_user_table(int center, int subcenter, int ptable);

extern  struct ParmTable parm_table_ncep_opn[256];
extern  struct ParmTable parm_table_ncep_reanal[256];
extern  struct ParmTable parm_table_nceptab_129[256];
extern  struct ParmTable parm_table_omb[256];
extern  struct ParmTable parm_table_nceptab_130[256];
extern  struct ParmTable parm_table_nceptab_131[256];

extern  struct ParmTable parm_table_ecmwf_128[256];
extern  struct ParmTable parm_table_ecmwf_129[256];
extern  struct ParmTable parm_table_ecmwf_130[256];
extern  struct ParmTable parm_table_ecmwf_131[256];
extern  struct ParmTable parm_table_ecmwf_140[256];
extern  struct ParmTable parm_table_ecmwf_150[256];
extern  struct ParmTable parm_table_ecmwf_151[256];
extern  struct ParmTable parm_table_ecmwf_160[256];
extern  struct ParmTable parm_table_ecmwf_170[256];
extern  struct ParmTable parm_table_ecmwf_180[256];
extern  struct ParmTable parm_table_ecmwf_190[256];
extern  struct ParmTable parm_table_user[256];
extern  struct ParmTable parm_table_dwd_002[256];
extern  struct ParmTable parm_table_dwd_201[256];
extern  struct ParmTable parm_table_dwd_202[256];
extern  struct ParmTable parm_table_dwd_203[256];
extern  struct ParmTable parm_table_cptec_254[256];

extern enum Def_NCEP_Table def_ncep_table;

extern int ncep_ens;

//int add_time(int *year, int *month, int *day, int *hour, int dtime, int unit);

//void GDS_prt_thin_lon(unsigned char *gds);

//void GDS_winds(unsigned char *gds, int verbose);
//int missing_points(unsigned char *bitmap, int n);

//void EC_ext(unsigned char *pds, char *prefix, char *suffix, int verbose);
//int flt2ieee(float x, unsigned char *ieee);

//int wrtieee(float *array, int n, int header, FILE *output);
//int wrtieee_header(unsigned int n, FILE *output);


double getUndefinedValue()
{
	return UNDEFINED;
}


/*
Read record specified in pRecord into the pRecord structure.
nRecordNumber and strFileName have to be set before calling this function.
*/
int openGRIB(GribRecord* pRecord)
{

    unsigned char *buffer;
    int i;
    long int len_grib, pos = 0, buffer_size, count = 1;
    unsigned char *msg; // *pds, *gds, *bms, *bds, *pointer;
    FILE *input;

	// Open file.
	if ((input = fopen(pRecord->getFileName().c_str(),"rb")) == NULL)
	{
        fprintf(stderr,"could not open file: %s\n", pRecord->getFileName().c_str());
        return -1;
    }

	buffer = (unsigned char *) malloc(BUFF_ALLOC0);
    
	if (buffer == 0)
	{
		fprintf(stderr,"not enough memory\n");
		return -2;
    }

	buffer_size = BUFF_ALLOC0;

    // Search for the record number we wish to load.
    for (i = 1; i < pRecord->getRecordNumber(); i++)
	{
		msg = seek_grib(input, &pos, &len_grib, buffer, MSEEK);
		if (msg == NULL)
		{
			free (buffer);
			fprintf(stderr, "ran out of data or bad file\n");
			return -3;
		}
		pos += len_grib;
    }

	int nResult = readRecord(pRecord, input, pos, len_grib, &buffer, buffer_size, true);

    fclose(input);

	free(buffer);

	return nResult;
}


int readRecord(GribRecord* pRecord, FILE* input,
			   long int& pos,
			   long int& len_grib,
			   unsigned char** pBuffer,
			   long int& buffer_size,
			   bool bReadData)
{
    int i, nx, ny;
	long int nxny;
    unsigned char *msg, *pds, *gds, *bms, *bds, *pointer;
    double temp, rmin, rmax;

	// Look for start of record.  Return pos and len_grib
	msg = seek_grib(input, &pos, &len_grib, *pBuffer, MSEEK);

	/* parse grib message */
	msg = *pBuffer;
    pds = (msg + 8);
    pointer = pds + PDS_LEN(pds);

	if (PDS_HAS_GDS(pds))
	{
        gds = pointer;
        pointer += GDS_LEN(gds);
    }
    else
	{
        gds = NULL;
    }

	// Check record type.  If not correct type, exit.
	if (msg == NULL || gds == 0 || !GDS_LatLon(gds))
	{
		// Invalid file.
		return -10;
	}
	else
	{
		// Get description of data contents of this record.
		char szLevel[100];
		levels(szLevel, PDS_KPDS6(pds),PDS_KPDS7(pds),PDS_Center(pds));
		
		char szItemAndUnits[100];
		sprintf(szItemAndUnits,"%s, %s", k5toa(pds), k5_comments(pds));

		char szTimeDesc[100];
		szTimeDesc[0] = 0;
		PDStimes(szTimeDesc, PDS_TimeRange(pds),PDS_P1(pds),PDS_P2(pds),
					PDS_ForecastTimeUnit(pds));

		std::string strDesc(szItemAndUnits);
		strDesc.append(",");
		strDesc.append(szLevel);
		if (szTimeDesc[0] != 0)
		{
			strDesc.append(",");
			strDesc.append(szTimeDesc);
		}
		pRecord->setDescription(strDesc);
	}

    /* Allocate buffer to hold data*/
    if (len_grib + msg - *pBuffer > buffer_size)
	{
        buffer_size = len_grib + msg - *pBuffer + 1000;
        *pBuffer = (unsigned char *) realloc((void *) *pBuffer, buffer_size);
        if (*pBuffer == NULL)
		{
            fprintf(stderr,"ran out of memory\n");
			return -4;
        }
    }

	// Read record from file.
    read_grib(input, pos, len_grib, *pBuffer);

	/* parse grib message */

	// Since buffer may have been reallocated above, we need to set these again.
	msg = *pBuffer;
    pds = (msg + 8);
    pointer = pds + PDS_LEN(pds);
    if (PDS_HAS_GDS(pds))
	{
        gds = pointer;
        pointer += GDS_LEN(gds);
    }
    else
	{
        gds = NULL;
    }

    if (PDS_HAS_BMS(pds))
	{
        bms = pointer;
        pointer += BMS_LEN(bms);
	}
    else 
	{
        bms = NULL;
    }

    bds = pointer;
    pointer += BDS_LEN(bds);

	/* figure out size of array */
	if (gds != NULL)
	{
	    GDS_grid(gds, bds, &nx, &ny, &nxny);
	}
	else if (bms != NULL)
	{
	    nxny = nx = BMS_nxny(bms);
	    ny = 1;
	}
	else
	{
	    if (BDS_NumBits(bds) == 0)
		{
                nxny = nx = 1;
                fprintf(stderr,"Missing GDS, constant record .. cannot "
                    "determine number of data points\n");
	    }
	    else
		{
	        nxny = nx = BDS_NValues(bds);
	    }
	    ny = 1;
	}

	pRecord->setLonMin(0.001*GDS_LatLon_Lo1(gds));
	pRecord->setLonMax(0.001*GDS_LatLon_Lo2(gds));

	pRecord->setLatMin(0.001*GDS_LatLon_La1(gds));
	pRecord->setLatMax(0.001*GDS_LatLon_La2(gds));

	pRecord->setLatStep(0.001*GDS_LatLon_dy(gds));
	pRecord->setLonStep(0.001*GDS_LatLon_dx(gds));

	pRecord->setBandXSize(nx);
	pRecord->setBandYSize(ny);

	int nGDSLen = GDS_LEN(gds);

	// Read data from file and place in GRIB record.
	if (bReadData)
	{
		// Size vector for number of data items.
		pRecord->getDataVector().reserve(nxny);

		temp = int_power(10.0, - PDS_DecimalScale(pds));

		// Unpack the data and place in array.
		BDS_unpack(pRecord->getDataVector(), bds, BMS_bitmap(bms), BDS_NumBits(bds), nxny,
				temp*BDS_RefValue(bds),temp*int_power(2.0, BDS_BinScale(bds)));
	}

	// Data is read from bottom to top.
	pRecord->setBottomUp();

	/*
	Check to see if data source has data from top to bottom instead of 
	the normal bottom to top.  If so, flip it.
	*/
	if (pRecord->getLatMin() > pRecord->getLatMax())
	{
		pRecord->setBottomUp(false);
		// Switch min and max lat.
		double fTemp = pRecord->getLatMin();
		pRecord->setLatMin(pRecord->getLatMax());
		pRecord->setLatMax(fTemp);
	}

	/*
	If dataset starts at date line (which is considered 180, not -180
	and continues east - adjust start to -180.0
	*/
	if (pRecord->getLonMin() == 180.0 && pRecord->getLonMin() > pRecord->getLonMax())
	{
		pRecord->setLonMin(-180.0);
	}

	// Get min/max data values for this record.
	{
		rmin = FLT_MAX;
		rmax = -FLT_MAX;

		if (bReadData)
		{
			for (i = 0; i < nxny; i++) 
			{
				if (pRecord->getDataVector()[i] != getUndefinedValue()) 
				{
					rmin = min(rmin,pRecord->getDataVector()[i]);
					rmax = max(rmax,pRecord->getDataVector()[i]);
				}
			}
		}

		pRecord->setMinValue(rmin);
		pRecord->setMaxValue(rmax);
	}
	return 1;
}


// Read record specified in pRecord into the pRecord structure.
int getGRIBInfo(const std::string& strFileName, std::vector<GribRecord>& vecInfo)
{
    unsigned char *buffer;
    long int len_grib, pos = 0, buffer_size, count = 1;
    unsigned char *msg; //, *pds, *gds, *bms, *bds, *pointer;
    FILE *input;

	// Open file.
	if ((input = fopen(strFileName.c_str(),"rb")) == NULL)
	{
        fprintf(stderr,"could not open file: %s\n", strFileName.c_str());
        return -1;
    }

	buffer = (unsigned char *) malloc(BUFF_ALLOC0);
    
	if (buffer == 0)
	{
		fprintf(stderr,"not enough memory\n");
		return -2;
    }

	buffer_size = BUFF_ALLOC0;

	int nCount = 0;
    // Search for the record number we wish to load.
	do
	{
		msg = seek_grib(input, &pos, &len_grib, buffer, MSEEK);
		if (msg == NULL)
		{
			// Reached end of file.  Exit from loop.
			break;
		}
		pos += len_grib;

		GribRecord gribRecord;

		int nResult = readRecord(	&gribRecord,
									input,
									pos,
									len_grib,
									&buffer,
									buffer_size,
									false); // We don't want to read data.

		// Was there an error during reading? If so, exit.
		if (nResult <= 0)
		{
			break;
		}

		gribRecord.setFileName(strFileName);
		gribRecord.setRecordNumber(nCount);
		++nCount;
		vecInfo.push_back(gribRecord);
	}
	while (true);

    fclose(input);

	free(buffer);

	// Return number of records read.
	return nCount;
}


/*
 * find next grib header
 *
 * file = what do you think?
 * pos = initial position to start looking at  ( = 0 for 1st call)
 *       returns with position of next grib header (units=bytes)
 * len_grib = length of the grib record (bytes)
 * buffer[buf_len] = buffer for reading/writing
 *
 * returns (char *) to start of GRIB header+PDS
 *         NULL if not found
 *
 * adapted from SKGB (Mark Iredell)
 *
 * v1.1 9/94 Wesley Ebisuzaki
 * v1.2 3/96 Wesley Ebisuzaki handles short records at end of file
 * v1.3 8/96 Wesley Ebisuzaki increase NTRY from 3 to 100 for the folks
 *      at Automation decided a 21 byte WMO bulletin header wasn't long 
 *      enough and decided to go to an 8K header.  
 * v1.4 11/10/2001 D. Haalman, looks at entire file, does not try
 *      to read past EOF
 */

unsigned char *seek_grib(FILE *file, long *pos, long *len_grib, 
        unsigned char *buffer, unsigned int buf_len) 
{

    int i, j, len;

    j = 1;
    clearerr(file);
    while ( !feof(file) ) 
	{

        if (fseek(file, *pos, SEEK_SET) == -1) break;
        i = fread(buffer, sizeof (unsigned char), buf_len, file);     
        if (ferror(file)) break;
        len = i - LEN_HEADER_PDS;
     
        for (i = 0; i < len; i++) 
		{
            if (buffer[i] == 'G' && buffer[i+1] == 'R' && buffer[i+2] == 'I'
                && buffer[i+3] == 'B' && buffer[i+7] == 1) 
			{
                    *pos = i + *pos;
                    *len_grib = (buffer[i+4] << 16) + (buffer[i+5] << 8) +
                            buffer[i+6];
                    return (buffer+i);
            }
        }

	if (j++ == NTRY) 
	{
	    fprintf(stderr,"found unidentified data \n");
           /* break; // stop seeking after NTRY records */  
        }

	*pos = *pos + (buf_len - LEN_HEADER_PDS);
    }

    *len_grib = 0;
    return (unsigned char *) NULL;
}



/* ibm2flt       wesley ebisuzaki
 *
 * v1.1 .. faster
 * v1.1 .. if mant == 0 -> quick return
 *
 */

double ibm2flt(unsigned char *ibm) 
{

	int positive, power;
	unsigned int abspower;
	long int mant;
	double value, exp;

	mant = (ibm[1] << 16) + (ibm[2] << 8) + ibm[3];
        if (mant == 0)
		{
			return 0.0;
		}
        
	positive = (ibm[0] & 0x80) == 0;
	power = (int) (ibm[0] & 0x7f) - 64;
	abspower = power > 0 ? power : -power;


	// calc exp
	exp = 16.0;
	value = 1.0;
	while (abspower) 
	{
		if (abspower & 1) 
		{
			value *= exp;
		}
		exp = exp * exp;
		abspower >>= 1;
	}

	if (power < 0)
	{
		value = 1.0 / value;
	}

	value = value * mant / 16777216.0;
	
	if (positive == 0)
	{
		value = -value;
	}

	return value;
}


/*
 * read_grib.c
 *
 * reads grib message
 *
 * input: pos, byte position of grib message
 *        len_grib, length of grib message
 * output: *buffer, grib message
 *
 * note: call seek_grib first
 *
 * v1.0 9/94 Wesley Ebisuzaki
 *
 */

int read_grib(FILE *file, long pos, long len_grib, unsigned char *buffer) 
{

    int i;


    if (fseek(file, pos, SEEK_SET) == -1) 
	{
	    return 0;
    }

    i = fread(buffer, sizeof (unsigned char), len_grib, file);
    return (i == len_grib);
}

/*
 * w. ebisuzaki
 *
 *  return x**y
 *
 *
 *  input: double x
 *	   int y
 */

double int_power(double x, int y) 
{

	double value;

	if (y < 0) 
	{
		y = -y;
		x = 1.0 / x;
	}
	value = 1.0;

	while (y) 
	{
		if (y & 1) 
		{
			value *= x;
		}
		x = x * x;
		y >>= 1;
	}
	return value;
}

static struct ParmTable *Parm_Table(unsigned char *pds) 
{

    int i, center, subcenter, ptable, process;
    static int missing_count = 0, reanal_opn_count = 0;

    center = PDS_Center(pds);
    subcenter = PDS_Subcenter(pds);
    ptable = PDS_Vsn(pds);

#ifdef P_TABLE_FIRST
    i = setup_user_table(center, subcenter, ptable);
    if (i == 1) return &parm_table_user[0];
#endif
    /* figure out if NCEP opn or reanalysis */
    if (center == NMC && ptable <= 3) 
	{
		if (subcenter == 1)
		{
			return &parm_table_ncep_reanal[0];
		}

		process = PDS_Model(pds);
		if (subcenter != 0 || (process != 80 && process != 180) || 
			(ptable != 1 && ptable != 2))
		{
				return &parm_table_ncep_opn[0];
		}

		/* at this point could be either the opn or reanalysis table */
		if (def_ncep_table == opn_nowarn)
		{
			return &parm_table_ncep_opn[0];
		}

		if (def_ncep_table == rean_nowarn)
		{
			return &parm_table_ncep_reanal[0];
		}

		if (reanal_opn_count++ == 0) 
		{
			fprintf(stderr, "Using NCEP %s table, see -ncep_opn, -ncep_rean options\n",
				(def_ncep_table == opn) ?  "opn" : "reanalysis");
		}
			return (def_ncep_table == opn) ?  &parm_table_ncep_opn[0] 
			: &parm_table_ncep_reanal[0];
    }

    
	if (center == NMC) 
	{
        if (ptable == 128) return &parm_table_omb[0];
        if (ptable == 129) return &parm_table_nceptab_129[0];
        if (ptable == 130) return &parm_table_nceptab_130[0];
        if (ptable == 131) return &parm_table_nceptab_131[0];
        if (ptable == 132) return &parm_table_ncep_reanal[0];
    }
    if (center == ECMWF) 
	{
        if (ptable == 128) return &parm_table_ecmwf_128[0];
        if (ptable == 129) return &parm_table_ecmwf_129[0];
        if (ptable == 130) return &parm_table_ecmwf_130[0];
        if (ptable == 131) return &parm_table_ecmwf_131[0];
        if (ptable == 140) return &parm_table_ecmwf_140[0];
        if (ptable == 150) return &parm_table_ecmwf_150[0];
        if (ptable == 151) return &parm_table_ecmwf_151[0];
        if (ptable == 160) return &parm_table_ecmwf_160[0];
        if (ptable == 170) return &parm_table_ecmwf_170[0];
        if (ptable == 180) return &parm_table_ecmwf_180[0];
        if (ptable == 190) return &parm_table_ecmwf_190[0];
    }
    if (center == DWD) 
	{
        if (ptable ==   2) return &parm_table_dwd_002[0];
        if (ptable == 201) return &parm_table_dwd_201[0];
        if (ptable == 202) return &parm_table_dwd_202[0];
        if (ptable == 203) return &parm_table_dwd_203[0];
    }
    if (center == CPTEC) 
	{
		if (ptable == 254) return &parm_table_cptec_254[0];
    }

#ifndef P_TABLE_FIRST
    i = setup_user_table(center, subcenter, ptable);
    if (i == 1)
	{
		return &parm_table_user[0];
	}
#endif

    if ((ptable > 3 || (PDS_PARAM(pds)) > 127) && missing_count++ == 0) 
	{
	fprintf(stderr,
            "\nUndefined parameter table (center %d-%d table %d), using NCEP-opn\n",
            center, subcenter, ptable);
    }
    return &parm_table_ncep_opn[0];
}

/*
 * return name field of PDS_PARAM(pds)
 */

char *k5toa(unsigned char *pds) 
{

    return (Parm_Table(pds) + PDS_PARAM(pds))->name;
}

/*
 * return comment field of the PDS_PARAM(pds)
 */

char *k5_comments(unsigned char *pds) 
{

    return (Parm_Table(pds) + PDS_PARAM(pds))->comment;
}

/* 1996				wesley ebisuzaki
 *
 * Unpack BDS section
 *
 * input: *bits, pointer to packed integer data
 *        *bitmap, pointer to bitmap (undefined data), NULL if none
 *        n_bits, number of bits per packed integer
 *        n, number of data points (includes undefined data)
 *        ref, scale: flt[] = ref + scale*packed_int
 * output: *flt, pointer to output array
 *        undefined values filled with UNDEFINED
 *
 * note: code assumes an integer > 32 bits
 *
 * 7/98 v1.2.1 fix bug for bitmaps and nbit >= 25 found by Larry Brasfield
 * 2/01 v1.2.2 changed jj from long int to double
 * 3/02 v1.2.3 added unpacking extensions for spectral data 
 *             Luis Kornblueh, MPIfM 
 */

void BDS_unpack(std::vector<double>& vecData, unsigned char *bds, unsigned char *bitmap,
	int n_bits, int n, double ref, double scale) 
{

    unsigned char *bits;

    int i, mask_idx, t_bits, c_bits, j_bits;
    unsigned int j, map_mask, tbits, jmask, bbits;
    double jj;

	static unsigned int mask[] = {0,1,3,7,15,31,63,127,255};
	static unsigned int map_masks[8] = {128, 64, 32, 16, 8, 4, 2, 1};
	static double shift[9] = {1.0, 2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0};

    if (BDS_Harmonic(bds)) 
	{
        bits = bds + 15;
        /* fill in global mean */
        vecData.push_back(BDS_Harmonic_RefValue(bds));
        n -= 1; 
    }
    else 
	{
        bits = bds + 11;  
    }

    tbits = bbits = 0;

    /* assume integer has 32+ bits */
    if (n_bits <= 25)
	{
        jmask = (1 << n_bits) - 1;
        t_bits = 0;

        if (bitmap) 
		{
			for (i = 0; i < n; i++) 
			{
				/* check bitmap */
				mask_idx = i & 7;
				if (mask_idx == 0) bbits = *bitmap++;
				{
					if ((bbits & map_masks[mask_idx]) == 0) 
					{
						vecData.push_back(getUndefinedValue());
						continue;
					}

					while (t_bits < n_bits) 
					{
						tbits = (tbits * 256) + *bits++;
						t_bits += 8;
					}
					t_bits -= n_bits;
					j = (tbits >> t_bits) & jmask;
					vecData.push_back(ref + scale*j);
				}
			}
        }
        else 
		{
			for (i = 0; i < n; i++) 
			{
					while (t_bits < n_bits) 
					{
						tbits = (tbits * 256) + *bits++;
						t_bits += 8;
					}
					t_bits -= n_bits;
					vecData.push_back((tbits >> t_bits) & jmask);
			}
			/* at least this vectorizes :) */
			for (i = 0; i < n; i++) 
			{
				vecData[i] = ref + scale*vecData[i];
			}
        }
    }
    else 
	{
	/* older unoptimized code, not often used */
        c_bits = 8;
        map_mask = 128;
        while (n-- > 0) 
		{
	    if (bitmap) 
		{
	        j = (*bitmap & map_mask);
	        if ((map_mask >>= 1) == 0) 
			{
		    map_mask = 128;
		    bitmap++;
	        }
	        if (j == 0) 
			{
		    vecData.push_back(getUndefinedValue());
		    continue;
	        }
	    }

	    jj = 0.0;
	    j_bits = n_bits;
	    while (c_bits <= j_bits) 
		{
	        if (c_bits == 8) 
			{
		    jj = jj * 256.0  + (double) (*bits++);
		    j_bits -= 8;
	        }
	        else 
			{
		    jj = (jj * shift[c_bits]) + (double) (*bits & mask[c_bits]);
		    bits++;
		    j_bits -= c_bits;
		    c_bits = 8;
	        }
	    }
	    if (j_bits) 
		{
	        c_bits -= j_bits;
	        jj = (jj * shift[j_bits]) + (double) ((*bits >> c_bits) & mask[j_bits]);
	    }
	    vecData.push_back(ref + scale*jj);
        }
    }
    return;
}

/*
 * convert a float to an ieee single precision number v1.1
 * (big endian)
 *                      Wesley Ebisuzaki
 *
 * bugs: doesn't handle subnormal numbers
 * bugs: assumes length of integer >= 25 bits
 */

int flt2ieee(float x, unsigned char *ieee) 
{

	int sign, exp;
        unsigned int umant;
	double mant;

	if (x == 0.0) 
	{
		ieee[0] = ieee[1] = ieee[2] = ieee[3] = 0;
		return 0;
	}

	/* sign bit */
	if (x < 0.0) 
	{
		sign = 128;
		x = -x;
	}
	else sign = 0;
	mant = frexp((double) x, &exp);

        /* 2^24 = 16777216 */

	umant = (unsigned int)(mant * 16777216.0 + 0.5);
	if (umant >= 16777216) 
	{
            umant = umant / 2;
            exp++;
        }
        /* bit 24 should be a 1 .. not used in ieee format */

	exp = exp - 1 + 127;

	if (exp < 0) 
	{
		/* signed zero */
		ieee[0] = sign;
		ieee[1] = ieee[2] = ieee[3] = 0;
		return 0;
	}
	if (exp > 255) 
	{
		/* signed infinity */
		ieee[0] = sign + 127;
		ieee[1] = 128;
                ieee[2] = ieee[3] = 0;
                return 0;
	}
	/* normal number */

	ieee[0] = sign + (exp >> 1);

        ieee[3] = umant & 255;
        ieee[2] = (umant >> 8) & 255;
        ieee[1] = ((exp & 1) << 7) + ((umant >> 16) & 127);
	return 0;
}


/* wesley ebisuzaki v1.3
 *
 * write ieee file -- big endian format
 *
 * input float *array		data to be written
 *	 int n			size of array
 *	 int header		1 for f77 style header 0 for none
 *				(header is 4 byte header
 *	 FILE *output		output file
 *
 * v1.2 7/97 buffered, faster
 * v1.3 2/99 fixed (typo) error in wrtieee_header found by
 *     Bob Farquhar
 */

int wrtieee(float *array, int n, int header, FILE *output) 
{

	unsigned long int l;
	int i, nbuf;
	unsigned char buff[BSIZ];
	unsigned char h4[4];

	nbuf = 0;
	if (header) 
	{
		l = n * 4;
		for (i = 0; i < 4; i++) 
		{
			h4[i] = (unsigned char)(l & 255);
			l >>= 8;
		}
		buff[nbuf++] = h4[3];
		buff[nbuf++] = h4[2];
		buff[nbuf++] = h4[1];
		buff[nbuf++] = h4[0];
	}
	for (i = 0; i < n; i++) 
	{
		if (nbuf >= BSIZ) 
		{
		    fwrite(buff, 1, BSIZ, output);
		    nbuf = 0;
		}
		flt2ieee(array[i], buff + nbuf);
		nbuf += 4;
	}
	if (header) 
	{
		if (nbuf == BSIZ) 
		{
		    fwrite(buff, 1, BSIZ, output);
		    nbuf = 0;
		}
		buff[nbuf++] = h4[3];
		buff[nbuf++] = h4[2];
		buff[nbuf++] = h4[1];
		buff[nbuf++] = h4[0];
	}
	if (nbuf) fwrite(buff, 1, nbuf, output);
	return 0;
}

/* write a big-endian 4 byte integer .. f77 IEEE  header */

int wrtieee_header(unsigned int n, FILE *output) 
{
	unsigned h4[4];

	h4[0] = n & 255;
	h4[1] = (n >> 8) & 255;
	h4[2] = (n >> 16) & 255;
	h4[3] = (n >> 24) & 255;

	putc(h4[3],output);
	putc(h4[2],output);
	putc(h4[1],output);
	putc(h4[0],output);

	return 0;
}


/* wesley ebisuzaki v1.0
 *
 * levels.c
 *
 * prints out a simple description of kpds6, kpds7
 *    (level/layer data)
 *  kpds6 = octet 10 of the PDS
 *  kpds7 = octet 11 and 12 of the PDS
 *    (kpds values are from NMC's grib routines)
 *  center = PDS_Center(pds) .. NMC, ECMWF, etc
 *
 * the description of the levels is 
 *   (1) incomplete
 *   (2) include some NMC-only values (>= 200?)
 *
 * v1.1 wgrib v1.7.3.1 updated with new levels
 * v1.2 added new level and new parameter
 * v1.2.1 modified level 117 pv units
 * v1.2.2 corrected level 141
 * v1.2.3 fixed layer 206 (was 205)
 */

void levels(char* szDesc, int kpds6, int kpds7, int center)
{

	int o11, o12;

	/* octets 11 and 12 */
	o11 = kpds7 / 256;
	o12 = kpds7 % 256;


	switch (kpds6) 
	{

	case 1: sprintf(szDesc, "sfc");
		break;
	case 2: sprintf(szDesc, "cld base");
		break;
	case 3: sprintf(szDesc, "cld top");
		break;
	case 4: sprintf(szDesc, "0C isotherm");
		break;
	case 5: sprintf(szDesc, "cond lev");
		break;
	case 6: sprintf(szDesc, "max wind lev");
		break;
	case 7: sprintf(szDesc, "tropopause");
		break;
	case 8: sprintf(szDesc, "nom. top");
		break;
	case 9: sprintf(szDesc, "sea bottom");
		break;
	case 200:
	case 10: sprintf(szDesc, "atmos col");
		break;

	case 12:
	case 212: sprintf(szDesc, "low cld bot");
		break;
	case 13:
	case 213: sprintf(szDesc, "low cld top");
		break;
	case 14:
	case 214: sprintf(szDesc, "low cld lay");
		break;
	case 22:
	case 222: sprintf(szDesc, "mid cld bot");
		break;
	case 23:
	case 223: sprintf(szDesc, "mid cld top");
		break;
	case 24:
	case 224: sprintf(szDesc, "mid cld lay");
		break;
	case 32:
	case 232: sprintf(szDesc, "high cld bot");
		break;
	case 33:
	case 233: sprintf(szDesc, "high cld top");
		break;
	case 34:
	case 234: sprintf(szDesc, "high cld lay");
		break;

	case 201: sprintf(szDesc, "ocean column");
		break;
	case 204: sprintf(szDesc, "high trop freezing lvl");
		break;
	case 206: sprintf(szDesc, "grid-scale cld bot");
		break;
	case 207: sprintf(szDesc, "grid-scale cld top");
		break;
	case 209: sprintf(szDesc, "bndary-layer cld bot");
		break;
	case 210: sprintf(szDesc, "bndary-layer cld top");
		break;
	case 211: sprintf(szDesc, "bndary-layer cld layer");
		break;
	case 235: if (kpds7 % 10 == 0)
		sprintf(szDesc, "%dC ocean isotherm level",kpds7/10);
		else sprintf(szDesc, "%.1fC ocean isotherm level",kpds7/10.0);
		break;
	case 236: sprintf(szDesc, "%d-%dm ocean layer",o11*10,o12*10);
		break;
	case 237: sprintf(szDesc, "ocean mixed layer bot");
		break;
	case 238: sprintf(szDesc, "ocean isothermal layer bot");
		break;
	case 242: sprintf(szDesc, "convect-cld bot");
		break;
	case 243: sprintf(szDesc, "convect-cld top");
		break;
	case 244: sprintf(szDesc, "convect-cld layer");
		break;
	case 246: sprintf(szDesc, "max e-pot-temp lvl");
		break;
	case 247: sprintf(szDesc, "equilibrium lvl");
		break;
	case 248: sprintf(szDesc, "shallow convect-cld bot");
		break;
	case 249: sprintf(szDesc, "shallow convect-cld top");
		break;
	case 251: sprintf(szDesc, "deep convect-cld bot");
		break;
	case 252: sprintf(szDesc, "deep convect-cld top");
		break;

	case 100: sprintf(szDesc, "%d mb",kpds7);
	 	break;
	case 101: sprintf(szDesc, "%d-%d mb",o11*10,o12*10);
	 	break;
	case 102: sprintf(szDesc, "MSL");
	 	break;
	case 103: sprintf(szDesc, "%d m above MSL",kpds7);
	 	break;
	case 104: sprintf(szDesc, "%d-%d m above msl",o11*100,o12*100);
	 	break;
	case 105: sprintf(szDesc, "%d m above gnd",kpds7);
	 	break;
	case 106: sprintf(szDesc, "%d-%d m above gnd",o11*100,o12*100);
	 	break;
	case 107: sprintf(szDesc, "sigma=%.4f",kpds7/10000.0);
	 	break;
	case 108: sprintf(szDesc, "sigma %.2f-%.2f",o11/100.0,o12/100.0);
	 	break;
	case 109: sprintf(szDesc, "hybrid lev %d",kpds7);
	 	break;
	case 110: sprintf(szDesc, "hybrid %d-%d",o11,o12);
	 	break;
	case 111: sprintf(szDesc, "%d cm down",kpds7);
	 	break;
	case 112: sprintf(szDesc, "%d-%d cm down",o11,o12);
	 	break;
	case 113: sprintf(szDesc, "%dK",kpds7);
	 	break;
	case 114: sprintf(szDesc, "%d-%dK",475-o11,475-o12);
	 	break;
	case 115: sprintf(szDesc, "%d mb above gnd",kpds7);
	 	break;
	case 116: sprintf(szDesc, "%d-%d mb above gnd",o11,o12);
	 	break;
	case 117: sprintf(szDesc, "%d pv units",INT2(o11,o12)); /* units are suspect */
	 	break;
	case 119: sprintf(szDesc, "%.5f (ETA level)",kpds7/10000.0);
	 	break;
	case 120: sprintf(szDesc, "%.2f-%.2f (ETA levels)",o11/100.0,o12/100.0);
	 	break;
	case 121: sprintf(szDesc, "%d-%d mb",1100-o11,1100-o12);
	 	break;
	case 125: sprintf(szDesc, "%d cm above gnd",kpds7);
	 	break;
	case 126: 
		if (center == NMC) sprintf(szDesc, "%.2f mb",kpds7*0.01);
	 	break;
	case 128: sprintf(szDesc, "%.3f-%.3f (sigma)",1.1-o11/1000.0, 1.1-o12/1000.0);
	 	break;
	case 141: sprintf(szDesc, "%d-%d mb",o11*10,1100-o12);
	 	break;
	case 160: sprintf(szDesc, "%d m below sea level",kpds7);
	 	break;
	default:
	 	break;
	}
}

/*
 * PDStimes.c   v1.2 wesley ebisuzaki
 *
 * prints something readable for time code in grib file
 *
 * not all cases decoded
 * for NCEP/NCAR Reanalysis
 *
 * v1.2.1 1/99 fixed forecast time unit table
 * v1.2.2 10/01 add time_range = 11 (at DWD)  Helmut P. Frank
 */

void PDStimes(char* szTimeDesc, int time_range, int p1, int p2, int time_unit) 
{

	char *unit;
	enum {anal, fcst, unknown} type;
	int fcst_len = 0;

	if (time_unit >= 0 && time_unit <= sizeof(units)/sizeof(char *))
             unit = units[time_unit];
	else unit = "";

	szTimeDesc[0] = 0;

        /* change x3/x6/x12 to hours */

        if (time_unit == HOURS3) 
		{
	    p1 *= 3; p2 *= 3;
	    time_unit = HOUR;
        }
        else if (time_unit == HOURS6) 
		{
	    p1 *= 6; p2 *= 6;
	    time_unit = HOUR;
        }
        else if (time_unit == HOURS12) 
		{
	    p1 *= 12; p2 *= 12;
	    time_unit = HOUR;
        }

	if (time_unit >= 0 && time_unit <= sizeof(units)/sizeof(char *))
             unit = units[time_unit];
	else unit = "";

	/* figure out if analysis or forecast */
	/* in GRIB, there is a difference between init and uninit analyses */
	/* not case at NMC .. no longer run initialization */
	/* ignore diff between init an uninit analyses */

	switch (time_range) 
	{

	case 0:
	case 1:
	case 113:
	case 114:
	case 118:
		if (p1 == 0) type = anal;
		else 
		{
			type = fcst;
			fcst_len = p1;
		}
		break;
	case 10: /* way NMC uses it, should be unknown? */
		type = fcst;
		fcst_len = p1*256 + p2;
		if (fcst_len == 0) type = anal;
		break;

	case 51:
		type = unknown;
		break;
	case 123:
	case 124:
		type = anal;
		break;

	case 135:
		type = anal;
		break;

	default: type = unknown;
		break;
	}

	/* ----------------------------------------------- */

	char szTemp[100];

	//if (type == anal)
	//{
	//	sprintf(szTemp, "anl:");
	//	strcat(szTimeDesc,szTemp);
	//}
	//else
	if (type == fcst)
	{
		sprintf(szTemp, "%d%s fcst:",fcst_len,unit);
		strcat(szTimeDesc, szTemp);
	}


	if (time_range == 123 || time_range == 124) 
	{
		if (p1 != 0)
		{
			sprintf(szTemp, "start@%d%s:",p1,unit);
			strcat(szTimeDesc, szTemp);
		}
	}


	/* print time range */

	szTemp[0] = 0;
	switch (time_range) 
	{

	case 0:
	case 1:
	case 10:
		break;
	case 2: sprintf(szTemp, "valid %d-%d%s:",p1,p2,unit);
		break;
	case 3: sprintf(szTemp, "%d-%d%s ave:",p1,p2,unit);
		break;
	case 4: sprintf(szTemp, "%d-%d%s acc:",p1,p2,unit);
		break;
	case 5: sprintf(szTemp, "%d-%d%s diff:",p1,p2,unit);
		break;
        case 6: sprintf(szTemp, "-%d to -%d %s ave:", p1,p2,unit);
                break;
        case 7: sprintf(szTemp, "-%d to %d %s ave:", p1,p2,unit);
                break;
	case 11: if (p1 > 0) 
			 {
		    sprintf(szTemp, "init fcst %d%s:",p1,unit);
		}
		else
		{
	            sprintf(szTemp, "time?:");
		}
		break;
	case 51: if (p1 == 0) 
			 {
		    /* printf("clim %d%s:",p2,unit); */
		    sprintf(szTemp, "0-%d%s product:ave@1yr:",p2,unit);
		}
		else if (p1 == 1) 
		{
		    /* printf("clim (diurnal) %d%s:",p2,unit); */
		    sprintf(szTemp, "0-%d%s product:same-hour,ave@1yr:",p2,unit);
		}
		else 
		{
		    sprintf(szTemp, "clim? p1=%d? %d%s?:",p1,p2,unit);
		}
		break;
	case 113:
	case 123:
		sprintf(szTemp, "ave@%d%s:",p2,unit);
		break;
	case 114:
	case 124:
		sprintf(szTemp, "acc@%d%s:",p2,unit);
		break;
	case 115:
		sprintf(szTemp, "ave of fcst:%d to %d%s:",p1,p2,unit);
		break;
	case 116:
		sprintf(szTemp, "acc of fcst:%d to %d%s:",p1,p2,unit);
		break;
	case 118: 
		sprintf(szTemp, "var@%d%s:",p2,unit);
		break;
	case 128:
		sprintf(szTemp, "%d-%d%s fcst acc:ave@24hr:", p1, p2, unit);
		break;
	case 129:
		sprintf(szTemp, "%d-%d%s fcst acc:ave@%d%s:", p1, p2, unit, p2-p1,unit);
		break;
	case 130:
		sprintf(szTemp, "%d-%d%s fcst ave:ave@24hr:", p1, p2, unit);
		break;
	case 131:
		sprintf(szTemp, "%d-%d%s fcst ave:ave@%d%s:", p1, p2, unit,p2-p1,unit);
		break;
		/* for CFS */
	case 132:
		sprintf(szTemp, "%d-%d%s anl:ave@1yr:", p1, p2, unit);
		break;
	case 133:
		sprintf(szTemp, "%d-%d%s fcst:ave@1yr:", p1, p2, unit);
		break;
	case 134:
		sprintf(szTemp, "%d-%d%s fcst-anl:rms@1yr:", p1, p2, unit);
		break;
	case 135:
		sprintf(szTemp, "%d-%d%s fcst-fcst_mean:rms@1yr:", p1, p2, unit);
		break;
	case 136:
		sprintf(szTemp, "%d-%d%s anl-anl_mean:rms@1yr:", p1, p2, unit);
		break;

	}

	strcat(szTimeDesc, szTemp);
}


int missing_points(unsigned char *bitmap, int n) 
{

    int count;
    unsigned int tmp;
    if (bitmap == NULL) return 0;

    count = 0;
    while (n >= 8) 
	{
	tmp = *bitmap++;
	n -= 8;
        count += bitsum[tmp];
    }
    tmp = *bitmap | ((1 << (8 - n)) - 1);
    count += bitsum[tmp];

    return count;
}


/*
 * EC_ext	v1.0 wesley ebisuzaki
 *
 * prints something readable from the EC stream parameter
 *
 * prefix and suffix are only printed if EC_ext has text
 */

void EC_ext(unsigned char *pds, char *prefix, char *suffix, int verbose) 
{

    int local_id, ec_type, ec_class, ec_stream;
    char string[200];

    if (PDS_Center(pds) != ECMWF) return;

    local_id = PDS_EcLocalId(pds);
    if (local_id  == 0) return;
    ec_class = PDS_EcClass(pds);
    ec_type = PDS_EcType(pds);
    ec_stream = PDS_EcStream(pds);

    if (verbose == 2) printf("%sECext=%d%s", prefix, local_id, suffix);

    if (verbose == 2) 
	{
	switch(ec_class) 
	{
	    case 1: strcpy(string, "operations"); break;
	    case 2: strcpy(string, "research"); break;
	    case 3: strcpy(string, "ERA-15"); break;
	    case 4: strcpy(string, "Euro clim support network"); break;
	    case 5: strcpy(string, "ERA-40"); break;
	    case 6: strcpy(string, "DEMETER"); break;
	    case 7: strcpy(string, "PROVOST"); break;
	    case 8: strcpy(string, "ELDAS"); break;
	     default: sprintf(string, "%d", ec_class); break;
	}
        printf("%sclass=%s%s",prefix,string,suffix);
    }
    /*
     10/03/2000: R.Rudsar : subroutine changed.
                 Tests for EcType and extra test for EcStream 1035
    */


    if (verbose == 2) 
	{
        switch(ec_type) 
		{
            case 1: strcpy(string, "first guess"); break;
            case 2: strcpy(string, "analysis"); break;
            case 3: strcpy(string, "init analysis"); break;
            case 4: strcpy(string, "OI analysis"); break;
            case 10: strcpy(string, "Control forecast"); break;
            case 11: strcpy(string, "Perturbed forecasts"); break;
            case 14: strcpy(string, "Cluster means"); break;
            case 15: strcpy(string, "Cluster std. dev."); break;
            case 16: strcpy(string, "Forecast probability"); break;
            case 17: strcpy(string, "Ensemble means"); break;
            case 18: strcpy(string, "Ensemble std. dev."); break;
    	    case 20: strcpy(string, "Climatology"); break;
            case 21: strcpy(string, "Climatology simulation"); break;
            case 80: strcpy(string, "Fcst seasonal mean"); break;
            default: sprintf(string, "%d", ec_type); break;
        }
        printf("%stype=%s%s",prefix,string,suffix);
    }

    if (verbose == 2) 
	{
        switch(ec_stream) 
		{
	    case 1035: strcpy(string, "ensemble forecasts"); break;
	    case 1043: strcpy(string, "mon mean"); break;
	    case 1070: strcpy(string, "mon (co)var"); break;
	    case 1071: strcpy(string, "mon mean from daily"); break;
	    case 1090: strcpy(string, "EC ensemble fcsts"); break;
	    case 1091: strcpy(string, "EC seasonal fcst mon means"); break;
	    default:   sprintf(string, "%d", ec_stream); break;
        }
        printf("%sstream=%s%s",prefix,string,suffix);
    }
    if (verbose == 2) 
	{
        printf("%sVersion=%c%c%c%c%s", prefix, *(PDS_Ec16Version(pds)), *(PDS_Ec16Version(pds)+1),
		*(PDS_Ec16Version(pds)+2), *(PDS_Ec16Version(pds)+3), suffix);
        if (local_id == 16) 
		{
	    printf("%sSysVersion=%d%s", prefix, PDS_Ec16SysNum(pds), suffix);
	    printf("%sAvgPeriod=%d%s", prefix, PDS_Ec16AvePeriod(pds), suffix);
	    printf("%sFcstMon=%d%s", prefix, PDS_Ec16FcstMon(pds), suffix);

        }
    }

        if (local_id == 16) 
		{
	    printf("%sEnsem_mem=%d%s", prefix, PDS_Ec16Number(pds), suffix);
	    printf("%sVerfDate=%d%s", prefix, PDS_Ec16VerfMon(pds), suffix);
        }

}

/*
 * get grid size from GDS
 *
 * added calculation of nxny of spectral data and clean up of triangular
 * grid nnxny calculation     l. kornblueh 
 * 7/25/03 wind fix Dusan Jovic
 * 9/17/03 fix scan mode
 */

int GDS_grid(unsigned char *gds, unsigned char *bds, int *nx, int *ny, 
             long int *nxny) 
{

    int i, d, ix, iy, pl;
    long int isum;

    *nx = ix = GDS_LatLon_nx(gds);
    *ny = iy = GDS_LatLon_ny(gds);
    *nxny = ix * iy;

    /* thin grid */

    if (GDS_Gaussian(gds) || GDS_LatLon(gds)) 
	{
	if (ix == 65535) 
	{
	    *nx = -1;
	    /* reduced grid */
	    isum = 0;
	    pl = GDS_PL(gds);
	    for (i = 0; i < iy; i++) 
		{
		isum += gds[pl+i*2]*256 + gds[pl+i*2+1];
	    }
	    *nxny = isum;
	}
	return 0;
    }
    if (GDS_Triangular(gds))
	{
        i = GDS_Triangular_ni(gds);
        d = GDS_Triangular_nd(gds);
	*nx = *nxny = d * (i + 1) * (i + 1);
        *ny = 1;
	return 0;
    }
    if (GDS_Harmonic(gds)) 
	{
        /* this code assumes j, k, m are consistent with bds */
        *nx = *nxny = (8*(BDS_LEN(bds)-15)-BDS_UnusedBits(bds))/
		BDS_NumBits(bds)+1;
           if ((8*(BDS_LEN(bds)-15)-BDS_UnusedBits(bds)) % BDS_NumBits(bds)) 
		   {
	       fprintf(stderr,"inconsistent harmonic BDS\n");
           }
        *ny = 1;
    }
    return 0;
}

#define NCOL 15
void GDS_prt_thin_lon(unsigned char *gds) 
{
    int iy, i, col, pl;

    iy = GDS_LatLon_ny(gds);
    iy = (iy + 1) / 2;
    iy = GDS_LatLon_ny(gds);

    if ((pl = GDS_PL(gds)) == -1) 
	{
	fprintf(stderr,"\nprogram error: GDS_prt_thin\n");
	return;
    }
    for (col = i = 0; i < iy; i++) 
	{
	if (col == 0) printf("   ");
	printf("%5d", (gds[pl+i*2] << 8) + gds[pl+i*2+1]);
	col++;
	if (col == NCOL) 
	{
	    col = 0;
	    printf("\n");
	}
    }
    if (col != 0) printf("\n");
}

void GDS_winds(unsigned char *gds, int verbose) 
{
    int scan = -1, mode = -1;

    if (gds != NULL) 
	{
        if (GDS_LatLon(gds)) 
		{
	    scan = GDS_LatLon_scan(gds);
	    mode = GDS_LatLon_mode(gds);
	}
	else if (GDS_Mercator(gds)) 
	{
	    scan =GDS_Merc_scan(gds);
	    mode =GDS_Merc_mode(gds);
	}
	/* else if (GDS_Gnomonic(gds)) { */
	else if (GDS_Lambert(gds)) 
	{
	    scan = GDS_Lambert_scan(gds);
	    mode = GDS_Lambert_mode(gds);
	}
	else if (GDS_Gaussian(gds)) 
	{
	    scan = GDS_LatLon_scan(gds);
	    mode = GDS_LatLon_mode(gds);
	}
	else if (GDS_Polar(gds)) 
	{
	    scan = GDS_Polar_scan(gds);
	    mode = GDS_Polar_mode(gds);
	}
	else if (GDS_RotLL(gds)) 
	{
	    scan = GDS_RotLL_scan(gds);
	    mode = GDS_RotLL_mode(gds);
	}
	/* else if (GDS_Triangular(gds)) { */
	else if (GDS_ssEgrid(gds)) 
	{
	    scan = GDS_ssEgrid_scan(gds);
	    mode = GDS_ssEgrid_mode(gds);
	}
	else if (GDS_fEgrid(gds)) 
	{
	    scan = GDS_fEgrid_scan(gds);
	    mode = GDS_fEgrid_mode(gds);
	}
	else if (GDS_ss2dEgrid(gds)) 
	{
	    scan = GDS_ss2dEgrid_scan(gds);
	    mode = GDS_ss2dEgrid_mode(gds);
	}
    }
    if (verbose == 1) 
	{
	if (mode != -1) 
	{
	    if (mode & 8) printf("winds in grid direction:");
	    else printf("winds are N/S:"); 
	}
    }
    else if (verbose == 2) 
	{
	if (scan != -1) 
	{
	    printf(" scan: %s", scan_mode[(scan >> 5) & 7]);
        }
	if (mode != -1) 
	{
	    if (mode & 8) printf(" winds(grid) ");
	    else printf(" winds(N/S) "); 
	}
    }
}


/*
 * sets up user parameter table
 * v1.1 12/2005 w. ebisuzaki
 */

int setup_user_table(int center, int subcenter, int ptable) 
{

    int i, j, c0, c1, c2;
    static FILE *input;
    static int file_open = 0;
    char *filename, line[300];

    if (status == init) 
	{
	for (i = 0; i < 256; i++) 
	{
	    parm_table_user[i].name = parm_table_user[i].comment = NULL;
	}
	status = not_checked;
    }

    if (status == no_file) return 0;

    if ((user_center == -1 || center == user_center) &&
	    (user_subcenter == -1 || subcenter == user_subcenter) &&
	    (user_ptable == -1 || ptable == user_ptable)) 
	{

	if (status == filled) return 1;
	if (status == not_found) return 0;
    }

    /* open gribtab file if not open */

    if (!file_open) 
	{
        filename = getenv("GRIBTAB");
        if (filename == NULL) filename = getenv("gribtab");
        if (filename == NULL) filename = "gribtab";

        if ((input = fopen(filename,"r")) == NULL) 
		{
            status = no_file;
            return 0;
        }
	file_open = 1;
    }
    else 
	{
	rewind(input);
    }

    user_center = center;
    user_subcenter = subcenter;
    user_ptable = ptable;

    /* scan for center & subcenter and ptable */
    for (;;) 
	{
        if (fgets(line, 299, input) == NULL) 
		{
	    status = not_found;
            return 0;
        }
	if (atoi(line) != START) continue;
	i = sscanf(line,"%d:%d:%d:%d", &j, &center, &subcenter, &ptable);
        if (i != 4) 
		{
	    fprintf(stderr,"illegal gribtab center/subcenter/ptable line: %s\n", line);
            continue;
        }
	if ((center == -1 || center == user_center) &&
	    (subcenter == -1 || subcenter == user_subcenter) &&
	    (ptable == -1 || ptable == user_ptable)) break;
    }

    user_center = center;
    user_subcenter = subcenter;
    user_ptable = ptable;

    /* free any used memory */
    for (i = 0; i < 256; i++) 
	{
        if (parm_table_user[i].name != NULL) free(parm_table_user[i].name);
        if (parm_table_user[i].comment != NULL) free(parm_table_user[i].comment);
	parm_table_user[i].name = parm_table_user[i].comment = NULL;
    }

    /* read definitions */

    for (;;) 
	{
        if (fgets(line, 299, input) == NULL) break;
	if ((i = atoi(line)) == START) break;
	line[299] = 0;

	/* find the colons and end-of-line */
	for (c0 = 0; line[c0] != ':' && line[c0] != 0; c0++) ;
        /* skip blank lines */
        if (line[c0] == 0) continue;

	for (c1 = c0 + 1; line[c1] != ':' && line[c1] != 0; c1++) ;
	c2 = strlen(line);
        if (line[c2-1] == '\n') line[--c2] = '\0';
        if (c2 <= c1) 
		{
	    fprintf(stderr,"illegal gribtab line:%s\n", line);
	    continue;
	}
	line[c0] = 0;
	line[c1] = 0;

	parm_table_user[i].name = (char *) malloc(c1 - c0);
	parm_table_user[i].comment = (char *) malloc(c2 - c1);
	strcpy(parm_table_user[i].name, line+c0+1);
	strcpy(parm_table_user[i].comment, line+c1+1);
    }

    /* now to fill in undefined blanks */
    for (i = 0; i < 255; i++) 
	{
	if (parm_table_user[i].name == NULL) 
	{
	    parm_table_user[i].name = (char *) malloc(7);
	    sprintf(parm_table_user[i].name, "var%d", i);
	    parm_table_user[i].comment = (char *) malloc(strlen("undefined")+1);
	    strcpy(parm_table_user[i].comment, "undefined");
        }
    }
    status = filled;
    return 1;
}

/*
 * PDS_date.c  v1.2 wesley ebisuzaki
 *
 * prints a string with a date code
 *
 * PDS_date(pds,option, v_time)
 *   options=0  .. 2 digit year
 *   options=1  .. 4 digit year
 *
 *   v_time=0   .. initial time
 *   v_time=1   .. verification time
 *
 * assumption: P1 and P2 are unsigned integers (not clear from doc)
 *
 * v1.2 years that are multiple of 400 are leap years, not 500
 * v1.2.1  make the change to the source code for v1.2
 * v1.2.2  add 3/6/12 hour forecast time units
 */

static int msg_count = 0;

int PDS_date(unsigned char *pds, int option, int v_time) 
{

    int year, month, day, hour, min;

    if (v_time == 0) 
	{
        year = PDS_Year4(pds);
        month = PDS_Month(pds);
        day  = PDS_Day(pds);
        hour = PDS_Hour(pds);
    }
    else 
	{
        if (verf_time(pds, &year, &month, &day, &hour) != 0) {
	    if (msg_count++ < 5) fprintf(stderr, "PDS_date: problem\n");
	}
    }
    min =  PDS_Minute(pds);

    switch(option) 
	{
	case 0:
	    printf("%2.2d%2.2d%2.2d%2.2d", year % 100, month, day, hour);
	    break;
	case 1:
	    printf("%4.4d%2.2d%2.2d%2.2d", year, month, day, hour);
	    break;
	default:
	    fprintf(stderr,"missing code\n");
	    exit(8);
    }
    return 0;
}

int add_time(int *year, int *month, int *day, int *hour, int dtime, int unit) 
{
    int y, m, d, h, jday, i;

    y = *year;
    m = *month;
    d = *day;
    h = *hour;

    if (unit == YEAR) 
	{
	*year = y + dtime;
	return 0;
    }
    if (unit == DECADE) 
	{
	*year =  y + (10 * dtime);
	return 0;
    }
    if (unit == CENTURY) 
	{
	*year = y + (100 * dtime);
	return 0;
    }
    if (unit == NORMAL) 
	{
	*year = y + (30 * dtime);
	return 0;
    }
    if (unit == MONTH) 
	{
        if (dtime < 0) 
		{
           i = (-dtime) / 12 + 1;
           y -= i;
           dtime += (i * 12);
        }
	dtime += (m - 1);
	*year = y + (dtime / 12);
	*month = 1 + (dtime % 12);
	return 0;
    }

    if (unit == SECOND) 
	{
	dtime /= 60;
	unit = MINUTE;
    }
    if (unit == MINUTE) 
	{
	dtime /= 60;
	unit = HOUR;
    }

    if (unit == HOURS3) 
	{
        dtime *= 3;
        unit = HOUR;
    }
    else if (unit == HOURS6) 
	{
        dtime *= 6;
        unit = HOUR;
    }
    else if (unit == HOURS12) 
	{
        dtime *= 12;
        unit = HOUR;
    }

    if (unit == HOUR) 
	{
	dtime += h;

        *hour = dtime % 24;
        dtime = dtime / 24;
        if (*hour < 0) 
		{
            *hour += 24;
            dtime--;
        }
        unit = DAY;
    }

    /* this is the hard part */

    if (unit == DAY) 
	{
	/* set m and day to Jan 0, and readjust dtime */
	jday = d + monthjday[m-1];
	if (leap(y) && m > 2) jday++;
        dtime += jday;

        while (dtime < 1) 
		{
            y--;
	    dtime += 365 + leap(y);
        }

	/* one year chunks */
	while (dtime > 365 + leap(y)) 
	{
	    dtime -= (365 + leap(y));
	    y++;
	}

	/* calculate the month and day */

	if (leap(y) && dtime == FEB29) 
	{
	    m = 2;
	    d = 29;
	}
	else 
	{
	    if (leap(y) && dtime > FEB29) dtime--;
	    for (i = 11; monthjday[i] >= dtime; --i);
	    m = i + 1;
	    d = dtime - monthjday[i];
	}
	*year = y;
	*month = m;
	*day = d;
	return 0;
   }
   fprintf(stderr,"add_time: undefined time unit %d\n", unit);
   return 1;
}


/*
 * verf_time:
 *
 * this routine returns the "verification" time
 * should have behavior similar to gribmap
 *
 */

int verf_time(unsigned char *pds, int *year, int *month, int *day, int *hour)
{
    int tr, dtime, unit;

    *year = PDS_Year4(pds);
    *month = PDS_Month(pds);
    *day  = PDS_Day(pds);
    *hour = PDS_Hour(pds);

    // find time increment 

    dtime = PDS_P1(pds);
    tr = PDS_TimeRange(pds);
    unit = PDS_ForecastTimeUnit(pds);

    if (tr == 10)
	{
		dtime = PDS_P1(pds) * 256 + PDS_P2(pds);
	}

    if (tr > 1 && tr < 6 )
	{
		dtime = PDS_P2(pds);
	}

    if (tr == 6 || tr == 7)
	{
		dtime = - PDS_P1(pds);
	}

    if (dtime == 0)
	{
		return 0;
	}

    return add_time(year, month, day, hour, dtime, unit);
}


