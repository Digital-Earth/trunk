#ifndef GEO_FILE_LOCATOR_PROC_H
#define GEO_FILE_LOCATOR_PROC_H
/******************************************************************************
geo_file_locator_proc.h

begin		: 2007-10-18
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "module_pyxis_coverages.h"

#include "pyxis/pipe/process.h"
#include "pyxis/data/feature.h"

/*!
Geolocates a file on the globe by associating a with a pyxis geometry. This process accepts a path
process and a feature process as it's inputs. The feature process contains the geometry that the path
process is to be geolocated to. The Path process contains the file that is to be geolocated on the earth.
*/
//! Geolocates a file on the earth with a Pyxis geometry.
class MODULE_PYXIS_COVERAGES_DECL GeoRerefencedFileProcess : public ProcessImpl<GeoRerefencedFileProcess>, public IFeature
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: //IRecord

	IRECORD_IMPL();

public: //IProcess 

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeature*>(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeature*>(this);
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IFeature

	//! Determine if this feature is writable or not.
	virtual bool STDMETHODCALLTYPE isWritable() const;

	//! Get this feature's Id.
	virtual const std::string& STDMETHODCALLTYPE getID() const;

	//! Get the geometry of this feature.
	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const;
	
	//! Get the geometry of this feature.
	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry();

	//! Get the style that determines how to display this feature.
	virtual std::string STDMETHODCALLTYPE getStyle() const;

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const;

public: //FileGeoLocatorProcess

	static const std::string kstrPdf;
	static const std::string kstrDoc;
	static const std::string kstrDocx;

	//! Default Constructor
	GeoRerefencedFileProcess(){;}

protected:
	//! Destructor
	virtual ~GeoRerefencedFileProcess(){;}

private:

	//! The feature that the file is associated with.
	boost::intrusive_ptr<IFeature> m_spAssociatedFeature;

	//! The Id of this feature.
	std::string m_strFeatureID;

};

#endif //end guard 

