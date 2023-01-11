#pragma once
#include "afxwin.h"

// CSetupDlg dialog

class CSetupDlg : public CDialog
{
	DECLARE_DYNAMIC(CSetupDlg)

public:
	CSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetupDlg();

// Dialog Data
	enum { IDD = IDD_SetupPage };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL CSetupDlg::OnInitDialog();
	void EnableDisableComboBoxes(bool, bool, bool);

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_NumPlayers;
	afx_msg void OnBnClickedBtnstartgame();
	CEdit m_P1Name;
	CEdit m_P2Name;
	CEdit m_P3Name;
	CEdit m_P4Name;
	afx_msg void OnCbnSelchangeNumofplayers();
	bool m_STARTGAME;
	afx_msg void OnPaint();
	CStatic m_1Label;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
