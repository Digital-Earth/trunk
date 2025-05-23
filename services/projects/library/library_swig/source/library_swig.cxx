/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.30
 * 
 * This file is not intended to be easily readable and contains a number of 
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG 
 * interface file instead. 
 * ----------------------------------------------------------------------------- */

#define SWIG_DIRECTORS

#ifdef __cplusplus
template<class T> class SwigValueWrapper {
    T *tt;
public:
    SwigValueWrapper() : tt(0) { }
    SwigValueWrapper(const SwigValueWrapper<T>& rhs) : tt(new T(*rhs.tt)) { }
    SwigValueWrapper(const T& t) : tt(new T(t)) { }
    ~SwigValueWrapper() { delete tt; } 
    SwigValueWrapper& operator=(const T& t) { delete tt; tt = new T(t); return *this; }
    operator T&() const { return *tt; }
    T *operator&() { return tt; }
private:
    SwigValueWrapper& operator=(const SwigValueWrapper<T>& rhs);
};
#endif

/* -----------------------------------------------------------------------------
 *  This section contains generic SWIG labels for method/variable
 *  declarations/attributes, and other compiler dependent labels.
 * ----------------------------------------------------------------------------- */

/* template workaround for compilers that cannot correctly implement the C++ standard */
#ifndef SWIGTEMPLATEDISAMBIGUATOR
# if defined(__SUNPRO_CC)
#   if (__SUNPRO_CC <= 0x560)
#     define SWIGTEMPLATEDISAMBIGUATOR template
#   else
#     define SWIGTEMPLATEDISAMBIGUATOR 
#   endif
# else
#   define SWIGTEMPLATEDISAMBIGUATOR 
# endif
#endif

/* inline attribute */
#ifndef SWIGINLINE
# if defined(__cplusplus) || (defined(__GNUC__) && !defined(__STRICT_ANSI__))
#   define SWIGINLINE inline
# else
#   define SWIGINLINE
# endif
#endif

/* attribute recognised by some compilers to avoid 'unused' warnings */
#ifndef SWIGUNUSED
# if defined(__GNUC__)
#   if !(defined(__cplusplus)) || (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
#     define SWIGUNUSED __attribute__ ((__unused__)) 
#   else
#     define SWIGUNUSED
#   endif
# elif defined(__ICC)
#   define SWIGUNUSED __attribute__ ((__unused__)) 
# else
#   define SWIGUNUSED 
# endif
#endif

#ifndef SWIGUNUSEDPARM
# ifdef __cplusplus
#   define SWIGUNUSEDPARM(p)
# else
#   define SWIGUNUSEDPARM(p) p SWIGUNUSED 
# endif
#endif

/* internal SWIG method */
#ifndef SWIGINTERN
# define SWIGINTERN static SWIGUNUSED
#endif

/* internal inline SWIG method */
#ifndef SWIGINTERNINLINE
# define SWIGINTERNINLINE SWIGINTERN SWIGINLINE
#endif

/* exporting methods */
#if (__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#  ifndef GCC_HASCLASSVISIBILITY
#    define GCC_HASCLASSVISIBILITY
#  endif
#endif

#ifndef SWIGEXPORT
# if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   if defined(STATIC_LINKED)
#     define SWIGEXPORT
#   else
#     define SWIGEXPORT __declspec(dllexport)
#   endif
# else
#   if defined(__GNUC__) && defined(GCC_HASCLASSVISIBILITY)
#     define SWIGEXPORT __attribute__ ((visibility("default")))
#   else
#     define SWIGEXPORT
#   endif
# endif
#endif

/* calling conventions for Windows */
#ifndef SWIGSTDCALL
# if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   define SWIGSTDCALL __stdcall
# else
#   define SWIGSTDCALL
# endif 
#endif

/* Deal with Microsoft's attempt at deprecating C standard runtime functions */
#if !defined(SWIG_NO_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
# define _CRT_SECURE_NO_DEPRECATE
#endif


#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/* Support for throwing C# exceptions from C/C++. There are two types: 
 * Exceptions that take a message and ArgumentExceptions that take a message and a parameter name. */
