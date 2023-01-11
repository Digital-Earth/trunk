#include "stdafx.h"
#include "utilities.h"
#include "pyxnamedpipe.h"

#include <io.h>
#include <fstream>

void addMessagePipe(const char* pWhere, const char* pWhat, const char* pOther = 0)
{
	//! Mutex to serialize concurrent access by multiple threads
	static pyxMutex theMutex;
	
	pyxMutexLock theLock(theMutex);

	std::ofstream ofs;
	ofs.open("c:\\PYXNamedPipe.txt", std::ios::app);

	if (pOther == 0)
	{
		ofs << GetTickCount() << " " << pWhere << " " << pWhat << std::endl;
	}
	else
	{
		ofs << GetTickCount() << " " << pWhere << " " << pWhat << " " << pOther << std::endl; 
	}

	ofs.close(); 

}

PYXNamedPipe::PYXNamedPipe(const std::string &strPipeName)
:	m_hConnectEvent(INVALID_HANDLE_VALUE),
	m_hPipeEvent(INVALID_HANDLE_VALUE),
	m_hTerminateEvent(INVALID_HANDLE_VALUE),
	m_hPipe(INVALID_HANDLE_VALUE),
	m_strPipeName(strPipeName),
	m_nLastError( 0 )
{
	m_nPipeBufferLength = 5000;
	m_pipeBuffer = new char[m_nPipeBufferLength];
}

PYXNamedPipe::PYXNamedPipe()
:	m_hConnectEvent(INVALID_HANDLE_VALUE),
	m_hPipeEvent(INVALID_HANDLE_VALUE),
	m_hTerminateEvent(INVALID_HANDLE_VALUE),
	m_hPipe(INVALID_HANDLE_VALUE),
	m_nLastError( 0 )
{
	m_nPipeBufferLength = 5000;
	m_pipeBuffer = new char[m_nPipeBufferLength];
}

PYXNamedPipe::~PYXNamedPipe()
{
	// Call here just in case the user forgets to call it first.
	terminate();

	// Close handle if we are terminating.
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}

	delete[] m_pipeBuffer;
}

bool PYXNamedPipe::terminate()
{
	SetEvent(m_hTerminateEvent);

	return true;
}

bool PYXNamedPipe::sendResponse(const std::string& strResponse)
{
	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD nBytesWritten;

	BOOL bResult = WriteFile(m_hPipe, strResponse.c_str(), DWORD(strResponse.length()), &nBytesWritten, &m_overLappedWriteObject);

	if (!bResult)
	{
	   m_nLastError = GetLastError();

	   if (m_nLastError != ERROR_IO_PENDING)
	   {
			DisconnectNamedPipe(m_hPipe);
			return false;
	   }
	   else
	   {
		   bResult = TRUE;
	   }
	}

	int nResult = WaitForMultipleObjects(2, m_waitWriteHandles, false, INFINITE);

	DisconnectNamedPipe(m_hPipe);

	// Should we terminate?
	if (nResult == WAIT_OBJECT_0 + 1)
	{
		m_nLastError = -1;

		return false;
	}

	// Has read completed with an error?
	if (bResult == false && nResult == WAIT_OBJECT_0)
	{
	   return false;
	}

	if (!bResult)
	{
		m_nLastError = GetLastError();
		return false;
	}

	return true;
}

