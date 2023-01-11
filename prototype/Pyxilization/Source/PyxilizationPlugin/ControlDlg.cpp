// ControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PyxilizationPlugin.h"
#include "plugin_pyxis.h"
#include "pyx_dialog_event_handler.h"
#include "SetupDlg.h"
#include "ControlDlg.h"
#include "StatsDlg.h"
#include "Units.h"
#include "PyxilizationDlg.h"



extern int ThePlayer;



// CControlDlg dialog

IMPLEMENT_DYNAMIC(CControlDlg, CDialog)

CControlDlg::CControlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CControlDlg::IDD, pParent)
{

}

CControlDlg::~CControlDlg()
{
}

void CControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Cell1, m_Cell1Index);
	DDX_Control(pDX, IDC_Cell2, m_Cell2Index);
	DDX_Control(pDX, IDC_PlayersTurn, m_PlayersName);
	DDX_Control(pDX, IDC_PICTURE, m_UnitDisplay);
	DDX_Control(pDX, IDC_COLOR_CHANGER, m_ColorChanger);
}


BEGIN_MESSAGE_MAP(CControlDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_EndTurn, &CControlDlg::OnBnClickedEndturn)
END_MESSAGE_MAP()


// CControlDlg message handlers

void CControlDlg::OnPaint()
{
	CPaintDC dc(this);

	//Sets up Rectangle Object
	CRect Dialog;

	//Gets Dialogs Height and Width
	GetClientRect(&Dialog);

	//Colors Dialogs Background Black
	dc.FillSolidRect(&Dialog,RGB(0,0,0));

	//-----------------------Player Color----------------------------

	CRect RectColorChanger;

	// Get the location and dimensions of the control
	m_ColorChanger.GetWindowRect(&RectColorChanger);
	CBrush BrushRed = (RGB(255, 0, 0));
	CBrush BrushGreen = (RGB(0, 255, 0));
	CBrush BrushBlue = (RGB(0, 0, 255));
	CBrush BrushPurple = (RGB(255, 0, 255));
	CBrush *pOldBrush = dc.SelectObject(&BrushRed);
	
	//Depending on player
	switch(ThePlayer)
	{	
		// Select the new brush
		case 1: pOldBrush = dc.SelectObject(&BrushRed);break;
		case 2: pOldBrush = dc.SelectObject(&BrushGreen);break;
		case 3: pOldBrush = dc.SelectObject(&BrushBlue);break;
		case 4: pOldBrush = dc.SelectObject(&BrushPurple);break;
		default:break;
	}

	// Convert the current coordinates from Screen to Client
	ScreenToClient(&RectColorChanger);
	// Change the background of the control
	dc.Rectangle(RectColorChanger);
}


HBRUSH CControlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = ::CreateSolidBrush(RGB(0,0,0)); //= __super::OnCtlColor(pDC, pWnd, nCtlColor);
	
	//Makes all static controls have a transparent background
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,255,255));

	return hbr;
}

void CControlDlg::OnBnClickedEndturn()
{

	//End users turn
	PyxDlg->EndTurn();
}
