// mfc_sampleDlg.h : header file
//

#if !defined(AFX_MFC_SAMPLEDLG_H__7F4D8313_C51F_4E0D_AAC1_EC7C7F2F875C__INCLUDED_)
#define AFX_MFC_SAMPLEDLG_H__7F4D8313_C51F_4E0D_AAC1_EC7C7F2F875C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDlg dialog

class gist;

class CMfc_sampleDlg : public CDialog
{
// Construction
public:
	CMfc_sampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMfc_sampleDlg)
	enum { IDD = IDD_MFC_SAMPLE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMfc_sampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	int query(gist& sr_index);

	// Generated message map functions
	//{{AFX_MSG(CMfc_sampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFC_SAMPLEDLG_H__7F4D8313_C51F_4E0D_AAC1_EC7C7F2F875C__INCLUDED_)
