// Units.cpp : implementation file
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

//From Pyxilization Dlg
extern int type[10];
extern int playerID[10];
extern int featID[10];
extern int HealthID[10];
extern int BuildType[10];
extern bool ToggleCell;


int NumOfUnits[3];
int UnitCreationType=0;
bool UNITSSELECTED=false;


// CUnits dialog

IMPLEMENT_DYNAMIC(CUnits, CDialog)

CUnits::CUnits(CWnd* pParent /*=NULL*/)
	: CDialog(CUnits::IDD, pParent)
{

}

CUnits::~CUnits()
{
}

void CUnits::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_chkSoldier, m_chkSoldier);
	DDX_Control(pDX, IDC_chkAirplane, m_chkAirplane);
	DDX_Control(pDX, IDC_chkTank, m_chkTank);
	DDX_Control(pDX, IDC_NumSoldiers, m_NumOfSoldiers);
	DDX_Control(pDX, IDC_NumAirplane, m_NumOfAirplanes);
	DDX_Control(pDX, IDC_NumTank, m_NumOfTanks);
	DDX_Control(pDX, IDC_SelectType, m_SelectType);
	DDX_Control(pDX, IDC_btnLoad, m_btnLoad);
	DDX_Control(pDX, IDC_btnUnload, m_btnUnload);
	DDX_Control(pDX, IDC_BuyUnits, m_BuyUnits);
}


BEGIN_MESSAGE_MAP(CUnits, CDialog)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_chkSoldier, &CUnits::OnBnClickedchksoldier)
	ON_BN_CLICKED(IDC_chkAirplane, &CUnits::OnBnClickedchkairplane)
	ON_BN_CLICKED(IDC_chkTank, &CUnits::OnBnClickedchktank)
	ON_BN_CLICKED(IDC_btnLoad, &CUnits::OnBnClickedbtnload)
	ON_BN_CLICKED(IDC_btnUnload, &CUnits::OnBnClickedbtnunload)
	ON_BN_CLICKED(IDC_BuyUnits, &CUnits::OnBnClickedBuyunits)
END_MESSAGE_MAP()


// CUnits message handlers

void CUnits::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	//Sets up Rectangle Object
	CRect Dialog;
	//Gets Dialogs Height and Width
	GetClientRect(&Dialog);
	
	//Colors Dialogs Background Black
	dc.FillSolidRect(&Dialog,RGB(0,0,0));
}

HBRUSH CUnits::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = ::CreateSolidBrush(RGB(0,0,0)); //= __super::OnCtlColor(pDC, pWnd, nCtlColor);
	
	//Makes all static controls have a transparent background
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,255,255));

	return hbr;
}

