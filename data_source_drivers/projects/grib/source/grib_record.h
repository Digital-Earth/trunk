#ifndef GRIB_RECORD_H
#define GRIB_RECORD_H
/******************************************************************************
grib_record_.h

begin		: 2006-03-20
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "grib.h"

// Standard includes.
#include <vector>
#include <string>

/*!
This class wraps the various parameters and data values associated
with a record from a GRIB file.
*/
class GribRecord
{
public:

	//! Default constructor.
	GribRecord(){;}

	//! Destructor.
	virtual ~GribRecord(){;}


	/*!
	Set name of file that contains GRIB record.

	\param strFileName	Name of file.
	*/
	//! Set name of file that contains GRIB record.
	void setFileName(const std::string& strFileName) { m_strFileName = strFileName; }
	
	/*!
	Set the record number of the GRIB record.

	\param nRecordNumber	Record number.
	*/
	//! Set record number of GRIB record.
	void setRecordNumber(int nRecordNumber) { m_nRecordNumber = nRecordNumber; }

	/*!
	Set minumum latitude value of bounds of GRIB record.

	\param fLatMin	Latitude of start of data.
	*/
	//! Set minimum latitude value of bounds of GRIB record.
	void setLatMin(double fLatMin) { m_fLatMin = fLatMin; }

	/*!
	Set minimum longitude value of bounds of GRIB record.

	\param	fLonMin	Longitude of start of data.
	*/
	//! Set minimum longitude value of bounds of GRIB record.
	void setLonMin(double fLonMin) { m_fLonMin = fLonMin; }

	/*!
	Set maximum latitude value of bounds of GRIB record.
	
	\param	fLatMax	Latitude of end of data.
	*/
	//! Set maximum latitude value of bounds of GRIB record.
	void setLatMax(double fLatMax) { m_fLatMax = fLatMax; }

	/*!
	Set maximum longitude value of bounds of GRIB record.
	
	\param	fLonMax	Longitude of end of data.
	*/
	//! Set maximum longitude value of bounds of GRIB record.
	void setLonMax(double fLonMax) { m_fLonMax = fLonMax; }

	/*!
	Set latitude increment of GRIB record.

	\param	fLatStep	Latitude increment of data in record.
	*/
	//! Set latitude increment of GRIB record.
	void setLatStep(double fLatStep) { m_fLatStep = fLatStep; }

	/*!
	Set longitude increment of GRIB record.

	\param	fLonStep	Longitude increment of data in record.
	*/
	//! Set longitude increment of GRIB record.
	void setLonStep(double fLonStep) { m_fLonStep = fLonStep; }

	/*!
	Set minimum values of data in GRIB record.

	\param	fMinValue	Minimum value of data in this record.
	*/
	//! Set minimum values of data in GRIB record.
	void setMinValue(double fMinValue) { m_fMinValue = fMinValue; }

	/*!
	Set maximum value of data in GRIB record.

	\param	fMaxValue	Maximum value of data in this record.
	*/
	//! Set maximum value of data in GRIB record.
	void setMaxValue(double fMaxValue) { m_fMaxValue = fMaxValue; }

	/*!
	true if data read bottom up on record (standard) else false for top down.

	\param	bBottomUp	true if data read from bottom (standard), else false for top down.
	*/
	//! true if data read bottom up on record (standard) else false for top down.
	void setBottomUp(bool bBottomUp = true) { m_bBottomUp = bBottomUp; }

	/*!
	Set number of data values in the X (longitude) direction.

	\param	nBandXSize	Number of data value in the X (longitude) direction.
	*/
	//! Set number of data values in the X (longitude) direction.
	void setBandXSize(int nBandXSize) { m_nBandXSize = nBandXSize;
										m_fBandXSize = static_cast<double>(nBandXSize); }
	/*!
	Set number of data values in the Y (latitude) direction.

	\param	nBandYSize	Number of data values in the Y (latitude) direction.
	*/
	//! Set number of data values in the Y (latitude) direction.
	void setBandYSize(int nBandYSize) { m_nBandYSize = nBandYSize;
										m_fBandYSize = static_cast<double>(nBandYSize); }

	/*!
	Set number of data values in the X (longitude) direction.
	
	\param fBandXSize	Number of data values in the X (longitude) direction.
	*/
	//! Set number of data values in the X (longitude) direction.
	void setBandXSize(double fBandXSize) { m_fBandXSize = fBandXSize;
										   m_nBandXSize = static_cast<int>(fBandXSize); }

	/*!
	Set number of data values in the Y (latitude) direction.

	\param	fBandYSize	Number of data values in the Y (latitude) direction.
	*/
	//! Set number of data values in the Y (latitude) direction.
	void setBandYSize(double fBandYSize) { m_fBandYSize = fBandYSize;
										   m_nBandYSize = static_cast<int>(fBandYSize); }

