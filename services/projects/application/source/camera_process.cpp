/******************************************************************************
camera_process.cpp

begin      : 23/06/2008 5:27:27 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#include "camera_process.h"

// {C2FEA4BA-A73E-4ad0-85D2-349018E92DB4}
PYXCOM_DEFINE_CLSID(CameraProcess, 
0xc2fea4ba, 0xa73e, 0x4ad0, 0x85, 0xd2, 0x34, 0x90, 0x18, 0xe9, 0x2d, 0xb4);
PYXCOM_CLASS_INTERFACES(CameraProcess, ICameraView::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(CameraProcess, "Camera View", "A way of storing a camera location portion of a ViewPoint.", "Utility",
					ICameraView::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END