void CUnits::GetUnitInformation()
{
	UNITSSELECTED=false;
	char Temp[5];
	
	//Enable selection boxes when Dialog is called
	for(int i=0; i<10; i++)
	{
		switch(type[i])
		{
			//if soldier
			case 0:
				
				m_chkSoldier.EnableWindow(true);
				m_NumOfSoldiers.SetWindowText(itoa(HealthID[i],Temp,10));

				CheckDlgButton(IDC_chkSoldier, BST_CHECKED);
				OnBnClickedchksoldier();
			break;
			//if airplane
			case 1:
			
				m_chkAirplane.EnableWindow(true);
				m_NumOfAirplanes.SetWindowText(itoa(HealthID[i],Temp,10));
				
				CheckDlgButton(IDC_chkAirplane, BST_CHECKED);
				OnBnClickedchkairplane();

				m_btnLoad.EnableWindow(true);
				m_btnUnload.EnableWindow(true);
			break;
			case 2:
				//Lets allow user to buy some units
				m_BuyUnits.EnableWindow(true);
				//if Soldier
				if(BuildType[i]==0)
				{
					m_SelectType.EnableWindow(true);
					m_SelectType.SetCurSel(0);
				}

				//if Tank
				if(BuildType[i]==4)
				{
					m_SelectType.EnableWindow(true);
					m_SelectType.SetCurSel(1);
					
				}
				
			break;
			//if tank
			case 4:
				
				m_chkTank.EnableWindow(true);
				m_NumOfTanks.SetWindowText(itoa(HealthID[i],Temp,10));

				CheckDlgButton(IDC_chkTank, BST_CHECKED);
				OnBnClickedchktank();
			break;
			default:
			break;
		}
	}

}
void CUnits::FINISHED()
{
	CString Temp;
	//Grab information
	UpdateData(TRUE);
	int nIndex=0;

	UNITSSELECTED=true;
	
	//Get num of soldiers to move
	if(m_NumOfSoldiers.GetWindowTextLengthA()>0)
	{
		m_NumOfSoldiers.GetWindowTextA(Temp);
		NumOfUnits[0]=atoi(Temp);
	}
	else
		NumOfUnits[0]=0;
	
	//Get num of airplanes to move
	if(m_NumOfAirplanes.GetWindowTextLengthA()>0)
	{
		m_NumOfAirplanes.GetWindowTextA(Temp);
		NumOfUnits[1]=atoi(Temp);
	}
	else
		NumOfUnits[1]=0;
	
	//Get num of tanks to move
	if(m_NumOfTanks.GetWindowTextLengthA()>0)
	{
		m_NumOfTanks.GetWindowTextA(Temp);
		NumOfUnits[2]=atoi(Temp);
	}
	else
		NumOfUnits[2]=0;
	
	//Disable everything
	//Soldier Disable
	CheckDlgButton(IDC_chkSoldier, BST_UNCHECKED);
	OnBnClickedchksoldier();
	m_chkSoldier.EnableWindow(false);
	m_NumOfSoldiers.EnableWindow(false);
	m_NumOfSoldiers.SetWindowTextA("");
	
	//Airplane Disable
	CheckDlgButton(IDC_chkAirplane, BST_UNCHECKED);
	OnBnClickedchkairplane();
	m_chkAirplane.EnableWindow(false);
	m_NumOfAirplanes.EnableWindow(false);
	m_NumOfAirplanes.SetWindowTextA("");
	m_btnLoad.EnableWindow(false);
	m_btnUnload.EnableWindow(false);
	
	//Tank Disable
	CheckDlgButton(IDC_chkTank, BST_UNCHECKED);
	OnBnClickedchktank();
	m_chkTank.EnableWindow(false);
	m_NumOfTanks.EnableWindow(false);
	m_NumOfTanks.SetWindowTextA("");
	
	//disable buy units button
	m_BuyUnits.EnableWindow(false);

	nIndex=m_SelectType.GetCurSel();
	
	//change factory creation type
	switch(nIndex)
	{
	case 0:
		UnitCreationType=0;
		break;
	case 1:
		UnitCreationType=4;
		break;
	case -1:
		break;
	}

	//Disable selection
	m_SelectType.EnableWindow(false);

	//Update the plug-in to make controls/stats dialogs to reappear	
	PyxDlg->UpdateVisibleWindow();

}

BOOL CUnits::OnInitDialog()
{
	CDialog::OnInitDialog();

	//Lets add some build types
	m_SelectType.AddString("Soldiers");
	m_SelectType.AddString("Tanks");

	return TRUE;  // return TRUE  unless you set the focus to a control

}
void CUnits::OnBnClickedchksoldier()
{	
	static bool Check=false;

	Check=!Check;

	m_NumOfSoldiers.EnableWindow(Check);

	if(Check==false)
	{	
		m_NumOfSoldiers.SetWindowTextA("");
	}


	if(UNITSSELECTED==true)
	{
		m_NumOfSoldiers.EnableWindow(false);
		Check=false;
	}	
}

void CUnits::OnBnClickedchkairplane()
{
	static bool Check=false;

	Check=!Check;

	m_NumOfAirplanes.EnableWindow(Check);
	m_btnLoad.EnableWindow(Check);
	m_btnUnload.EnableWindow(Check);
	
	if(Check==false)
	{
		m_NumOfAirplanes.SetWindowTextA("");
		m_btnLoad.EnableWindow(false);
		m_btnUnload.EnableWindow(false);
	}

	if(UNITSSELECTED==true)
	{
		m_NumOfAirplanes.EnableWindow(false);
		Check=false;
	}
	

}

void CUnits::OnBnClickedchktank()
{
	static bool Check=false;
	
	Check=!Check;

	m_NumOfTanks.EnableWindow(Check);

	if(Check==false)
	{
		m_NumOfTanks.SetWindowTextA("");
	}
		
	if(UNITSSELECTED==true)
	{
		m_NumOfTanks.EnableWindow(false);
		Check=false;
	}
	
}


void CUnits::OnBnClickedbtnload()
{
	int Load;
	CString buff;

	m_NumOfAirplanes.GetWindowTextA(buff);

	Load=atoi(buff);
	//Load up a plane
	PyxDlg->LoadPlane(Load);
}

void CUnits::OnBnClickedbtnunload()
{
	int Unload;
	CString buff;

	m_NumOfAirplanes.GetWindowTextA(buff);

	Unload=atoi(buff);
	//Load up a plane
	PyxDlg->UnloadPlane(Unload);
}

void CUnits::OnBnClickedBuyunits()
{
	//Lets buy some units     (Pyxilization Dialog)
	PyxDlg->BuyUnits();
}
