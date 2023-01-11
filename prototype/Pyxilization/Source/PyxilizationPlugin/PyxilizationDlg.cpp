// PyxilizationDlg.cpp : implementation file
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
#include <string>

/*NOTE*/
/*Most methods in the this main PyxilizationDlg call methods out in its children dialogs...This is 
how information updates in the children dialogs.*/


//From Units Dlg
//0 - Soldiers
//1 - Airplane
//2 - Tank
extern int NumOfUnits[3];
extern int UnitCreationType;
bool UNITSELECTION;


//Trigger Declarations
bool ToggleCell=false;
int counter=0;

//Setup Declarations
int ThePlayer=1;
int NumPLAYERS=0;
char Name1[20],Name2[20],Name3[20],Name4[20];
char NumUnits[2];
char FeatID[5];
int Points=3;


int type[10];
int playerID[10];
int featID[10];
int HealthID[10];
int BuildType[10];


// PyxilizationDlg dialog

IMPLEMENT_DYNAMIC(PyxilizationDlg, CDialog)

PyxilizationDlg::PyxilizationDlg(CWnd* pParent /*=NULL*/)
	: CDialog()
	, m_NumPlayers(0)
{
}

PyxilizationDlg::~PyxilizationDlg()
{
}

void PyxilizationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GameSetup, m_TabControl);
}

bool PyxilizationDlg::create(HWND hParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	

	return Create(PyxilizationDlg::IDD, CWnd::FromHandle(hParent)) ? true : false;
}

void PyxilizationDlg::setOwner(HWND hOwner, PlugInPyxis* pPlugIn, PlugInDLL* pPlugInDLL)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_hOwnerWindow = hOwner;
	m_pPlugIn = pPlugIn;
	m_pPlugInDLL = pPlugInDLL;
}

BEGIN_MESSAGE_MAP(PyxilizationDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_GameSetup, &PyxilizationDlg::OnTcnSelchangeGamesetup)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// PyxilizationDlg message handlers


void DLLSetOwner(int nInstance, HWND hOwnerWindow, PlugInPyxis* pPlugInPyxis, PlugInDLL* pPlugInDLL)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if(!PyxDlg)
	{
		PyxDlg = new PyxilizationDlg();
	}
	PyxDlg->setOwner(hOwnerWindow, pPlugInPyxis, pPlugInDLL);
}

PYXDialogEventHandler* DLLGetPYXDialogEventHandler(int nInstance)
{
	return PyxDlg;
}

bool DLLAutoLoad(int nInstance)
{
	return true;
}

bool DLLDataSourceOpened(int nInstance, const char* szURI)
{
	MessageBoxA(NULL, szURI, "hi", MB_OK);
	return 1;
}

void PyxilizationDlg::OnTcnSelchangeGamesetup(NMHDR *pNMHDR, LRESULT *pResult)
{
	//Swaps Property Pages
	UpdateVisibleWindow();

	*pResult = 0;
}


