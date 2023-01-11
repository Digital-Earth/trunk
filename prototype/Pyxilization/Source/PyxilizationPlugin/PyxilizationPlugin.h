// PyxilizationPlugin.h : main header file for the PyxilizationPlugin DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPyxilizationPluginApp
// See PyxilizationPlugin.cpp for the implementation of this class
//

class CPyxilizationPluginApp : public CWinApp
{
public:
	CPyxilizationPluginApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
