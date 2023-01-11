#ifndef CAMERA_VIEW_H
#define CAMERA_VIEW_H
/******************************************************************************
camera_view.h

begin      : 17/06/2008 4:27:27 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "application.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

//! A process that stores the definition of a unique place on the globe.
struct APPLICATION_API ICameraView : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	/*!
	This method will return the exact position of a camera as a string. If the 
	camera has not been initialized the string that is returned will be empty.

	\return The string representation of a camera view.
	*/
	//! Request a camera position as a string
	virtual const std::string STDMETHODCALLTYPE getCameraView() const = 0;

	/*!
	Set the camera view using a formatted string. If the string is not properly 
	formatted and exception is thrown.

	\param strView	A formatted string that represents a camera location (camera
					cookie string).
	*/
	//! Store a camera view as specified by a formatted string.
	virtual void STDMETHODCALLTYPE setCameraView(const std::string& strView) = 0;

// enable when this process can be moved into the view model and the camera 
// object can be accessed.
#if 0
	/*!
	This method will return the exact position of a camera as a string. If the
	process has not be initialized a default camera is returned.

	\return	A camera object that is initialized to the location of the view.
	*/
	//! Create a camera from the stored view.
	virtual const Camera STDMETHODCALLTYPE getCamera() const = 0;

	/*!
	Save the camera view currently used in a camera object.

	\param camera	A camera object to set the view with.
	*/
	//! Store a camera view as specified by a camera object
	virtual void STDMETHODCALLTYPE setCamera(const Camera& camera) = 0;
#endif
};

#endif // guard