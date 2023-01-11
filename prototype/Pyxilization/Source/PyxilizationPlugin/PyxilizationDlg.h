#pragma once

class PyxilizationDlg;
//Global Declarations
static PyxilizationDlg *PyxDlg = NULL;




// PyxilizationDlg dialog


class PyxilizationDlg : public CDialog, public PYXDialogEventHandler
{
	DECLARE_DYNAMIC(PyxilizationDlg)

public:
	PyxilizationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PyxilizationDlg();
	virtual bool create(HWND hParent);
	virtual void documentLoad(){};
	virtual void documentClose(){};
	virtual void monitorDataSources(){};
	int getHeight(){return 80;}
	void setOwner(HWND, PlugInPyxis*, PlugInDLL*);
	//! Handle SetSelectedCell function.
	virtual void setSelectedCell(const std::string& strIndex);
	void EndTurn();
	void UpdateVisibleWindow();
	void LoadPlane(int);
	void UnloadPlane(int);
	void UpdateTab();
	void Move(bool);
	void SetType();
	void BuyUnits();

// Dialog Data
	enum { IDD = IDD_PyxilizationPlugin };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//! Pointer to class that allows us to call functions in Pyxis.
	PlugInPyxis* m_pPlugIn;
    
	//! Pointer to class that allows us to call functions in Pyxis.
	PlugInDLL* m_pPlugInDLL;

	//! Handle to owner window (ie. the dialog or client we draw in.
	HWND m_hOwnerWindow;

	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_TabControl;
	CUnits UnitsPage;
	CSetupDlg SetupPage;
	CControlDlg ControlsPage;
	CStatsDlg StatsPage;

	afx_msg void OnTcnSelchangeGamesetup(NMHDR *pNMHDR, LRESULT *pResult);
	int m_NumPlayers;
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	
};
