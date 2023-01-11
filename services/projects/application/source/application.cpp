// application.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "application.h"

#include "camera_process.h"
#include "document_process.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(DocumentProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CameraProcess),
PYXCOM_END_CLASS_OBJECT_TABLE