//When a shift-click happens
void PyxilizationDlg::setSelectedCell(const std::string& strIndex)
{

	//Make sure you don't get any co-ordinates if the Controls Tab isn't selected
	if(m_TabControl.GetCurSel()==0 && SetupPage.m_STARTGAME==true)
	{
			/*Just uses already created dialog objects*/
			if(ToggleCell==false)
			{
				//Reset Feature counter
				counter=0;
				
				//Clear stats variables
				for(int i=0; i<10; i++)
				{
				   type[i]=-1;
				   playerID[i]=-1;
				   featID[i]=-1;
				   HealthID[i]=-1;
				   BuildType[i]=-1;
				}

				//Grab Feature information
				m_pPlugIn->performCommands("getfeaturefromsymbolatcursor A-00000000 c:\\pyxis_data\\pyxilization.tr1\n", m_pPlugInDLL);
				
				//If current players turn
				if(playerID[0]+1==ThePlayer || playerID[1]+1==ThePlayer)
				{
					UNITSELECTION=true;

					PyxDlg->UpdateVisibleWindow();

					//Display Cell 1 Index
					ControlsPage.m_Cell1Index.SetWindowTextA(strIndex.c_str());
					
					ToggleCell=true;
				
				}
				
			}
			else
			{
				//Get info for move
				UnitsPage.FINISHED();
				
				//Only move units if their are some to move
				if(NumOfUnits[0] == 0 && NumOfUnits[1] == 0 && NumOfUnits[2]==0)
				{
					for(int i=0; i<10; i++)
					{
						//Change factory unit creation type
						if(type[i]==2)
						{
							itoa(featID[i],FeatID,10);
							SetType();
						}
					}

					ToggleCell=false;
				}
				else
				{
					//Display Cell 2 Index
					ControlsPage.m_Cell2Index.SetWindowTextA(strIndex.c_str());
					

					//Moves all the units if wanted on current cell
					for(int i=0; i<3; i++)
					{
						if(NumOfUnits[i]!=0)
						{
							for(int j=0; j<10; j++)
							{

									switch(type[j])
									{
									//Move Soldier
									case 0:
										if(i==0)
										{
											itoa(featID[j],FeatID,10);
											itoa(NumOfUnits[0],NumUnits,10);

											PyxDlg->Move(false);
										}
										break;
									//Move Airplane
									case 1:
										if(i==1)
										{
											itoa(featID[j],FeatID,10);
											itoa(NumOfUnits[1],NumUnits,10);

											PyxDlg->Move(true);
										}
										break;
									//If Factory change unit creation type
									case 2:
										itoa(featID[j],FeatID,10);
										SetType();
										break;
									//Move Tank
									case 4:
										if(i==2)
										{
											itoa(featID[j],FeatID,10);
											itoa(NumOfUnits[2],NumUnits,10);

											PyxDlg->Move(false);
										}
										break;

									}	

							}
						}
					}

					ToggleCell=false;
				}
			}
	}
}

BOOL PyxilizationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


   // Create three modeless dialogs and embed them as child windows
   // of PyxilizationDlg. 
	VERIFY(SetupPage.Create(CSetupDlg::IDD, this));
	VERIFY(ControlsPage.Create(CControlDlg::IDD, this));
	VERIFY(StatsPage.Create(CStatsDlg::IDD, this));
	VERIFY(UnitsPage.Create(CUnits::IDD, this));
	
	//Add Tab Items
	m_TabControl.InsertItem(2,"Setup");
	
	//Update the tabs
	UpdateVisibleWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void PyxilizationDlg::UpdateVisibleWindow()
{
   //find out which tab is selected
   int current = PyxDlg->m_TabControl.GetCurSel();
	
   //if user has selected a unit set the dialog to the Unit Selection Dialog
   if(UNITSELECTION == true)
   {
	   current = 2;
   }
   
	
   //Switch between tabs
   if(PyxDlg->SetupPage.m_STARTGAME==false)
   {
	 PyxDlg->SetupPage.ShowWindow(current == 0 ? SW_SHOW : SW_HIDE);
   }
   else
   {

		//Make sure main tab control is reshown once unit selection is complete
	    PyxDlg->m_TabControl.ShowWindow(SW_SHOW);
		//Hide the main setup page
		PyxDlg->SetupPage.ShowWindow(SW_HIDE);
		
		 //Display either control page or stats page or units page depending on tab selected
		PyxDlg-> ControlsPage.ShowWindow(current == 0 ? SW_SHOW : SW_HIDE);
		PyxDlg-> StatsPage.ShowWindow(current == 1 ? SW_SHOW : SW_HIDE);
		PyxDlg-> UnitsPage.ShowWindow(current == 2 ? SW_SHOW : SW_HIDE);
			
		 //If Unit Selection Dialog is showing
		 if(UNITSELECTION == true)
		 {
			//Get rid of tab control
		    PyxDlg->m_TabControl.ShowWindow(SW_HIDE);
			//Call method in Units Dialog to grab information about units from this dialog
			PyxDlg->UnitsPage.GetUnitInformation();
			//Reset Unit Selection Dialog
			UNITSELECTION=false;
		 }
   }
}

