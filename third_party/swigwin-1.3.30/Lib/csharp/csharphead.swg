/* -----------------------------------------------------------------------------
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * csharphead.swg
 *
 * Support code for exceptions if the SWIG_CSHARP_NO_EXCEPTION_HELPER is not defined
 * Support code for strings if the SWIG_CSHARP_NO_STRING_HELPER is not defined
 * ----------------------------------------------------------------------------- */

%insert(runtime) %{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
%}

#if !defined(SWIG_CSHARP_NO_EXCEPTION_HELPER)
%insert(runtime) %{
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
%}

%insert(runtime) %{
#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionCallbacks_$module(
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
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionArgumentCallbacks_$module(
                                                SWIG_CSharpExceptionArgumentCallback_t argumentCallback,
                                                SWIG_CSharpExceptionArgumentCallback_t argumentNullCallback,
                                                SWIG_CSharpExceptionArgumentCallback_t argumentOutOfRangeCallback) {
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentException].callback = argumentCallback;
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentNullException].callback = argumentNullCallback;
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentOutOfRangeException].callback = argumentOutOfRangeCallback;
}
%}

%pragma(csharp) imclasscode=%{
  protected class SWIGExceptionHelper {

    public delegate void ExceptionDelegate(string message);
    public delegate void ExceptionArgumentDelegate(string message, string paramName);

    static ExceptionDelegate applicationDelegate = new ExceptionDelegate(SetPendingApplicationException);
    static ExceptionDelegate arithmeticDelegate = new ExceptionDelegate(SetPendingArithmeticException);
    static ExceptionDelegate divideByZeroDelegate = new ExceptionDelegate(SetPendingDivideByZeroException);
    static ExceptionDelegate indexOutOfRangeDelegate = new ExceptionDelegate(SetPendingIndexOutOfRangeException);
    static ExceptionDelegate invalidCastDelegate = new ExceptionDelegate(SetPendingInvalidCastException);
    static ExceptionDelegate invalidOperationDelegate = new ExceptionDelegate(SetPendingInvalidOperationException);
    static ExceptionDelegate ioDelegate = new ExceptionDelegate(SetPendingIOException);
    static ExceptionDelegate nullReferenceDelegate = new ExceptionDelegate(SetPendingNullReferenceException);
    static ExceptionDelegate outOfMemoryDelegate = new ExceptionDelegate(SetPendingOutOfMemoryException);
    static ExceptionDelegate overflowDelegate = new ExceptionDelegate(SetPendingOverflowException);
    static ExceptionDelegate systemDelegate = new ExceptionDelegate(SetPendingSystemException);

    static ExceptionArgumentDelegate argumentDelegate = new ExceptionArgumentDelegate(SetPendingArgumentException);
    static ExceptionArgumentDelegate argumentNullDelegate = new ExceptionArgumentDelegate(SetPendingArgumentNullException);
    static ExceptionArgumentDelegate argumentOutOfRangeDelegate = new ExceptionArgumentDelegate(SetPendingArgumentOutOfRangeException);

    [DllImport("$dllimport", EntryPoint="SWIGRegisterExceptionCallbacks_$module")]
    public static extern void SWIGRegisterExceptionCallbacks_$module(
                                ExceptionDelegate applicationDelegate,
                                ExceptionDelegate arithmeticDelegate,
                                ExceptionDelegate divideByZeroDelegate, 
                                ExceptionDelegate indexOutOfRangeDelegate, 
                                ExceptionDelegate invalidCastDelegate,
                                ExceptionDelegate invalidOperationDelegate,
                                ExceptionDelegate ioDelegate,
                                ExceptionDelegate nullReferenceDelegate,
                                ExceptionDelegate outOfMemoryDelegate, 
                                ExceptionDelegate overflowDelegate, 
                                ExceptionDelegate systemExceptionDelegate);

    [DllImport("$dllimport", EntryPoint="SWIGRegisterExceptionArgumentCallbacks_$module")]
    public static extern void SWIGRegisterExceptionCallbacksArgument_$module(
                                ExceptionArgumentDelegate argumentDelegate,
                                ExceptionArgumentDelegate argumentNullDelegate,
                                ExceptionArgumentDelegate argumentOutOfRangeDelegate);

    static void SetPendingApplicationException(string message) {
      SWIGPendingException.Set(new System.ApplicationException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingArithmeticException(string message) {
      SWIGPendingException.Set(new System.ArithmeticException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingDivideByZeroException(string message) {
      SWIGPendingException.Set(new System.DivideByZeroException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingIndexOutOfRangeException(string message) {
      SWIGPendingException.Set(new System.IndexOutOfRangeException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingInvalidCastException(string message) {
      SWIGPendingException.Set(new System.InvalidCastException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingInvalidOperationException(string message) {
      SWIGPendingException.Set(new System.InvalidOperationException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingIOException(string message) {
      SWIGPendingException.Set(new System.IO.IOException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingNullReferenceException(string message) {
      SWIGPendingException.Set(new System.NullReferenceException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingOutOfMemoryException(string message) {
      SWIGPendingException.Set(new System.OutOfMemoryException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingOverflowException(string message) {
      SWIGPendingException.Set(new System.OverflowException(message, SWIGPendingException.Retrieve()));
    }
    static void SetPendingSystemException(string message) {
      SWIGPendingException.Set(new System.SystemException(message, SWIGPendingException.Retrieve()));
    }

    static void SetPendingArgumentException(string message, string paramName) {
      SWIGPendingException.Set(new System.ArgumentException(message, paramName, SWIGPendingException.Retrieve()));
    }
    static void SetPendingArgumentNullException(string message, string paramName) {
      Exception e = SWIGPendingException.Retrieve();
      if (e != null) message = message + " Inner Exception: " + e.Message;
      SWIGPendingException.Set(new System.ArgumentNullException(paramName, message));
    }
    static void SetPendingArgumentOutOfRangeException(string message, string paramName) {
      Exception e = SWIGPendingException.Retrieve();
      if (e != null) message = message + " Inner Exception: " + e.Message;
      SWIGPendingException.Set(new System.ArgumentOutOfRangeException(paramName, message));
    }

    static SWIGExceptionHelper() {
      SWIGRegisterExceptionCallbacks_$module(
                                applicationDelegate,
                                arithmeticDelegate,
                                divideByZeroDelegate,
                                indexOutOfRangeDelegate,
                                invalidCastDelegate,
                                invalidOperationDelegate,
                                ioDelegate,
                                nullReferenceDelegate,
                                outOfMemoryDelegate,
                                overflowDelegate,
                                systemDelegate);

      SWIGRegisterExceptionCallbacksArgument_$module(
                                argumentDelegate,
                                argumentNullDelegate,
                                argumentOutOfRangeDelegate);
    }
  }

  protected static SWIGExceptionHelper swigExceptionHelper = new SWIGExceptionHelper();

  public class SWIGPendingException {
    [ThreadStatic]
    private static Exception pendingException = null;
    private static int numExceptionsPending = 0;

    public static bool Pending {
      get {
        bool pending = false;
        if (numExceptionsPending > 0)
          if (pendingException != null)
            pending = true;
        return pending;
      } 
    }

    public static void Set(Exception e) {
      if (pendingException != null)
        throw new ApplicationException("FATAL: An earlier pending exception from unmanaged code was missed and thus not thrown (" + pendingException.ToString() + ")", e);
      pendingException = e;
      lock(typeof($imclassname)) {
        numExceptionsPending++;
      }
    }

    public static Exception Retrieve() {
      Exception e = null;
      if (numExceptionsPending > 0) {
        if (pendingException != null) {
          e = pendingException;
          pendingException = null;
          lock(typeof($imclassname)) {
            numExceptionsPending--;
          }
        }
      }
      return e;
    }
  }
%}
#endif // SWIG_CSHARP_NO_EXCEPTION_HELPER

#if !defined(SWIG_CSHARP_NO_STRING_HELPER)
%insert(runtime) %{
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
%}

%pragma(csharp) imclasscode=%{
  /// <summary>
  /// This struct wraps std::strings allocated in unmanaged space.
  /// This is required because std::string can contain null characters.
  /// </summary>
  [StructLayout(LayoutKind.Sequential)]
  public struct UnmanagedString {
    /// <summary>
    /// The number of bytes in the unmanaged string.
    /// </summary>
    private uint length;

    /// <summary>
    /// The byte array, allocated in unmanaged space.
    /// </summary>
    private IntPtr data;

    private static System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding();

    /// <summary>
    /// Copies the string into a new unmanaged string.
    /// Can throw OutOfMemoryException, ArgumentNullException,
    /// or ArgumentOutOfRangeException (if the string is too long).
    /// </summary>
    static public IntPtr Create(String from) {
	  if (from == null) {
	    throw new ArgumentNullException(from);
	  }

      // Ctreate Utf8 string
      byte[] bytes = utf8.GetBytes(from);

      // Create a struct to contain the unmanaged string.
      UnmanagedString unmanagedString;

      // Set the length in the length field.
      unmanagedString.length = (uint)bytes.Length;

      int size = System.Runtime.InteropServices.Marshal.SizeOf(typeof(byte)) * bytes.Length;

      unmanagedString.data = System.Runtime.InteropServices.Marshal.AllocCoTaskMem(size);

      try {
        System.Runtime.InteropServices.Marshal.Copy(bytes, 0, unmanagedString.data , bytes.Length);
      } catch (Exception) {
        // This doesn't throw.
        System.Runtime.InteropServices.Marshal.FreeCoTaskMem(unmanagedString.data);
        throw;
      }

      try {
        // Allocate unmanaged space for the struct.
        // Can throw OutOfMemoryException.
        IntPtr unmanagedStringPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf(unmanagedString));

        // Marshal the struct data into the unmanaged space allocated.
        // Can throw ArgumentException if misused, but it is not here.
        Marshal.StructureToPtr(unmanagedString, unmanagedStringPtr, false);

        // Return the pointer to the unmanaged struct.
        return unmanagedStringPtr;
      } catch (Exception) {
        // This doesn't throw.
        Marshal.FreeCoTaskMem(unmanagedString.data);
        throw;
      }
    }

    /// <summary>
    /// Copies the unmanaged string data into a new String and deallocates the
    /// unmanaged string.
    /// Can throw ArgumentNullException, or ArgumentException if IntPtr is bad.
    /// May return a null String if the struct pointed to by IntPtr had a null data pointer,
    /// which it shouldn't under proper usage.
    /// </summary>
    static public String Consume(IntPtr unmanagedStringPtr) {
      // Marshal the struct from the unmanaged space.
      // Can throw ArgumentNullException.
      // Can throw ArgumentException if IntPtr is bad.
      // Never returns null.
      UnmanagedString unmanagedString = 
        (UnmanagedString)Marshal.PtrToStructure(unmanagedStringPtr, typeof(UnmanagedString));

      // Allocate a managed string and copies characters into it.
      // Returns null if unmanagedString.data is null.
      // Can throw ArgumentException if the integer cast overflows.	
      
      byte[] bytes = new byte[(int)unmanagedString.length];

      System.Runtime.InteropServices.Marshal.Copy(unmanagedString.data, bytes, 0, bytes.Length);

      string to = utf8.GetString(bytes);

      // Free the unmanaged string data.
      // This doesn't throw, and is a no-op for null.
      Marshal.FreeCoTaskMem(unmanagedString.data);

      // Free the unmanaged struct.
      // This doesn't throw.
      Marshal.FreeCoTaskMem(unmanagedStringPtr);

      // Return the managed string.
      return to;
    }
  }

  /// <summary>
  /// This class is used for marshalling null-terminated char * strings
  /// allocated in unmanaged space.
  /// </summary>
  protected class SWIGStringHelper {
    public delegate string SWIGStringDelegate(string message);
    static SWIGStringDelegate stringDelegate = new SWIGStringDelegate(CreateString);

    [DllImport("$dllimport", EntryPoint="SWIGRegisterStringCallback_$module")]
    public static extern void SWIGRegisterStringCallback_$module(SWIGStringDelegate stringDelegate);

    static string CreateString(string cString) {
      return cString;
    }

    static SWIGStringHelper() {
      SWIGRegisterStringCallback_$module(stringDelegate);
    }
  }

  static protected SWIGStringHelper swigStringHelper = new SWIGStringHelper();
%}

%insert(runtime) %{
#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterStringCallback_$module(SWIG_CSharpStringHelperCallback callback) {
  SWIG_csharp_string_callback = callback;
}
%}
#endif // SWIG_CSHARP_NO_STRING_HELPER

#if !defined(SWIG_CSHARP_NO_IMCLASS_STATIC_CONSTRUCTOR)
// Ensure the class is not marked beforefieldinit
%pragma(csharp) imclasscode=%{
  static $imclassname() {
  }
%}
#endif

%insert(runtime) %{
/* Contract support */

#define SWIG_contract_assert(nullreturn, expr, msg) if (!(expr)) {SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentOutOfRangeException, msg, ""); return nullreturn; } else
%}
