#ifndef WMSCLIENT_UTILITIES_H
#define WMSCLIENT_UTILITIES_H

#define _SECURE_SCL 0
#include <string>

struct ci_char_traits : public std::char_traits<char>
            // just inherit all the other functions
            //  that we don't need to override
{
	static bool eq( char c1, char c2 ) { return tolower(c1) == tolower(c2); }

	static bool ne( char c1, char c2 ) { return tolower(c1) != tolower(c2); }

	static bool lt( char c1, char c2 ) { return tolower(c1) < tolower(c2);  }

	static int compare( const char* s1, const char* s2, size_t n ) {  return _strnicmp( s1, s2, int(n )); }
};

typedef std::basic_string<char, ci_char_traits> ci_string;


class pyxMutex
{
public:

	pyxMutex()
		: m_hMutex(INVALID_HANDLE_VALUE)
	{
		m_hMutex = CreateMutex( NULL, FALSE, NULL);
	};

	~pyxMutex()
	{
		if (m_hMutex != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hMutex);
		}
	}

	HANDLE m_hMutex;
};

class pyxSemaphore
{
public:

	pyxSemaphore()
		: m_hSemaphore(INVALID_HANDLE_VALUE)
	{
		m_hSemaphore = CreateSemaphore( 0, 0, 9999, 0);
	};

	~pyxSemaphore()
	{
		if (m_hSemaphore != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hSemaphore);
		}
	}

	void wait()
	{
		WaitForSingleObject(m_hSemaphore, INFINITE);
	}

	void signal()
	{
		LONG nPrevCount;
		ReleaseSemaphore(m_hSemaphore, 1, &nPrevCount);
	}

	HANDLE m_hSemaphore;
};

class pyxMutexLock
{
public:

	pyxMutexLock(pyxMutex& theMutex)
		: m_theMutex(theMutex)
	{
		WaitForSingleObject(m_theMutex.m_hMutex, INFINITE);
	}

	~pyxMutexLock()
	{
		ReleaseMutex(m_theMutex.m_hMutex);
	}

	pyxMutex& m_theMutex;
};
class utilities
{
public:

	static void createDirectory(std::string& strDirectory);
	static bool isFile(std::string& strPath);
	static std::string toString(double fValue);
	static std::string toString(int nValue);
	static std::string toString(size_t nValue);
	static void replaceAll(std::string& strSource, char szToFind, char szToReplace);

	utilities(void);

	~utilities(void);
};

#endif