void DLLAddCmdResponse(int nInstance, const char* szCmdResponse)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	//local variables
	static int frameCount;
	static bool Tracker;
	static int Type[5];
	static int Player;
	char message[50];
	char pointmessage[50];
	char* PlayerStats;
	static char SendStats[50];
	CString SendStatistics;
	strcpy_s(message, szCmdResponse);
	
	//DEBUG LINE
	//PyxDlg->MessageBox(szCmdResponse);

	strcpy(pointmessage, szCmdResponse);
	pointmessage[20]=0;
	PlayerStats=&pointmessage[0];
	
	//If its the points string
	if(strcmp(PlayerStats, "pyxilization points ")==0)
	{
		//Lets now get the amount of points the player has
		PlayerStats=&message[0];
		//Convert the value
		Points=atoi(PlayerStats+20);
	}

		

	PlayerStats=&message[15];
	//DEBUG LINE
	//PyxDlg->MessageBox(PlayerStats);

	//lets grab some player stats info
	switch(message[13])
	{	
		//player
		case 'p':
			PlayerStats=&message[25];
			Player=atoi(PlayerStats);
			break;
		//soldiers
		case '0':
			strcat(SendStats,PlayerStats);
			break;
		//airplanes
		case '1':
			strcat(SendStats,PlayerStats);
			break;
		//factories
		case '2':
			strcat(SendStats,PlayerStats);
			break;
		//airports
		case '3':
			strcat(SendStats,PlayerStats);
			break;
		//tanks
		case '4':
			strcat(SendStats,PlayerStats);
			
			//Convert the char buff to CString so we can send it over to our Stats Page
			SendStatistics=SendStats;
			//Add all the new stats info to stats page
			PyxDlg->StatsPage.AddStats(Player,SendStatistics);
			//Reset SendStats Buff
			SendStats[0]=0;
			break;
	}
	


	//Checks to see if No Feature was selected
	if (strcmp(szCmdResponse, "getfeaturefromsymbolatcursor <NO DATA FOUND>")== 0)
	{
		counter=0;
	}
	else
	{
		//downsize the string to keep the important values
		strcpy(message, message+31);

		switch(message[0])
		{
			//if FEATURE ID frame
			case ':':
				featID[counter] = atoi(message+33);
				frameCount++;
			break;
			//if TYPE frame
			case 't':
				type[counter] = atoi(message+36);
				frameCount++;
			break;
			//if HEALTH ID frame
			case 'h':
				HealthID[counter] = atoi(message+38);
				frameCount++;
			break;
			//if Build Type frame
			case 'b':
				BuildType[counter] = atoi(message+41);
				frameCount++;
			break;
			//if PLAYER ID frame
			case 'p':
				playerID[counter] = atoi(message+40);
				frameCount++;
			break;
			default:
			break;		
		}

			//when finished, output INT values in CellInfo struct
			if(frameCount>=5)
			{
				frameCount=0;
				
				//TEST CODE
				//-----------------------------------------------------

				/*itoa(featID[counter], featBuff, 10);
				itoa(playerID[counter], playerBuff, 10);
				itoa(type[counter], typeBuff, 10);
				itoa(HealthID[counter], healthBuff, 10);
				itoa(BuildType[counter], healthBuff,10);

				PyxDlg->MessageBoxA("Actual INT Value");
				PyxDlg->MessageBoxA(featBuff);
				PyxDlg->MessageBoxA(typeBuff);
				PyxDlg->MessageBoxA(playerBuff);
				PyxDlg->MessageBoxA(healthBuff);
				PyxDlg->MessageBoxA(healthBuff);*/

				//TEST CODE
				//-----------------------------------------------------

				//Increment Feature counter
				counter++;
			}

	}
}

