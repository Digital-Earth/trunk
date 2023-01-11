/******************************************************************************
string.cpp

begin		: 2007-04-13
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/procs/string.h"

// pyxlib includes

// {00ADECB2-E87A-4ef8-9313-972BEB88E58C}
PYXCOM_DEFINE_IID(IString, 
0xadecb2, 0xe87a, 0x4ef8, 0x93, 0x13, 0x97, 0x2b, 0xeb, 0x88, 0xe5, 0x8c);

// {ED389370-5781-46cc-A73D-665E10AE3C47}
PYXCOM_DEFINE_CLSID(StringProc, 
0xed389370, 0x5781, 0x46cc, 0xa7, 0x3d, 0x66, 0x5e, 0x10, 0xae, 0x3c, 0x47);
PYXCOM_CLASS_INTERFACES(StringProc, IString::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(StringProc, "Text", "A Process that provides an unformatted text string", "Drop",
					IString::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END
