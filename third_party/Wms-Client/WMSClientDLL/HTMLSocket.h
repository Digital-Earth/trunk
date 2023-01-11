#pragma once

#include "utilities.h"

#include <winsock2.h>
#include <string>

class HTMLSocket
{
public:

	static const int knBufferSize;

	HTMLSocket(std::string& strHost, int nPort, bool* pbTerminate);
	virtual ~HTMLSocket(void);

	void getCapabilities(std::string& strPath);

	bool getFile(std::string& strRequest, std::string& strPath, std::string& strFileName, std::string& strEnding);

private:

	void SocketSend(std::string& strRequest);
	SOCKET ConnectSocket();
	int getNumBytesFromHeader(std::string& strHeader);
	std::string SocketReceiveHeader();
	void SocketReceiveChunked(std::string& strFile);
	void SocketReceiveData(int nNumBytes, std::string& strFileName);
	bool SocketReceiveFile(std::string& strPath, std::string& strFileName, std::string& strEnding);

	char* m_buffer;
	int m_nBufferPosn;
	SOCKET m_hSocket;
	std::string m_strHost;
	int m_nPort;
	bool* m_pbTerminate;

};