void PyxilizationDlg::UpdateTab()
{
	//local declarations
	char NumPLAYER[5];
	char Buff[128];

	//Initialize variables
	for(int i=0; i<10; i++)
	{
	   type[i]=-1;
	   playerID[i]=-1;
	   featID[i]=-1;
	   HealthID[i]=-1;
	   BuildType[i]=-1;
	}

	//Get Player Names
	PyxDlg->SetupPage.m_P1Name.GetWindowTextA(Name1,20);
	PyxDlg->SetupPage.m_P2Name.GetWindowTextA(Name2,20);
	PyxDlg->SetupPage.m_P3Name.GetWindowTextA(Name3,20);
	PyxDlg->SetupPage.m_P4Name.GetWindowTextA(Name4,20);
	
	//Set Player 1's name to start
	PyxDlg->ControlsPage.m_PlayersName.SetWindowTextA(Name1);

	//Delete Setup Tab after setups complete
	PyxDlg->m_TabControl.DeleteAllItems();

	//Update Tabs
	PyxDlg->m_TabControl.InsertItem(0,"Controls");
	PyxDlg->m_TabControl.InsertItem(1,"Stats");
	

	//Lets build the startgame string
	itoa(NumPLAYERS, NumPLAYER, 10);
	strcpy(Buff, "pyxilization startgame ");
	strcat(Buff,NumPLAYER);
	strcat(Buff,"\n");

	//Start the game
	PyxDlg->m_pPlugIn->performCommands(Buff);
	PyxDlg->StatsPage.m_Name1.SetWindowTextA(Name1);
	PyxDlg->StatsPage.m_Name2.SetWindowTextA(Name2);
	PyxDlg->StatsPage.m_Name3.SetWindowTextA(Name3);
	PyxDlg->StatsPage.m_Name4.SetWindowTextA(Name4);


	
	//Update visible dialogs
	PyxDlg->UpdateVisibleWindow();

	return;
}
void PyxilizationDlg::OnPaint()
{
	CPaintDC dc(this);
	
	//Sets up Rectangle Object
	CRect Dialog;
	//Gets Dialogs Height and Width
	GetClientRect(&Dialog);

	
	//Colors Dialogs Background Black
	dc.FillSolidRect(&Dialog,RGB(0,0,0));

}

void PyxilizationDlg::EndTurn()
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ThePlayer++;

	//Reset number of players back to 1 if it has reached max amount
	if(ThePlayer>NumPLAYERS)
	{
		ThePlayer=1;
	}
	
	//Print player names
	switch(ThePlayer)
	{
	case 1:PyxDlg->ControlsPage.m_PlayersName.SetWindowTextA(Name1);break;
	case 2:PyxDlg->ControlsPage.m_PlayersName.SetWindowTextA(Name2);break;
	case 3:PyxDlg->ControlsPage.m_PlayersName.SetWindowTextA(Name3);break;
	case 4:PyxDlg->ControlsPage.m_PlayersName.SetWindowTextA(Name4);break;
	default:break;
	}
	
	//Update controls dialog display(Needs to be done otherwise color display won't update)
	PyxDlg->ControlsPage.Invalidate();
	
	//End the turn so next player can go
	PyxDlg->m_pPlugIn->performCommands("pyxilization endturn\n", PyxDlg->m_pPlugInDLL);

	return;
}

//Move Units
void PyxilizationDlg::Move(bool bLoadPlanes)
{
	char buff[1024];
	char buff2[128];
	
	//lets setup the move command string
	strcpy(buff, "pyxilization move ");
	strcat(buff, NumUnits);
	strcat(buff, " ");
	strcat(buff, FeatID);
	strcat(buff, " ");
	ControlsPage.m_Cell2Index.GetWindowTextA(buff2, 128);
	strcat(buff, buff2);
	strcat(buff, "\n");
	
	//DEBUG
	//MessageBox(buff);
	
	//Move the players units
	m_pPlugIn->performCommands(buff);

	return;
}

