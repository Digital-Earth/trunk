#pragma once
#include "afxwin.h"


//class CControlDlg;


// CControlDlg dialog

class CControlDlg : public CDialog
{
	DECLARE_DYNAMIC(CControlDlg)

public:
	CControlDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CControlDlg();

// Dialog Data
	enum { IDD = IDD_ControlsPage };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	PlugInPyxis* m_pPlugIn;

	DECLARE_MESSAGE_MAP()
public:
	int NumberOfPlayers;
	CEdit m_Cell1Index;
	CEdit m_Cell2Index;
	afx_msg void OnPaint();
	afx_msg void OnBnClickedMove();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedEndturn();
	CStatic m_PlayersName;
	CStatic m_UnitDisplay;
	CStatic m_ColorChanger;
};