bool PYXNamedPipe::createServer()
{
	m_hTerminateEvent = CreateEvent(0, false, 0, 0);

	m_hConnectEvent = CreateEvent(0, false, 0, 0);

	memset(&m_overLappedConnectObject, 0, sizeof(OVERLAPPED));

	m_overLappedConnectObject.hEvent = m_hConnectEvent;

	m_waitConnectHandles[0] = m_hConnectEvent;
	m_waitConnectHandles[1] = m_hTerminateEvent;

	m_hPipeEvent = CreateEvent(0, false, 0, 0);

	memset(&m_overLappedPipeObject, 0, sizeof(OVERLAPPED));

	m_overLappedPipeObject.hEvent = m_hPipeEvent;

	m_waitPipeHandles[0] = m_hPipeEvent;
	m_waitPipeHandles[1] = m_hTerminateEvent;

	m_hWriteEvent = CreateEvent(0, false, 0, 0);

	memset(&m_overLappedWriteObject, 0, sizeof(OVERLAPPED));

	m_overLappedWriteObject.hEvent = m_hWriteEvent;

	m_waitWriteHandles[0] = m_hWriteEvent;
	m_waitWriteHandles[1] = m_hTerminateEvent;

	m_hPipe = CreateNamedPipeA(m_strPipeName.c_str(),
	 PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
	 PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE,
	 PIPE_UNLIMITED_INSTANCES,
	 4096,
	 4096,
	 0, NULL);

	return m_hPipe != INVALID_HANDLE_VALUE;
}

bool PYXNamedPipe::waitForServerResponse(std::string& strResponse)
{
	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
		// Make pipe if not already done.
		if (!createServer())
		{
			return false;
		}
	}

	BOOL bResult = ConnectNamedPipe(m_hPipe, &m_overLappedConnectObject);

	m_nLastError = 0;

	if (!bResult)
	{
	   m_nLastError = GetLastError();

	   if (m_nLastError != ERROR_IO_PENDING && m_nLastError != ERROR_PIPE_CONNECTED)
	   {
		   return false;
	   }
	   else
	   {
		   bResult = TRUE;
	   }
	}

	DWORD nResult = WAIT_OBJECT_0;

	if (m_nLastError != ERROR_PIPE_CONNECTED)
	{
		nResult = WaitForMultipleObjects(2, m_waitConnectHandles, false, INFINITE);
	}

	// Should we terminate?
	if (nResult == WAIT_OBJECT_0 + 1)
	{
		m_nLastError = -1;
		return false;
	}

	DWORD nBytesRead = 0;

	// Was a connect event signaled.
	if (nResult == WAIT_OBJECT_0)
	{
		bResult = ReadFile(m_hPipe, m_pipeBuffer, m_nPipeBufferLength, &nBytesRead, &m_overLappedPipeObject);

		if (!bResult)
		{
		   m_nLastError = GetLastError();

		   if (m_nLastError != ERROR_IO_PENDING)
		   {
			   return false;
		   }
		   else
		   {
			   bResult = TRUE;
		   }
		}

		nResult = WaitForMultipleObjects(2, m_waitPipeHandles, false, INFINITE);

		// Should we terminate?
		if (nResult == WAIT_OBJECT_0 + 1)
		{
			sendResponse(std::string("OK"));
			m_nLastError = -1;
			return false;
		}

		// Has read completed with an error?
		if (bResult == false && nResult == WAIT_OBJECT_0)
		{
			sendResponse(std::string("OK"));
			if (m_nLastError == ERROR_MORE_DATA)
			{
			   return false;
			}
		}
	}

	if (bResult && nResult == WAIT_OBJECT_0)
	{
		GetOverlappedResult(m_hPipe, &m_overLappedPipeObject, &nBytesRead, FALSE);
		m_pipeBuffer[nBytesRead] = 0;
		strResponse = m_pipeBuffer;
	}

	return true;
}

bool PYXNamedPipe::callNamedPipe(const std::string& strMessageToSend, std::string& strResponse)
{
	BOOL bResult;

	// If no pipe exists with this name we will return immediately.
	bResult = WaitNamedPipeA(m_strPipeName.c_str(), 15000);

	if (bResult)
	{
		char outBuffer[1000];
		DWORD nBytesRead;

		bResult = CallNamedPipeA(m_strPipeName.c_str(), (void*)strMessageToSend.c_str(), int(strMessageToSend.length() + 1), outBuffer, 999, &nBytesRead, 15000);

		outBuffer[nBytesRead] = 0;
		strResponse = outBuffer;

		return bResult == TRUE ? true : false;
	}

	m_nLastError = GetLastError();

	return false;
}
