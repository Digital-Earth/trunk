#pragma once
#include "afxwin.h"


// CStatsDlg dialog

class CStatsDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatsDlg)

public:
	CStatsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatsDlg();
	void AddStats(int, CString);

// Dialog Data
	enum { IDD = IDD_StatsPage };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CStatic m_Name1;
	CStatic m_SoldiersStats;
	CStatic m_Name2;
	CStatic m_Name3;
	CStatic m_Name4;
	CStatic m_AirPlaneStats;
	CStatic m_AirportsStats;
	CStatic m_FactoryStats;
	CStatic m_TankStats;
};
