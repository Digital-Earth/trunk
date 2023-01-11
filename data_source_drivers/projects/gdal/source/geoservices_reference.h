#ifndef GEOSERVICES_REFERENCE_INTERFACE_H
#define GEOSERVICES_REFERENCE_INTERFACE_H

/******************************************************************************
geoservices_reference.h

begin      : 2016-02-09
copyright  : (c) 2016 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes

// pyxlib includes

//! IGeoServicesReference - interface to allow a process to expose a GeoServices reference. Used by ArcGIS FeatureServer to expose its output to GeoService processes
struct MODULE_GDAL_DECL IGeoServicesReference : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:
	//! Gets the URI for the service
	virtual const std::string& getUri() const = 0;

};

//! IGeoServicesFeatureServerReference - interface to allow process to to expose itself as a FeatureServer reference
struct MODULE_GDAL_DECL IGeoServicesFeatureServerReference : public IGeoServicesReference
{
	PYXCOM_DECLARE_INTERFACE();
};

#endif