typedef enum {
  SWIG_CSharpApplicationException,
  SWIG_CSharpArithmeticException,
  SWIG_CSharpDivideByZeroException,
  SWIG_CSharpIndexOutOfRangeException,
  SWIG_CSharpInvalidCastException,
  SWIG_CSharpInvalidOperationException,
  SWIG_CSharpIOException,
  SWIG_CSharpNullReferenceException,
  SWIG_CSharpOutOfMemoryException,
  SWIG_CSharpOverflowException,
  SWIG_CSharpSystemException
} SWIG_CSharpExceptionCodes;

typedef enum {
  SWIG_CSharpArgumentException,
  SWIG_CSharpArgumentNullException,
  SWIG_CSharpArgumentOutOfRangeException
} SWIG_CSharpExceptionArgumentCodes;

typedef void (SWIGSTDCALL* SWIG_CSharpExceptionCallback_t)(const char *);
typedef void (SWIGSTDCALL* SWIG_CSharpExceptionArgumentCallback_t)(const char *, const char *);

typedef struct {
  SWIG_CSharpExceptionCodes code;
  SWIG_CSharpExceptionCallback_t callback;
} SWIG_CSharpException_t;

typedef struct {
  SWIG_CSharpExceptionArgumentCodes code;
  SWIG_CSharpExceptionArgumentCallback_t callback;
} SWIG_CSharpExceptionArgument_t;

static SWIG_CSharpException_t SWIG_csharp_exceptions[] = {
  { SWIG_CSharpApplicationException, NULL },
  { SWIG_CSharpArithmeticException, NULL },
  { SWIG_CSharpDivideByZeroException, NULL },
  { SWIG_CSharpIndexOutOfRangeException, NULL },
  { SWIG_CSharpInvalidCastException, NULL },
  { SWIG_CSharpInvalidOperationException, NULL },
  { SWIG_CSharpIOException, NULL },
  { SWIG_CSharpNullReferenceException, NULL },
  { SWIG_CSharpOutOfMemoryException, NULL },
  { SWIG_CSharpOverflowException, NULL },
  { SWIG_CSharpSystemException, NULL }
};

static SWIG_CSharpExceptionArgument_t SWIG_csharp_exceptions_argument[] = {
  { SWIG_CSharpArgumentException, NULL },
  { SWIG_CSharpArgumentNullException, NULL },
  { SWIG_CSharpArgumentOutOfRangeException, NULL }
};

static void SWIGUNUSED SWIG_CSharpSetPendingException(SWIG_CSharpExceptionCodes code, const char *msg) {
  SWIG_CSharpExceptionCallback_t callback = SWIG_csharp_exceptions[SWIG_CSharpApplicationException].callback;
  if (code >=0 && (size_t)code < sizeof(SWIG_csharp_exceptions)/sizeof(SWIG_CSharpException_t)) {
    callback = SWIG_csharp_exceptions[code].callback;
  }
  callback(msg);
}

static void SWIGUNUSED SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpExceptionArgumentCodes code, const char *msg, const char *param_name) {
  SWIG_CSharpExceptionArgumentCallback_t callback = SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentException].callback;
  if (code >=0 && (size_t)code < sizeof(SWIG_csharp_exceptions_argument)/sizeof(SWIG_CSharpExceptionArgument_t)) {
    callback = SWIG_csharp_exceptions_argument[code].callback;
  }
  callback(msg, param_name);
}


#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionCallbacks_library_swig(
                                                SWIG_CSharpExceptionCallback_t applicationCallback,
                                                SWIG_CSharpExceptionCallback_t arithmeticCallback,
                                                SWIG_CSharpExceptionCallback_t divideByZeroCallback, 
                                                SWIG_CSharpExceptionCallback_t indexOutOfRangeCallback, 
                                                SWIG_CSharpExceptionCallback_t invalidCastCallback,
                                                SWIG_CSharpExceptionCallback_t invalidOperationCallback,
                                                SWIG_CSharpExceptionCallback_t ioCallback,
                                                SWIG_CSharpExceptionCallback_t nullReferenceCallback,
                                                SWIG_CSharpExceptionCallback_t outOfMemoryCallback, 
                                                SWIG_CSharpExceptionCallback_t overflowCallback, 
                                                SWIG_CSharpExceptionCallback_t systemCallback) {
  SWIG_csharp_exceptions[SWIG_CSharpApplicationException].callback = applicationCallback;
  SWIG_csharp_exceptions[SWIG_CSharpArithmeticException].callback = arithmeticCallback;
  SWIG_csharp_exceptions[SWIG_CSharpDivideByZeroException].callback = divideByZeroCallback;
  SWIG_csharp_exceptions[SWIG_CSharpIndexOutOfRangeException].callback = indexOutOfRangeCallback;
  SWIG_csharp_exceptions[SWIG_CSharpInvalidCastException].callback = invalidCastCallback;
  SWIG_csharp_exceptions[SWIG_CSharpInvalidOperationException].callback = invalidOperationCallback;
  SWIG_csharp_exceptions[SWIG_CSharpIOException].callback = ioCallback;
  SWIG_csharp_exceptions[SWIG_CSharpNullReferenceException].callback = nullReferenceCallback;
  SWIG_csharp_exceptions[SWIG_CSharpOutOfMemoryException].callback = outOfMemoryCallback;
  SWIG_csharp_exceptions[SWIG_CSharpOverflowException].callback = overflowCallback;
  SWIG_csharp_exceptions[SWIG_CSharpSystemException].callback = systemCallback;
}

