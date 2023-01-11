/******************************************************************************
excel.h

begin      : 2009-11-27
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes 
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "excel.h"

Excel::IWorkbookView::~IWorkbookView()
{
}

Excel::IWorkbookTable::~IWorkbookTable()
{
}

Excel::IWorkbook::~IWorkbook()
{
}

Excel::IExcel const * Excel::IExcel::Implementation(Excel::IExcel const * pImplementation)
{
	static Excel::IExcel const * s_pImplementation = 0;

	if (pImplementation != 0)
	{
		s_pImplementation = pImplementation;
	}
	return s_pImplementation;
}

Excel::IExcel::~IExcel()
{
}
