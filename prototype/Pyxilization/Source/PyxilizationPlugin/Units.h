#pragma once
#include "afxwin.h"


// CUnits dialog

class CUnits : public CDialog
{
	DECLARE_DYNAMIC(CUnits)

public:
	CUnits(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUnits();
	void FINISHED();

// Dialog Data
	enum { IDD = IDD_Units };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void GetUnitInformation();
	afx_msg void OnBnClickedFinish();
	CButton m_chkSoldier;
	CButton m_chkAirplane;
	CButton m_chkTank;
	CEdit m_NumOfSoldiers;
	CEdit m_NumOfAirplanes;
	CEdit m_NumOfTanks;
	afx_msg void OnBnClickedchksoldier();
	afx_msg void OnBnClickedchkairplane();
	afx_msg void OnBnClickedchktank();
	afx_msg void OnBnClickedFinish2();
	afx_msg void OnBnClickedCancel();
	CButton m_chkSoldierCreation;
	CButton m_chkTankCreation;
	afx_msg void OnBnClickedchksoldiercreation();
	afx_msg void OnBnClickedchktankcreation();
	CComboBox m_SelectType;
	CButton m_btnLoad;
	CButton m_btnUnload;
	afx_msg void OnBnClickedbtnload();
	afx_msg void OnBnClickedbtnunload();
public:
	afx_msg void OnBnClickedBuyunits();
public:
	CButton m_BuyUnits;
};
