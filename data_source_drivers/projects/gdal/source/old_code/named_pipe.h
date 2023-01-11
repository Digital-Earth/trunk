#ifndef NAMED_PIPE
#define NAMED_PIPE

#include "module_gdal.h"
#include <windows.h>
#include <string>
//TODO: Code clean up after changest tested. CHOWELL
class PYXNamedPipe
{
public:

	PYXNamedPipe(const std::string& strPipeName);
	PYXNamedPipe();

	~PYXNamedPipe();

	void setPipeName(const std::string& strPipeName) { m_strPipeName = strPipeName; };

	bool createServer();

	bool sendResponse(const std::string& strResponse);

	bool waitForServerResponse(std::string& strResponse);

	bool callNamedPipe(const std::string& strMessageToSend, std::string& strResponse);

	bool terminate();

	int getLastError() { return m_nLastError; }

private:

	HANDLE m_hConnectEvent;
	HANDLE  m_hWriteEvent;
	HANDLE  m_hPipeEvent;
	HANDLE  m_hTerminateEvent;
	HANDLE  m_hPipe;

	HANDLE  m_waitConnectHandles[2];
	HANDLE  m_waitWriteHandles[2];
	HANDLE  m_waitPipeHandles[2];

	OVERLAPPED  m_overLappedConnectObject;
	OVERLAPPED  m_overLappedWriteObject;
	OVERLAPPED  m_overLappedPipeObject;

	std::string m_strPipeName;
	char* m_pipeBuffer;
	int m_nLastError;
	int m_nPipeBufferLength;
};

#endif