	/*!
	Set description of the data contained in this record.

	\param	strDesc	Description of the data in this layer.
	*/
	//! Set description of the data contained in this record.
   void setDescription(const std::string& strDesc) { m_strDesc = strDesc; }

	/*!
    Name of file that contains GRIB record.

	\return	File name for this record.
	*/
    //! Name of file that contains GRIB record.
	const std::string& getFileName() const { return m_strFileName; }
	
	/*!
	Record number of GRIB record.
	
	\return	Record number of this record.
	*/
	//! Record number of GRIB record.
	const int getRecordNumber() const { return m_nRecordNumber; };

	/*!
	Vector to hold data values from record.
	
	\return	Reference to internal vector holding data.
	*/
	//! Vector to hold data values from record.
	std::vector<double>& getDataVector() { return m_vecData; }

	/*!
	Minimum latitude value of bounds of GRIB record.
	
	\return	Minimum latitude value.
	*/
	//! Minimum latitude value of bounds of GRIB record.
	double getLatMin() const { return m_fLatMin; }

	/*!
	Minimum longitude value of bounds of GRIB record.

	\return	Minimum longitude value.
	*/
	//! Minimum longitude value of bounds of GRIB record.
	double getLonMin() const { return m_fLonMin; }

	/*!
	Maximum latitude value of bounds of GRIB record.

	\return	Maximum latitude value.
	*/
	//! Maximum latitude value of bounds of GRIB record.
	double getLatMax() const { return m_fLatMax; }

	/*!
	Maximum longitude value of bounds of GRIB record.

	\return	Maximum longitude value.
	*/
	//! Maximum longitude value of bounds of GRIB record.
	double getLonMax() const { return m_fLonMax; }

	/*!
	Latitude increment of GRIB record.

	\return	Latitude increment of record.
	*/
	//! Latitude increment of GRIB record.
	double getLatStep() const { return m_fLatStep; }

	/*!
	Longitude increment of GRIB record.

	\return Longitude step of data.
	*/
	//! Longitude increment of GRIB record.
	double getLonStep() const { return m_fLonStep; }

	/*!
	Minimum values of data in GRIB record.

	\return Minimum value of data.
	*/
	//! Minimum values of data in GRIB record.
	double getMinValue() const { return m_fMinValue; }

	/*!
	 Maximum value of data in GRIB record.
	
	\return Maximum value of data.
	*/
	//! Maximum value of data in GRIB record.
	double getMaxValue() const { return m_fMaxValue; }

	/*!
	Is data read from bottom to top.

	return true if data read bottom up on record (standard) else false for top down.
	*/
	//! Is data read from bottom to top.
	bool isBottomUp() const { return m_bBottomUp; }

	/*!
	Number of data values in the X (longitude) direction.

	\return Number of data values in the X direction.
	*/
	//! Number of data values in the X (longitude) direction.
	int getBandXSize() const { return m_nBandXSize; }

	/*!
	Number of data values in the Y (latitude) direction.

	\return Number of data values in the Y direction.
	*/
	//! Number of data values in the Y (latitude) direction.
	int getBandYSize() const { return m_nBandYSize; }

	/*!
	Number of data values in the X (longitude) direction.

	\return Number of data values in the X direction.
	*/
	//! Number of data values in the X (longitude) direction.
	double getBandXSizeDouble() const { return m_fBandXSize; }

	/*!
	Number of data values in the Y (latitude) direction.

	\return Number of data values in the Y direction.
	*/
	//! Number of data values in the Y (latitude) direction.
	double getBandYSizeDouble() const { return m_fBandYSize; }

	/*!
	Description of the data contained in this record.

	\return Description of the data in this record.
	*/
	//! Description of the data contained in this record.
	const std::string& getDescription() const { return m_strDesc; }

private:

	//! Name of file that contains this record.
	std::string m_strFileName;

	//! Number of this record.
	int m_nRecordNumber;

	//! Vector to hold data values for the record.
	std::vector<double> m_vecData;

	//! Min lat value of bounds.
	double m_fLatMin;
	
	//! Min lon value of bounds.
	double m_fLonMin;

	//! Max lat value of bounds.
	double m_fLatMax;

	//! Max lon value of bounds.
	double m_fLonMax;

	//! Lat data step.
	double m_fLatStep;

	//! Lon data step.
	double m_fLonStep;

	//! Minimum data value in this record.
	double m_fMinValue;

	//! Maximum data value in this record.
	double m_fMaxValue;

	//! true if data read from bottom up, else false.
	bool m_bBottomUp;

	//! Number of data values along X (lon) data direction.
	int m_nBandXSize;

	//! Number of data values along Y (lat) data direction.
	int m_nBandYSize;

	//! Number of data values along X (lon) data direction.
	double m_fBandXSize;

	//! Number of data values along Y (lat) data direction.
	double m_fBandYSize;

	//! Description of the data in this record.
	std::string m_strDesc;

};

#endif 