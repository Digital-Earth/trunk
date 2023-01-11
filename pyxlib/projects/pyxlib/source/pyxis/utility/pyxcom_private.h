#ifndef PYXIS__UTILITY__PYXCOM_PRIVATE_H
#define PYXIS__UTILITY__PYXCOM_PRIVATE_H
/******************************************************************************
pyxcom_private.h

******************************************************************************/

// standard includes
#include <cstring>

typedef unsigned long		PYXCOM_ULONG;
typedef int                 PYXCOM_BOOL;
typedef long                PYXCOM_HRESULT;

#define PYXCOM_SUCCEEDED(hr) ((PYXCOM_HRESULT)(hr) >= 0)
#define PYXCOM_FAILED(hr) ((PYXCOM_HRESULT)(hr) < 0)
#define PYXCOM_IS_ERROR(Status) ((unsigned long)(Status) >> 31 == 0)

#define _PYXCOM_HRESULT_TYPEDEF_(_sc) ((PYXCOM_HRESULT)_sc)

#define PYXCOM_NOERROR             0

#define PYXCOM_UNEXPECTED                     _PYXCOM_HRESULT_TYPEDEF_(0x8000FFFFL)
#define PYXCOM_NOTIMPL                        _PYXCOM_HRESULT_TYPEDEF_(0x80004001L)
#define PYXCOM_OUTOFMEMORY                    _PYXCOM_HRESULT_TYPEDEF_(0x8007000EL)
#define PYXCOM_INVALIDARG                     _PYXCOM_HRESULT_TYPEDEF_(0x80070057L)
#define PYXCOM_NOINTERFACE                    _PYXCOM_HRESULT_TYPEDEF_(0x80004002L)
#define PYXCOM_POINTER                        _PYXCOM_HRESULT_TYPEDEF_(0x80004003L)
#define PYXCOM_HANDLE                         _PYXCOM_HRESULT_TYPEDEF_(0x80070006L)
#define PYXCOM_ABORT                          _PYXCOM_HRESULT_TYPEDEF_(0x80004004L)
#define PYXCOM_FAIL                           _PYXCOM_HRESULT_TYPEDEF_(0x80004005L)
#define PYXCOM_ACCESSDENIED                   _PYXCOM_HRESULT_TYPEDEF_(0x80070005L)

#define PYXCOM_OK                             _PYXCOM_HRESULT_TYPEDEF_(0x00000000L)
#define PYXCOM_FALSE                          _PYXCOM_HRESULT_TYPEDEF_(0x00000001L)

#define PYXCOM_CLASS_E_NOAGGREGATION            _PYXCOM_HRESULT_TYPEDEF_(0x80040110L)
#define PYXCOM_CLASS_E_CLASSNOTAVAILABLE        _PYXCOM_HRESULT_TYPEDEF_(0x80040111L)
#define PYXCOM_CLASS_E_NOTLICENSED              _PYXCOM_HRESULT_TYPEDEF_(0x80040112L)

////////////////////////////////////////////////////////////////////////////////

#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID
{
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
#ifdef SWIG
	PYXLIB_DECL _GUID( const char *strGuid);
#endif
} GUID;

#endif // !GUID_DEFINED

////////////////////////////////////////////////////////////////////////////////

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef GUID IID;
typedef IID *LPIID;
#define IID_NULL            GUID_NULL
#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
typedef GUID CLSID;
typedef CLSID *LPCLSID;
#define CLSID_NULL          GUID_NULL
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)
typedef GUID FMTID;
typedef FMTID *LPFMTID;
#define FMTID_NULL          GUID_NULL
#define IsEqualFMTID(rfmtid1, rfmtid2) IsEqualGUID(rfmtid1, rfmtid2)

#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID const GUID &
#endif

#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#define REFIID const IID &
#endif

#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#define REFCLSID const IID &
#endif

#ifndef _REFFMTID_DEFINED
#define _REFFMTID_DEFINED
#define REFFMTID const IID &
#endif

#endif // !__IID_DEFINED__

////////////////////////////////////////////////////////////////////////////////

#ifndef _SYS_GUID_OPERATORS_
#define _SYS_GUID_OPERATORS_

__inline int InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return (
      ((unsigned long *) &rguid1)[0] == ((unsigned long *) &rguid2)[0] &&
      ((unsigned long *) &rguid1)[1] == ((unsigned long *) &rguid2)[1] &&
      ((unsigned long *) &rguid1)[2] == ((unsigned long *) &rguid2)[2] &&
      ((unsigned long *) &rguid1)[3] == ((unsigned long *) &rguid2)[3]);
}

__inline int IsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return !memcmp(&rguid1, &rguid2, sizeof(GUID));
}

#ifdef __INLINE_ISEQUAL_GUID
#undef IsEqualGUID
#define IsEqualGUID(rguid1, rguid2) InlineIsEqualGUID(rguid1, rguid2)
#endif

#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)

#if !defined _SYS_GUID_OPERATOR_EQ_ && !defined _NO_SYS_GUID_OPERATOR_EQ_
#define _SYS_GUID_OPERATOR_EQ_

__inline int operator==(REFGUID guidOne, REFGUID guidOther)
{
    return IsEqualGUID(guidOne,guidOther);
}

__inline int operator!=(REFGUID guidOne, REFGUID guidOther)
{
    return !(guidOne == guidOther);
}

#endif // !_SYS_GUID_OPERATOR_EQ_

#endif // !_SYS_GUID_OPERATORS_

////////////////////////////////////////////////////////////////////////////////

// Not guarded in cguid.h

extern "C" const GUID GUID_NULL;

////////////////////////////////////////////////////////////////////////////////

// Simplified from system headers

#define STDMETHODCALLTYPE __stdcall

#ifdef INSTANCE_COUNTING
#include "instance_counter.h"
#endif

////////////////////////////////////////////////////////////////////////////////
//! PYXCOM interface for all objects.
struct PYXLIB_DECL PYXCOM_IUnknown
#ifdef INSTANCE_COUNTING
	: private InstanceCounter
#endif
{
public:

	static const IID iid;

public:

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppObject) = 0; 

	template<class Interface>
	boost::intrusive_ptr<Interface> QueryInterface()
	{		
		Interface * object = NULL;

		if (PYXCOM_SUCCEEDED(QueryInterface(Interface::iid,(void**)&object)))
		{		
			//return an interusive pointer. but we don't increase reference counting because QueryInterface is already AddRef for us.
			return boost::intrusive_ptr<Interface>(object,false);
		}
		//return a null pointer
		return boost::intrusive_ptr<Interface>();
	}

	virtual PYXCOM_ULONG STDMETHODCALLTYPE AddRef() = 0;
	virtual PYXCOM_ULONG STDMETHODCALLTYPE Release() = 0;

protected:

	//virtual to allow polymorphic deletion, and protected to prevent stack allocation
	virtual ~PYXCOM_IUnknown() {}
};

////////////////////////////////////////////////////////////////////////////////
//! PYXCOM interface for class factories.
struct PYXLIB_DECL PYXCOM_IClassFactory : public PYXCOM_IUnknown
{
public:

	static const IID iid;

public:

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE CreateInstance(PYXCOM_IUnknown* pUnkOuter, REFIID riid, void** ppObject) = 0;

	// not currently used
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE LockServer(PYXCOM_BOOL bLock) = 0;
};

typedef PYXCOM_IClassFactory* LPPYXCOM_CLASSFACTORY;

#endif // guard
