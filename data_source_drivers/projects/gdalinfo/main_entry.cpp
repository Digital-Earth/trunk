/******************************************************************************
main_entry.cpp

begin		: 2009-07-29
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition
// windows includes
#include <windows.h>
#pragma warning(pop)

//---------------------------------------------------------------------
//--
//-- standard DLL entry point.
//--
//---------------------------------------------------------------------

BOOL APIENTRY DllMain( HMODULE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
