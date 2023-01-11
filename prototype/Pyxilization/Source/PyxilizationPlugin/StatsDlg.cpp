// StatsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PyxilizationPlugin.h"
#include "plugin_pyxis.h"
#include "pyx_dialog_event_handler.h"
#include "StatsDlg.h"
#include "ControlDlg.h"
#include "SetupDlg.h"
#include "Units.h"
#include "PyxilizationDlg.h"




// CStatsDlg dialog

IMPLEMENT_DYNAMIC(CStatsDlg, CDialog)

CStatsDlg::CStatsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatsDlg::IDD, pParent)
{

}

CStatsDlg::~CStatsDlg()
{
}

void CStatsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Name1, m_Name1);
	DDX_Control(pDX, IDC_Units, m_SoldiersStats);
	DDX_Control(pDX, IDC_Name2, m_Name2);
	DDX_Control(pDX, IDC_Name3, m_Name3);
	DDX_Control(pDX, IDC_Name4, m_Name4);
	DDX_Control(pDX, IDC_Airplanes, m_AirPlaneStats);
	DDX_Control(pDX, IDC_Airports, m_AirportsStats);
	DDX_Control(pDX, IDC_Factories, m_FactoryStats);
	DDX_Control(pDX, IDC_Factories2, m_TankStats);
}


BEGIN_MESSAGE_MAP(CStatsDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CStatsDlg message handlers

void CStatsDlg::OnPaint()
{
	CPaintDC dc(this);
	//Sets up Rectangle Object
	CRect Dialog;
	//Gets Dialogs Height and Width
	GetClientRect(&Dialog);
	
	//Colors Dialogs Background Black
	dc.FillSolidRect(&Dialog,RGB(0,0,0));
}

HBRUSH CStatsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = ::CreateSolidBrush(RGB(0,0,0)); //= __super::OnCtlColor(pDC, pWnd, nCtlColor);
	
	//Makes all static controls have a transparent background
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,255,255));

	return hbr;
}

void CStatsDlg::AddStats(int playa, CString Stats)
{
	char Buff[50];
	int Position;
	int fill=0;
	int space=0;

	static char SoldierStats[100];
	static char AirplaneStats[100];
	static char AirportStats[100];
	static char FactoryStats[100];
	static char TankStats[100];
	
	//Copy stats to a temp buff
	strcpy_s(Buff,Stats);

	//MessageBox(Buff);
	
	//Switch between players and set position to start 
	switch(playa)
	{
		case 0:
			//Initialize the buffs with spaces for display purposes
			for(int i=0;i<100;i++)
			{
				SoldierStats[i]=' ';
				AirplaneStats[i]=' ';
				AirportStats[i]=' ';
				FactoryStats[i]=' ';
				TankStats[i]=' ';
			}
			Position=0;
			break;
		case 1:Position=20;break;
		case 2:Position=40;break;
		case 3:Position=60;break;
	}
	
	//Fill all the fields
	for(int j=0; j<50; j++)
	{
		switch(fill)
	    {
			//Soldiers
			case 0:
				if(Buff[j]=='\n')
				{
					space=0;
					fill=1;
					break;
				}
				SoldierStats[Position+space]=Buff[j];
				space++;
				break;
			//Airplanes
			case 1:
				if(Buff[j]=='\n')
				{
					space=0;
					fill=2;
					break;
				}
				AirplaneStats[Position+space]=Buff[j];
				space++;
				break;
			//Factories
			case 2:
				if(Buff[j]=='\n')
				{
					space=0;
					fill=3;
					break;
				}
				FactoryStats[Position+space]=Buff[j];
				space++;
				break;
			//Airports
			case 3:
				if(Buff[j]=='\n')
				{
					space=0;
					fill=4;
					break;
				}
				AirportStats[Position+space]=Buff[j];
				space++;
				break;
			//Tanks
			case 4:
				if(Buff[j]=='\n')
				{
					space=0;
					fill=-1;
					break;
				}
				TankStats[Position+space]=Buff[j];
				space++;
				break;
		}	
	}
	
	//Display all the stats
	m_SoldiersStats.SetWindowTextA(SoldierStats);
	m_AirPlaneStats.SetWindowTextA(AirplaneStats);
	m_FactoryStats.SetWindowTextA(FactoryStats);
	m_AirportsStats.SetWindowTextA(AirportStats);
	m_TankStats.SetWindowTextA(TankStats);
	
	return;
}

