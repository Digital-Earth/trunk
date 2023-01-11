/******************************************************************************
process_init_error.cpp

begin      : 25/04/2008 11:00:21 AM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/process.h"

// {DA2445A9-1582-4965-B4D5-519AD9FF037C}
PYXCOM_DEFINE_IID(IProcessInitError, 
0xda2445a9, 0x1582, 0x4965, 0xb4, 0xd5, 0x51, 0x9a, 0xd9, 0xff, 0x3, 0x7c);

// {FC5BCF40-26D0-4323-9334-5363E0FBEF2F}
PYXCOM_DEFINE_CLSID(GenericProcInitError, 
0xfc5bcf40, 0x26d0, 0x4323, 0x93, 0x34, 0x53, 0x63, 0xe0, 0xfb, 0xef, 0x2f);
PYXCOM_CLASS_INTERFACES(GenericProcInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);

// {E47B687E-D078-4304-9F0E-A76243A62432}
PYXCOM_DEFINE_CLSID(InputInitError, 
0xe47b687e, 0xd078, 0x4304, 0x9f, 0xe, 0xa7, 0x62, 0x43, 0xa6, 0x24, 0x32);
PYXCOM_CLASS_INTERFACES(InputInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);

// {97EE0643-7092-46cb-AE2A-DB9C6665D70D}
PYXCOM_DEFINE_CLSID(ProcSpecFailure, 
0x97ee0643, 0x7092, 0x46cb, 0xae, 0x2a, 0xdb, 0x9c, 0x66, 0x65, 0xd7, 0xd);
PYXCOM_CLASS_INTERFACES(ProcSpecFailure, IProcessInitError::iid, PYXCOM_IUnknown::iid);

