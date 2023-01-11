#ifndef CAMERA_PROCESS_H
#define CAMERA_PROCESS_H
/******************************************************************************
camera_process.h

begin      : 23/06/2008 5:27:27 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "camera_view.h"

/*!
This process defines a unique view onto the earth.
*/
//! A process that defines a view onto the earth.
class APPLICATION_API CameraProcess : public ProcessImpl<CameraProcess>, public ICameraView
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(ICameraView)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(CameraProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICameraView*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICameraView*>(this);
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl() 
	{
		if (getData() == "")
		{
			// Sensible default position
			TRACE_INFO("Setting camera position to default.");
			setData("45 -75 0 0 0 12742014");
		}
		return knInitialized;
	}

public: // ICameraView

	virtual const std::string STDMETHODCALLTYPE getCameraView() const
	{
		return getData();
	}

	virtual void STDMETHODCALLTYPE setCameraView(const std::string& strView)
	{
		setData(strView);
	}

// enable when this process can be moved into the view model and the camera 
// object can be accessed.
#if 0
	virtual const Camera STDMETHODCALLTYPE getCamera() const
	{
		Camera camera;
		camFromCookieStr(camera, getData());
		return camera;
	}

	virtual void STDMETHODCALLTYPE setCamera(const Camera& camera)
	{
		std::string camString;
		camToCookieStr(camera, camString);
		setCameraView(camString);
	}
#endif

};

#endif /* CAMERA_PROCESS_H */