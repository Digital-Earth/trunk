// mfc_sample.h : main header file for the MFC_SAMPLE application
//

#if !defined(AFX_MFC_SAMPLE_H__3B15E877_05B6_4129_BCEA_10262906B95A__INCLUDED_)
#define AFX_MFC_SAMPLE_H__3B15E877_05B6_4129_BCEA_10262906B95A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleApp:
// See mfc_sample.cpp for the implementation of this class
//

class CMfc_sampleApp : public CWinApp
{
public:
	CMfc_sampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMfc_sampleApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMfc_sampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFC_SAMPLE_H__3B15E877_05B6_4129_BCEA_10262906B95A__INCLUDED_)