#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionArgumentCallbacks_library_swig(
                                                SWIG_CSharpExceptionArgumentCallback_t argumentCallback,
                                                SWIG_CSharpExceptionArgumentCallback_t argumentNullCallback,
                                                SWIG_CSharpExceptionArgumentCallback_t argumentOutOfRangeCallback) {
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentException].callback = argumentCallback;
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentNullException].callback = argumentNullCallback;
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentOutOfRangeException].callback = argumentOutOfRangeCallback;
}


/* Callback for returning char * strings to C# without leaking memory. */
typedef char * (SWIGSTDCALL* SWIG_CSharpStringHelperCallback)(const char *);
static SWIG_CSharpStringHelperCallback SWIG_csharp_string_callback = NULL;

/* Struct for passing std::strings that may contain null characters. */
#ifdef __cplusplus
extern "C" 
#endif
typedef struct {
	size_t length;
	void const * data;
} UnmanagedString;

#include <string>
#include <objbase.h>

static inline UnmanagedString * CreateUnmanagedString(std::string const & from) {
  size_t const length = from.length();
  void * const data = CoTaskMemAlloc(length);
  if (!data) {
    return 0;
  }
  memcpy(data, from.data(), length);
  UnmanagedString * const unmanagedStringPtr = 
    (UnmanagedString * const)CoTaskMemAlloc(sizeof UnmanagedString);
  if (!unmanagedStringPtr) {
    CoTaskMemFree((LPVOID)data);
    return 0;
  }
  unmanagedStringPtr->length = length;
  unmanagedStringPtr->data = data;
  return unmanagedStringPtr;
}

static inline void ConsumeUnmanagedString(UnmanagedString *& unmanagedStringPtr, std::string & to) {
  if (!unmanagedStringPtr) {
    return;
  }
  to.assign((char const * const)(unmanagedStringPtr->data), unmanagedStringPtr->length);
  CoTaskMemFree((LPVOID)(unmanagedStringPtr->data));
  CoTaskMemFree((LPVOID)unmanagedStringPtr);
  unmanagedStringPtr = 0;
}


#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterStringCallback_library_swig(SWIG_CSharpStringHelperCallback callback) {
  SWIG_csharp_string_callback = callback;
}


/* Contract support */

#define SWIG_contract_assert(nullreturn, expr, msg) if (!(expr)) {SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentOutOfRangeException, msg, ""); return nullreturn; } else

/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * director.swg
 *
 * This file contains support for director classes so that C# proxy 
 * methods can be called from C++.
 * ----------------------------------------------------------------------------- */

#ifdef __cplusplus

#if defined(DEBUG_DIRECTOR_OWNED)
#include <iostream>
#endif
#include <string>

namespace Swig {
  /* Director base class - not currently used in C# directors */
  class Director {
  };

  /* Base class for director exceptions */
  class DirectorException {
  protected:
    std::string swig_msg;

  public:
    DirectorException(const char* msg) : swig_msg(msg) {
    }
    DirectorException(const std::string &msg) : swig_msg(msg) {
    }
    const std::string& what() const {
      return swig_msg;
    }
    virtual ~DirectorException() {
    }
  };

