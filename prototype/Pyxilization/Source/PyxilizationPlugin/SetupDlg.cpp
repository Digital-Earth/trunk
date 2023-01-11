// SetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PyxilizationPlugin.h"
#include "SetupDlg.h"

#include "plugin_pyxis.h"
#include "pyx_dialog_event_handler.h"
#include "ControlDlg.h"
#include "StatsDlg.h"
#include "Units.h"
#include "PyxilizationDlg.h"



extern int NumPLAYERS;


// CSetupDlg dialog

IMPLEMENT_DYNAMIC(CSetupDlg, CDialog)

CSetupDlg::CSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetupDlg::IDD, pParent)
	//, NumberOfPlayers(0)
	, m_STARTGAME(false)
{

}

CSetupDlg::~CSetupDlg()
{
}

void CSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NumOfPlayers, m_NumPlayers);
	DDX_Control(pDX, IDC_Player1, m_P1Name);
	DDX_Control(pDX, IDC_Player2, m_P2Name);
	DDX_Control(pDX, IDC_Player3, m_P3Name);
	DDX_Control(pDX, IDC_Player4, m_P4Name);
	DDX_Control(pDX, IDC_lbl1, m_1Label);
}


BEGIN_MESSAGE_MAP(CSetupDlg, CDialog)
	ON_BN_CLICKED(IDC_BtnStartGame, &CSetupDlg::OnBnClickedBtnstartgame)
	ON_CBN_SELCHANGE(IDC_NumOfPlayers, &CSetupDlg::OnCbnSelchangeNumofplayers)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSetupDlg message handlers

void CSetupDlg::OnBnClickedBtnstartgame()
{
	//Get data from controls
	UpdateData(TRUE);

	//m_STARTGAME=true;

	for(int i=0; i<m_NumPlayers.GetCurSel()+1; i++)
	{
		switch(i)
		{
			case 0:
				if(m_P1Name.GetWindowTextLengthA()<=0)
				{
					MessageBox("Player 1: Please Enter Your Name");
					m_STARTGAME=false;
				}
				else
				{
					m_STARTGAME=true;
				}
				break;
			case 1:
				if(m_P2Name.GetWindowTextLengthA()<=0)
				{
					MessageBox("Player 2: Please Enter Your Name");
					m_STARTGAME=false;
				}
				break;
			case 2:
				if(m_P3Name.GetWindowTextLengthA()<=0)
				{
					MessageBox("Player 3: Please Enter Your Name");
					m_STARTGAME=false;
				}
				break;
			case 3:
				if(m_P4Name.GetWindowTextLengthA()<=0)
				{
					MessageBox("Player 4: Please Enter Your Name");
					m_STARTGAME=false;
				}
				break;
			default:
				MessageBox("Please Enter Number of Players!");
				m_STARTGAME=false;
				break;	
		}
	}
	
	if(m_STARTGAME==true)
	{
		//Once Setup is complete update tabs so user can start playing
		PyxDlg->UpdateTab();	
	}

	return;
}

void CSetupDlg::EnableDisableComboBoxes(bool P2, bool P3, bool P4)
{
	//Disable/Enable Edit Boxes
	m_P2Name.EnableWindow(P2);
	m_P3Name.EnableWindow(P3);
	m_P4Name.EnableWindow(P4);

	return;
}

BOOL CSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_STARTGAME=false;

	//Adds players to NumPlayers combobox
	m_NumPlayers.AddString("1");
	m_NumPlayers.AddString("2");
	m_NumPlayers.AddString("3");
	m_NumPlayers.AddString("4");



	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CSetupDlg::OnCbnSelchangeNumofplayers()
{
	int nIndex=m_NumPlayers.GetCurSel();
	NumPLAYERS=m_NumPlayers.GetCurSel()+1;
	
	//Calls Function to enable and disable the Name Boxes and Colour ComboBoxes with it
	switch(nIndex)
	{
		case 0:
			EnableDisableComboBoxes(false,false,false);
			break;
		case 1:
			EnableDisableComboBoxes(true,false,false);
			break;
		case 2:
			EnableDisableComboBoxes(true,true,false);
			break;
		case 3:
			EnableDisableComboBoxes(true,true,true);
			break;
		default:
			EnableDisableComboBoxes(false,false,false);
			break;
	}
}

void CSetupDlg::OnPaint()
{
	CPaintDC dc(this);

	//Sets up Rectangle Object
	CRect Dialog;
	//Gets Dialogs Height and Width
	GetClientRect(&Dialog);
	
	//Colors Dialogs Background Black
	dc.FillSolidRect(&Dialog,RGB(0,0,0));
	
}

HBRUSH CSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = ::CreateSolidBrush(RGB(0,0,0)); //= __super::OnCtlColor(pDC, pWnd, nCtlColor);
	
	//Makes all static controls have a transparent background
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,255,255));

	return hbr;
}
