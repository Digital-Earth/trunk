#ifndef XY_BOUNDS_REGION_PROC_H
#define XY_BOUNDS_REGION_PROC_H
/******************************************************************************
xy_bounds_region_proc.h

begin		: 2011-03-22
copyright	: (C) 2011 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"
#include "pyxis/data/feature.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/pyxcom.h"

class PYXXYBoundsRegion;

class PYXLIB_DECL XYBoundsRegionProc : 
	public ProcessImpl< XYBoundsRegionProc >, 
	public IFeature
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END
 
	IUNKNOWN_RC_IMPL_FINALIZE();

public: //IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr< PYXCOM_IUnknown const > STDMETHODCALLTYPE getOutput() const
	{
		return static_cast< IFeature const * >(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr< PYXCOM_IUnknown > STDMETHODCALLTYPE getOutput()
	{
		return static_cast< IFeature * >(this);
	}

	//! Get the schema to describe how the attributes should be edited.
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	/*!
	Get the attributes associated with  this process. 

	\return a map of standard string - standard string containing the attributes to be serialized.
	*/
	virtual std::map< std::string, std::string > STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes of this process. 
	virtual void STDMETHODCALLTYPE setAttributes(std::map< std::string, std::string > const & mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IFeature

	IFEATURE_IMPL();

public: // IRecord

	IRECORD_IMPL();
	
public: //XYBoundsRegionProc

	XYBoundsRegionProc();

private:

	//! Convert from lat lon to PYXIS
	WGS84CoordConverter m_converter;

	double m_xMin;
	double m_yMin;
	double m_xMax;
	double m_yMax;
};

#endif //end guard