//Load Units in a plane
void PyxilizationDlg::LoadPlane(int Units)
{
	//Local variables
	char buff[1024];
	char buff2[128];
	char Unit[10];
	
	//Convert the number of Units to a char buff
	itoa(Units,Unit,10);
	
	//If their are units
	if(Units>0)
	{
		//Setup load string command
		strcpy(buff, "pyxilization load ");
		PyxDlg->ControlsPage.m_Cell1Index.GetWindowTextA(buff2, 128);
		strcat(buff,buff2);
		strcat(buff, " ");
		strcat(buff, Unit);
		strcat(buff, "\n");
		
		//DEBUG
		//PyxDlg->MessageBox(buff);
		
		//Load some units
		PyxDlg->m_pPlugIn->performCommands(buff);
	}
	
	return;
}

//Unload units out of a plane
void PyxilizationDlg::UnloadPlane(int Units)
{
	//local variables
	char buff[1024];
	char buff2[128];
	char Unit[10];
	
	//convert number of units and put it in a char buff
	itoa(Units,Unit,10);
	
	//if their are units to unload
	if(Units>0)
	{
		//Setup unload string command
		strcpy(buff, "pyxilization unload ");
		PyxDlg->ControlsPage.m_Cell1Index.GetWindowTextA(buff2, 128);
		strcat(buff,buff2);
		strcat(buff, " ");
		strcat(buff, Unit);
		strcat(buff, "\n");
		
		//DEBUG
		//PyxDlg->MessageBox(buff);
		
		//unload some units
		PyxDlg->m_pPlugIn->performCommands(buff);
	}

	return;
}

//Set unit production type of a factory
void PyxilizationDlg::SetType()
{
	//local variables
	char buff[1024];
	char Type[5];
	
	//setup settype string command
	strcpy(buff, "pyxilization settype ");
	strcat(buff, FeatID);
	strcat(buff, " ");
	itoa(UnitCreationType,Type,10);
	strcat(buff, Type);
	strcat(buff, "\n");
	
	//DEBUG
	//MessageBox(buff);
	
	//set the type of a factory
	m_pPlugIn->performCommands(buff);

	return;
}

HBRUSH PyxilizationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = ::CreateSolidBrush(RGB(0,0,0)); //= __super::OnCtlColor(pDC, pWnd, nCtlColor);
	
	//Makes all static controls have a transparent background
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,255,255));

	return hbr;
}

void PyxilizationDlg::BuyUnits()
{
	char buff[50];
	char FeatureID[5];
	char BType[5];

	//Covert the current players ID to a character so we can
	//use it in our buy command string
	itoa(ThePlayer-1,FeatureID,10);
	
	//Setup buy command string
	strcpy(buff, "pyxilization getpoints ");
	strcat(buff,FeatureID);
	strcat(buff, "\n");
	
	//DEBUG
	//PyxDlg->MessageBox(buff);

	//Lets check to see how many points the player has
	PyxDlg->m_pPlugIn->performCommands(buff, PyxDlg->m_pPlugInDLL); 
	
	//If they have points
	if(Points>0)
	{
		//Search for factory then get its ID...This will be the factory the units created at
		for(int i=0; i<10; i++)
		{
			if(type[i]==2)
			{	
				//Build the buy string command
				strcpy(buff, "pyxilization buy ");
				itoa(featID[i],FeatureID,10);
				strcat(buff, FeatureID);
				strcat(buff," ");
				itoa(BuildType[i], BType,10);
				strcat(buff,BType);
				strcat(buff, "\n");

				//PyxDlg->MessageBox(buff);
				//lets buy some units
				PyxDlg->m_pPlugIn->performCommands(buff);
			}
		}
	}
	else
	{
		PyxDlg->MessageBox("Sorry you have no points :(");
	}

	return;
}
