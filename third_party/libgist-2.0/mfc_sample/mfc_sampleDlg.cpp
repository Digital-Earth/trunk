// mfc_sampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mfc_sample.h"
#include "mfc_sampleDlg.h"

#include "gist.h"
#include "gist_extensions.h"
#include "gist_rtpred_point.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDlg dialog

CMfc_sampleDlg::CMfc_sampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMfc_sampleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMfc_sampleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMfc_sampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMfc_sampleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMfc_sampleDlg, CDialog)
	//{{AFX_MSG_MAP(CMfc_sampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfc_sampleDlg message handlers

BOOL CMfc_sampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMfc_sampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMfc_sampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMfc_sampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

int CMfc_sampleDlg::query(gist& sr_index)
{
	double c[]={1.0, 5.0};
	rt_point *p=new rt_point(2, c); 
	rt_query_t q(rt_query_t::rt_contains, rt_query_t::rt_pointarg, p); 

	// create a cursor (or iterator) based on the specified query
	gist_cursor_t cursor;

	// Max 1
	int k=1;

	if(sr_index.fetch_init(cursor, &q, k)!=RCOK)
	{
//		cerr << "Can't initialize cursor." << endl;
		return(eERROR);
	}

	// now do the lookup itself
	
	bool eof=false;

	// found coordinates & corresponding labels will be put in these variables
	char label[gist_p::max_tup_sz];
	char coord[gist_p::max_tup_sz];

	// used to specify the size of coordinates & labels
	smsize_t coord_len, label_len;

	while(!eof) 
	{
		coord_len=gist_p::max_tup_sz;
		label_len=gist_p::max_tup_sz;

		// get Nth nearest coordinate & its label until done (eof)
		if(sr_index.fetch(cursor, (void *) coord, coord_len, 
			(void *) label, label_len, eof)!=RCOK)
		{
//			cerr << "Can't fetch from cursor." << endl;
		}
		if(!eof) 
		{
			// process key and data..
			double ll_x, ll_y, ur_x, ur_y;
			int i;

			// cast char data back to coordinates & labels
			ll_x=*((double *) coord);
			ur_x=*(((double *) coord)+1);
			ll_y=*(((double *) coord)+2);
			ur_y=*(((double *) coord)+3);
			i=*((int *) label);

		}
	}


	return 0;
}

void CMfc_sampleDlg::OnButton1() 
{
	gist sr_index;
	
	int nSize = sizeof(sr_index);

	struct _finddata_t file;
	long nFile;
	nFile = _findfirst("rtree-file", &file);
	if (-1L != nFile)
	{
		if (0 != remove(file.name))
		{
//			throw PYXException(kstrCantDeleteFile, file.name);
		}
	}

	// create a new index file, using the given extension 
	// (here an rtree, specified by &rt_rect_ext)
	int nRC = sr_index.create("rtree-file", &rt_rect_ext);

	// two coordinates & matching labels

	// insert above two coordinates & labels in the rtree
	for(int i = 0; i<1; i++)
	{
	
		double rect[]={i, i+5, i, i+5};
		int label=i;
		sr_index.insert((void *) rect, sizeof(rect), (void *) &label, sizeof(label));
	}

	// flush the tree
	sr_index.flush();

	for(int j=0; j<1; j++)
	{
		query(sr_index);
	}


	// That's it!
//	cout << "Done.\n";
	sr_index.close();	

	
}
