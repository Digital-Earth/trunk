// WMSClientDLL.h : main header file for the WMSClientDLL DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWMSClientDLLApp
// See WMSClientDLL.cpp for the implementation of this class
//

class CWMSClientDLLApp : public CWinApp
{
public:
	CWMSClientDLLApp();

	~CWMSClientDLLApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