  /* Pure virtual method exception */
  class DirectorPureVirtualException : public Swig::DirectorException {
  public:
    DirectorPureVirtualException(const char* msg) : DirectorException(std::string("Attempt to invoke pure virtual method ") + msg) {
    }
  };
}

#endif /* __cplusplus */




// So PYXObject operators are accessible (better way?)
#define SWIG_INTERNAL
#include "../../../../config/windows/force_include.h"

#include "library_process_resolver.h"

// We get 64-bit portability warning C4267
// It's safe for us now, and might even be safe when re-SWIG'd for Win64.
// So I sent a message to swig-devel mailing list and disabled it for now.
// http://msdn2.microsoft.com/en-gb/library/6kck0s93(VS.80).aspx
#pragma warning(disable: 4267)



#include <stdexcept>


#include <vector>
#include <algorithm>
#include <stdexcept>


	#include "library_process_resolver.h"



/* ---------------------------------------------------
 * C++ director class methods
 * --------------------------------------------------- */

#include "library_swig.h"

SwigDirector_LibraryProcessResolver::SwigDirector_LibraryProcessResolver() : LibraryProcessResolver(), Swig::Director() {
  swig_init_callbacks();
}

boost::intrusive_ptr<IProcess > SwigDirector_LibraryProcessResolver::resolve(ProcRef const &procRef) {
  boost::intrusive_ptr<IProcess > c_result ;
  void * jresult  = 0 ;
  void * jprocRef = 0 ;
  
  if (!swig_callbackresolve) {
    return LibraryProcessResolver::resolve(procRef);
  } else {
    jprocRef = (ProcRef *) &procRef; 
    jresult = (void *) swig_callbackresolve(jprocRef);
    if (!jresult) {
      SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Unexpected null return for type boost::intrusive_ptr<IProcess >", 0);
      return c_result;
    }
    c_result = *(boost::intrusive_ptr<IProcess > *)jresult; 
  }
  return c_result;
}

boost::intrusive_ptr<IProcess > SwigDirector_LibraryProcessResolver::notifyResolve(boost::intrusive_ptr<IProcess > spProcess) {
  boost::intrusive_ptr<IProcess > c_result ;
  void * jresult  = 0 ;
  void * jspProcess  ;
  
  if (!swig_callbacknotifyResolve) {
    return LibraryProcessResolver::notifyResolve(spProcess);
  } else {
    jspProcess = (void *)&spProcess; 
    jresult = (void *) swig_callbacknotifyResolve(jspProcess);
    if (!jresult) {
      SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Unexpected null return for type boost::intrusive_ptr<IProcess >", 0);
      return c_result;
    }
    c_result = *(boost::intrusive_ptr<IProcess > *)jresult; 
  }
  return c_result;
}

void SwigDirector_LibraryProcessResolver::notifyFinalize(boost::intrusive_ptr<IProcess > spProc) {
  void * jspProc  ;
  
  if (!swig_callbacknotifyFinalize) {
    ProcessResolver::notifyFinalize(spProc);
    return;
  } else {
    jspProc = (void *)&spProc; 
    swig_callbacknotifyFinalize(jspProc);
  }
}

long SwigDirector_LibraryProcessResolver::release() const {
  long c_result ;
  long jresult  = 0 ;
  
  if (!swig_callbackrelease) {
    return LibraryProcessResolver::release();
  } else {
    jresult = (long) swig_callbackrelease();
    c_result = (long)jresult; 
  }
  return c_result;
}

long SwigDirector_LibraryProcessResolver::addRef() const {
  long c_result ;
  long jresult  = 0 ;
  
  if (!swig_callbackaddRef) {
    return LibraryProcessResolver::addRef();
  } else {
    jresult = (long) swig_callbackaddRef();
    c_result = (long)jresult; 
  }
  return c_result;
}

SwigDirector_LibraryProcessResolver::~SwigDirector_LibraryProcessResolver() {
  
}


void SwigDirector_LibraryProcessResolver::swig_connect_director(SWIG_Callback0_t callbackresolve, SWIG_Callback1_t callbacknotifyResolve, SWIG_Callback2_t callbacknotifyFinalize, SWIG_Callback3_t callbackrelease, SWIG_Callback4_t callbackaddRef) {
  swig_callbackresolve = callbackresolve;
  swig_callbacknotifyResolve = callbacknotifyResolve;
  swig_callbacknotifyFinalize = callbacknotifyFinalize;
  swig_callbackrelease = callbackrelease;
  swig_callbackaddRef = callbackaddRef;
}

void SwigDirector_LibraryProcessResolver::swig_init_callbacks() {
  swig_callbackresolve = 0;
  swig_callbacknotifyResolve = 0;
  swig_callbacknotifyFinalize = 0;
  swig_callbackrelease = 0;
  swig_callbackaddRef = 0;
}


#ifdef __cplusplus
extern "C" {
#endif

SWIGEXPORT void * SWIGSTDCALL CSharp_new_LibraryProcessResolver_SPtr__SWIG_0() {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *result = 0 ;
  
  result = (boost::intrusive_ptr<LibraryProcessResolver > *)new boost::intrusive_ptr<LibraryProcessResolver >();
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_new_LibraryProcessResolver_SPtr__SWIG_1(void * jarg1, unsigned int jarg2) {
  void * jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  bool arg2 ;
  boost::intrusive_ptr<LibraryProcessResolver > *result = 0 ;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  arg2 = jarg2 ? true : false; 
  result = (boost::intrusive_ptr<LibraryProcessResolver > *)new boost::intrusive_ptr<LibraryProcessResolver >(arg1,arg2);
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_new_LibraryProcessResolver_SPtr__SWIG_2(void * jarg1) {
  void * jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  boost::intrusive_ptr<LibraryProcessResolver > *result = 0 ;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  result = (boost::intrusive_ptr<LibraryProcessResolver > *)new boost::intrusive_ptr<LibraryProcessResolver >(arg1);
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_new_LibraryProcessResolver_SPtr__SWIG_3(void * jarg1) {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = 0 ;
  boost::intrusive_ptr<LibraryProcessResolver > *result = 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1;
  if(!arg1) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "boost::intrusive_ptr<LibraryProcessResolver > const & type is null", 0);
    return 0;
  } 
  result = (boost::intrusive_ptr<LibraryProcessResolver > *)new boost::intrusive_ptr<LibraryProcessResolver >((boost::intrusive_ptr<LibraryProcessResolver > const &)*arg1);
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT void SWIGSTDCALL CSharp_delete_LibraryProcessResolver_SPtr(void * jarg1) {
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  delete arg1;
  
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_reset__SWIG_0(void * jarg1) {
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  (arg1)->reset();
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_reset__SWIG_1(void * jarg1, void * jarg2) {
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  LibraryProcessResolver *arg2 = (LibraryProcessResolver *) 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  arg2 = (LibraryProcessResolver *)jarg2; 
  (arg1)->reset(arg2);
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_get(void * jarg1) {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  LibraryProcessResolver *result = 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  result = (LibraryProcessResolver *)((boost::intrusive_ptr<LibraryProcessResolver > const *)arg1)->get();
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr___ref__(void * jarg1) {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  LibraryProcessResolver *result = 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  {
    LibraryProcessResolver &_result_ref = ((boost::intrusive_ptr<LibraryProcessResolver > const *)arg1)->operator *();
    result = (LibraryProcessResolver *) &_result_ref;
  }
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr___deref__(void * jarg1) {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  LibraryProcessResolver *result = 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  result = (LibraryProcessResolver *)((boost::intrusive_ptr<LibraryProcessResolver > const *)arg1)->operator ->();
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_swap(void * jarg1, void * jarg2) {
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg2 = 0 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  arg2 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg2;
  if(!arg2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "boost::intrusive_ptr<LibraryProcessResolver > & type is null", 0);
    return ;
  } 
  (arg1)->swap(*arg2);
}


SWIGEXPORT long SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_release(void * jarg1) {
  long jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  long result;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  
  try
  {
    result = (long)(*arg1)->release();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = result; 
  return jresult;
}


SWIGEXPORT long SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_addRef(void * jarg1) {
  long jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  long result;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  
  try
  {
    result = (long)(*arg1)->addRef();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = result; 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_create(void * jarg1) {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  boost::intrusive_ptr<LibraryProcessResolver > result;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  
  try
  {
    result = (*arg1)->create();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<LibraryProcessResolver >((boost::intrusive_ptr<LibraryProcessResolver > &)result); 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_resolve(void * jarg1, void * jarg2) {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  ProcRef *arg2 = 0 ;
  boost::intrusive_ptr<IProcess > result;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  arg2 = (ProcRef *)jarg2;
  if(!arg2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "ProcRef const & type is null", 0);
    return 0;
  } 
  
  try
  {
    result = (*arg1)->resolve((ProcRef const &)*arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<IProcess >((boost::intrusive_ptr<IProcess > &)result); 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_notifyResolve(void * jarg1, void * jarg2) {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  boost::intrusive_ptr<IProcess > arg2 ;
  boost::intrusive_ptr<IProcess > result;
  boost::intrusive_ptr<IProcess > *argp2 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  argp2 = (boost::intrusive_ptr<IProcess > *)jarg2; 
  if (!argp2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Attempt to dereference null boost::intrusive_ptr<IProcess >", 0);
    return 0;
  }
  arg2 = *argp2; 
  
  try
  {
    result = (*arg1)->notifyResolve(arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<IProcess >((boost::intrusive_ptr<IProcess > &)result); 
  return jresult;
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_set(void * jarg1, void * jarg2) {
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  boost::intrusive_ptr<LibraryProcessResolver > arg2 ;
  boost::intrusive_ptr<LibraryProcessResolver > *argp2 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  argp2 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg2; 
  if (!argp2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Attempt to dereference null boost::intrusive_ptr<LibraryProcessResolver >", 0);
    return ;
  }
  arg2 = *argp2; 
  
  try
  {
    (*arg1)->set(arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_SPtr_notifyFinalize(void * jarg1, void * jarg2) {
  boost::intrusive_ptr<LibraryProcessResolver > *arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *) 0 ;
  boost::intrusive_ptr<IProcess > arg2 ;
  boost::intrusive_ptr<IProcess > *argp2 ;
  
  arg1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  argp2 = (boost::intrusive_ptr<IProcess > *)jarg2; 
  if (!argp2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Attempt to dereference null boost::intrusive_ptr<IProcess >", 0);
    return ;
  }
  arg2 = *argp2; 
  
  try
  {
    (*arg1)->notifyFinalize(arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
}


SWIGEXPORT void * SWIGSTDCALL CSharp_new_LibraryProcessResolver() {
  void * jresult ;
  LibraryProcessResolver *result = 0 ;
  
  
  try
  {
    result = (LibraryProcessResolver *)new SwigDirector_LibraryProcessResolver();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = (void *)result; 
  return jresult;
}


SWIGEXPORT long SWIGSTDCALL CSharp_LibraryProcessResolver_release(void * jarg1) {
  long jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  long result;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  
  try
  {
    result = (long)((LibraryProcessResolver const *)arg1)->release();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = result; 
  return jresult;
}


SWIGEXPORT long SWIGSTDCALL CSharp_LibraryProcessResolver_releaseSwigExplicitLibraryProcessResolver(void * jarg1) {
  long jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  long result;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  
  try
  {
    result = (long)((LibraryProcessResolver const *)arg1)->LibraryProcessResolver::release();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = result; 
  return jresult;
}


SWIGEXPORT long SWIGSTDCALL CSharp_LibraryProcessResolver_addRef(void * jarg1) {
  long jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  long result;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  
  try
  {
    result = (long)((LibraryProcessResolver const *)arg1)->addRef();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = result; 
  return jresult;
}


SWIGEXPORT long SWIGSTDCALL CSharp_LibraryProcessResolver_addRefSwigExplicitLibraryProcessResolver(void * jarg1) {
  long jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  long result;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  
  try
  {
    result = (long)((LibraryProcessResolver const *)arg1)->LibraryProcessResolver::addRef();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = result; 
  return jresult;
}


SWIGEXPORT void SWIGSTDCALL CSharp_delete_LibraryProcessResolver(void * jarg1) {
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  
  try
  {
    delete arg1;
    
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_create() {
  void * jresult ;
  boost::intrusive_ptr<LibraryProcessResolver > result;
  
  
  try
  {
    result = LibraryProcessResolver::create();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<LibraryProcessResolver >((boost::intrusive_ptr<LibraryProcessResolver > &)result); 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_resolve(void * jarg1, void * jarg2) {
  void * jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  ProcRef *arg2 = 0 ;
  boost::intrusive_ptr<IProcess > result;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  arg2 = (ProcRef *)jarg2;
  if(!arg2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "ProcRef const & type is null", 0);
    return 0;
  } 
  
  try
  {
    result = (arg1)->resolve((ProcRef const &)*arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<IProcess >((boost::intrusive_ptr<IProcess > &)result); 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_resolveSwigExplicitLibraryProcessResolver(void * jarg1, void * jarg2) {
  void * jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  ProcRef *arg2 = 0 ;
  boost::intrusive_ptr<IProcess > result;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  arg2 = (ProcRef *)jarg2;
  if(!arg2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "ProcRef const & type is null", 0);
    return 0;
  } 
  
  try
  {
    result = (arg1)->LibraryProcessResolver::resolve((ProcRef const &)*arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<IProcess >((boost::intrusive_ptr<IProcess > &)result); 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_notifyResolve(void * jarg1, void * jarg2) {
  void * jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  boost::intrusive_ptr<IProcess > arg2 ;
  boost::intrusive_ptr<IProcess > result;
  boost::intrusive_ptr<IProcess > *argp2 ;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  argp2 = (boost::intrusive_ptr<IProcess > *)jarg2; 
  if (!argp2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Attempt to dereference null boost::intrusive_ptr<IProcess >", 0);
    return 0;
  }
  arg2 = *argp2; 
  
  try
  {
    result = (arg1)->notifyResolve(arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<IProcess >((boost::intrusive_ptr<IProcess > &)result); 
  return jresult;
}


SWIGEXPORT void * SWIGSTDCALL CSharp_LibraryProcessResolver_notifyResolveSwigExplicitLibraryProcessResolver(void * jarg1, void * jarg2) {
  void * jresult ;
  LibraryProcessResolver *arg1 = (LibraryProcessResolver *) 0 ;
  boost::intrusive_ptr<IProcess > arg2 ;
  boost::intrusive_ptr<IProcess > result;
  boost::intrusive_ptr<IProcess > *argp2 ;
  
  arg1 = (LibraryProcessResolver *)jarg1; 
  argp2 = (boost::intrusive_ptr<IProcess > *)jarg2; 
  if (!argp2) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Attempt to dereference null boost::intrusive_ptr<IProcess >", 0);
    return 0;
  }
  arg2 = *argp2; 
  
  try
  {
    result = (arg1)->LibraryProcessResolver::notifyResolve(arg2);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
  jresult = new boost::intrusive_ptr<IProcess >((boost::intrusive_ptr<IProcess > &)result); 
  return jresult;
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_set(void * jarg1) {
  boost::intrusive_ptr<LibraryProcessResolver > arg1 ;
  boost::intrusive_ptr<LibraryProcessResolver > *argp1 ;
  
  argp1 = (boost::intrusive_ptr<LibraryProcessResolver > *)jarg1; 
  if (!argp1) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "Attempt to dereference null boost::intrusive_ptr<LibraryProcessResolver >", 0);
    return ;
  }
  arg1 = *argp1; 
  
  try
  {
    LibraryProcessResolver::set(arg1);
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_reset() {
  try
  {
    LibraryProcessResolver::reset();
  }
  catch (PYXException& e)
  {
    SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
  }
  
}


SWIGEXPORT void SWIGSTDCALL CSharp_LibraryProcessResolver_director_connect(void *objarg, SwigDirector_LibraryProcessResolver::SWIG_Callback0_t callback0, SwigDirector_LibraryProcessResolver::SWIG_Callback1_t callback1, SwigDirector_LibraryProcessResolver::SWIG_Callback2_t callback2, SwigDirector_LibraryProcessResolver::SWIG_Callback3_t callback3, SwigDirector_LibraryProcessResolver::SWIG_Callback4_t callback4) {
  LibraryProcessResolver *obj = (LibraryProcessResolver *)objarg;
  SwigDirector_LibraryProcessResolver *director = dynamic_cast<SwigDirector_LibraryProcessResolver *>(obj);
  if (director) {
    director->swig_connect_director(callback0, callback1, callback2, callback3, callback4);
  }
}


SWIGEXPORT ProcessResolver * SWIGSTDCALL CSharp_LibraryProcessResolverUpcast(LibraryProcessResolver *objectRef) {
    return (ProcessResolver *)objectRef;
}

#ifdef __cplusplus
}
#endif

