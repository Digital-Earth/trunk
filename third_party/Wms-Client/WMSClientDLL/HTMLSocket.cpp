#include "StdAfx.h"
#include "HTMLSocket.h"

#include <Ws2tcpip.h>
#include <winsock2.h>
#include <ostream>
#include <io.h>
#include <fstream>

void addHTMLSocketMessage(const char* pWhere, const char* pWhat, const char* pOther = 0)
{
	//! Mutex to serialize concurrent access by multiple threads
	static pyxMutex theMutex;
	
	pyxMutexLock theLock(theMutex);

	std::ofstream ofs;
	ofs.open("c:\\htmlsocketlog.txt", std::ios::app);

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

const int HTMLSocket::knBufferSize = 10000;

HTMLSocket::HTMLSocket(std::string& strHost, int nPort, bool* pbTerminate)
:	m_strHost(strHost),
	m_nPort(nPort),
	m_hSocket(0),
	m_pbTerminate(pbTerminate)
{
	// Buffer for holding data received over a socket.
	m_buffer = new char[knBufferSize];

	m_nBufferPosn = 0;
}

HTMLSocket::~HTMLSocket(void)
{
	delete[] m_buffer;
}

void HTMLSocket::SocketSend(std::string& strRequest)
{
	std::string strRequestString;
	
	strRequestString = "GET ";
	strRequestString.append(strRequest);
	strRequestString.append(" HTTP/1.1\r\nHost: ");
	strRequestString.append(m_strHost);
	strRequestString.append("\r\nConnection: Close\r\n\r\n" );

	send(m_hSocket, strRequestString.c_str(), int(strRequestString.length()), 0);
}

SOCKET HTMLSocket::ConnectSocket()
{
	m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(m_hSocket == INVALID_SOCKET)
	{
		return 0;
	}

	struct addrinfo *aiList = NULL;

	char buffer[100];

	std::string strPort;

	_itoa_s(m_nPort, buffer, 99, 10);

	strPort = buffer;

	int nResult = getaddrinfo(m_strHost.c_str(), strPort.c_str(), NULL, &aiList);

	// If the call failed - exit.
	if (nResult != 0)
	{
		closesocket(m_hSocket);
		m_hSocket = 0;
		return 0;
	}

	//----------------------
	// Connect to server - if fail, close socket and exit.
	if ( connect( m_hSocket, aiList->ai_addr, sizeof(sockaddr) ) == SOCKET_ERROR)
	{
		freeaddrinfo(aiList);
		closesocket(m_hSocket);
		m_hSocket = 0;
		return 0;
	}

	char val = 1;

	// Disable nagle algorithm.  May help performance since we are sending small messages.
	nResult = setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, &val, 1);

	freeaddrinfo(aiList);

	return m_hSocket;
}

int HTMLSocket::getNumBytesFromHeader(std::string& strHeader)
{
	ci_string cistrHeader;
	
	cistrHeader = strHeader.c_str();
	
	ci_string cistrSearch("Content-Length:");

	size_t nPosn = cistrHeader.find(cistrSearch);

	// If not found we do not have a length.  Exit with appropriate code.
	if (nPosn == -1)
	{
		return -1;
	}

	ci_string strSize;
	
	strSize = cistrHeader.substr(nPosn+cistrSearch.length()+1, 20);

	int nNumBytes = atoi(strSize.c_str());

	return nNumBytes;
}

std::string HTMLSocket::SocketReceiveHeader()
{
	// Receive the server home page content.
	int bytes = 0;

	int nPosn = 0;

	// Read header info.
	do
	{
		bytes = recv(m_hSocket, &m_buffer[m_nBufferPosn], 249, 0);

		m_nBufferPosn += bytes;

		while (nPosn != -1 && nPosn < m_nBufferPosn - 3 && !(*m_pbTerminate))
		{
			if (m_buffer[nPosn] == 13 && m_buffer[nPosn+1] == 10 && m_buffer[nPosn+2] == 13 && m_buffer[nPosn+3] == 10)
			{
			nPosn += 4;

			std::string strRetPage;

			strRetPage.append(m_buffer,nPosn);

			int nNewLen = m_nBufferPosn - nPosn;

			memmove(m_buffer, &m_buffer[nPosn], nNewLen);

			m_nBufferPosn = nNewLen;

			return strRetPage;
			}
			else
			{
				++nPosn;
			}
		}
	}
	while ( bytes > 0  && !(*m_pbTerminate) );

   return std::string();
}

void HTMLSocket::SocketReceiveChunked(std::string& strFile)
{
	bool bNeedCRLF = true;
	bool bNeedSize = true;
	bool bHaveCRLF = false;
	bool bWriteData = false;
	bool bComplete = false;
	bool bRead = true;
	bool bDone = false;

	// Receive the server home page content.
	int bytes = m_nBufferPosn;

	int nPosn = 0;

	int nNumBytes = -1;

	std::ofstream ofs;

	ofs.open(strFile.c_str(), std::ios_base::binary);

	// Read data size from buffer.
	do
	{
		while (bNeedCRLF && nPosn < m_nBufferPosn - 1 && !(*m_pbTerminate))
		{
			if (m_buffer[nPosn] == 13 && m_buffer[nPosn+1] == 10)
			{
				bHaveCRLF = true;
				bNeedCRLF = false;
			}
			else
			{
			  ++nPosn;
			}
		}

		if (bHaveCRLF)
		{
			bHaveCRLF = false;
			if (bNeedSize)
			{
				bNeedSize = false;
				std::string strSize;

				nPosn += 2;

				strSize.append(m_buffer, nPosn);

				int nNewLen = m_nBufferPosn - nPosn;

				memmove(m_buffer, &m_buffer[nPosn], nNewLen);

				nNumBytes = 0;

				sscanf_s(strSize.c_str(), "%x", &nNumBytes, sizeof(nNumBytes));

				nPosn = -1;

				m_nBufferPosn = nNewLen;

			// If the chunk size is zero - we are done.
				if (nNumBytes == 0)
				{
					bComplete = true;
					bNeedCRLF = true;
				}
				else
				{
					bWriteData = true;
				}
			}
			else
			{
				// Discard CR/LF at end of string.
				m_nBufferPosn -= (nPosn+2);
				memmove(m_buffer, &m_buffer[nPosn+2], m_nBufferPosn);
				bHaveCRLF = false;
				bNeedCRLF = true;
				bNeedSize = true;
				bWriteData = false;

				nNumBytes = 0;

				if (bComplete)
				{
					bDone = true;
					bRead = false;
				}
			}
		}

		if (bWriteData && nNumBytes > 0)
		{
			int nBytesToWrite = nNumBytes > m_nBufferPosn ? m_nBufferPosn : nNumBytes;

			ofs.write(m_buffer, nBytesToWrite);

			memmove(m_buffer, &m_buffer[nBytesToWrite], m_nBufferPosn - nBytesToWrite);

			m_nBufferPosn -= nBytesToWrite;
			nNumBytes -= nBytesToWrite;

			if (nNumBytes == 0)
			{
				bNeedCRLF = true;
				bNeedSize = false;
			}
		}

		if (bRead)
		{
			bytes = recv(m_hSocket, &m_buffer[m_nBufferPosn], (knBufferSize-1)-m_nBufferPosn, 0 );
			m_nBufferPosn += bytes;
		}
	}
	while ( !bDone && !(*m_pbTerminate));

	ofs.close();

	m_nBufferPosn = 0;
}

void HTMLSocket::SocketReceiveData(int nNumBytes, std::string& strFileName)
{
   // Receive the server content.
   int bytes = m_nBufferPosn;

   std::ofstream ofs;

   ofs.open(strFileName.c_str(), std::ios_base::binary);

	// Read header info.
	do
	{
		if (bytes != 0)
		{
			int nBytesToWrite = min(bytes, nNumBytes);

			ofs.write(m_buffer, nBytesToWrite);

			memmove(m_buffer, &m_buffer[nBytesToWrite], m_nBufferPosn - nBytesToWrite);

			m_nBufferPosn -= nBytesToWrite;

			nNumBytes -=nBytesToWrite;
		}
		bytes = recv(m_hSocket, &m_buffer[m_nBufferPosn], (knBufferSize-1) - m_nBufferPosn, 0 );

		m_nBufferPosn += bytes;
	}
	while ( bytes > 0 && !(*m_pbTerminate));

	ofs.close();

	m_nBufferPosn = 0;
}

bool HTMLSocket::SocketReceiveFile(std::string& strPath, std::string& strFileName, std::string& strEnding)
{
	std::string strHeader = SocketReceiveHeader();

	ci_string cistrHeader(strHeader.c_str());

	ci_string strSearchChunked("Transfer-Encoding: chunked");
	ci_string strSearchLength("Content-Length:");

   size_t nPosnChunked = cistrHeader.find(strSearchChunked);
   size_t nPosnLength = cistrHeader.find(strSearchLength);
	
   // We have a file (either in chunks or by length) - read it.
  if (nPosnChunked != -1 || nPosnLength != -1)
  {
	  utilities::createDirectory(strPath);

	  std::string strFullName(strPath);
	  strFullName.append("\\");
	  strFullName.append(strFileName);
	  strFullName.append(strEnding);

	   if (nPosnChunked != -1)
	   {
			SocketReceiveChunked(strFullName);

			// If we have terminated - erase the file as download may not have completed.
			if ((*m_pbTerminate))
			{
				_unlink(strFullName.c_str());
				return false;
			}

			return true;
	   }

	   if (nPosnLength != -1)
	   {
		   int nNumBytes = getNumBytesFromHeader(strHeader);

		   if (nNumBytes != -1)
		   {
				SocketReceiveData(nNumBytes, strFullName);
			    // If we have terminated - erase the file as download may not have completed.
				if ((*m_pbTerminate))
				{
					_unlink(strFullName.c_str());
					return false;
				}

				return true;
		   }
	   }
  }
  else
  {
	   //Directory::CreateDirectory(strPath);
	   //String^ fullName = strPath + "\\" + fileName + ".err";
   //StreamWriter^ headerFileWriter = gcnew StreamWriter(fullName);

	   //headerFileWriter->Write(strHeader);

	   //headerFileWriter->Flush();

	   //headerFileWriter->Close();
  }
  return false;
}

void HTMLSocket::getCapabilities(std::string& strWMSPath)
{
	if (ConnectSocket() != 0)
	{
		std::string strRequest;
		strRequest = "/";
		strRequest.append(strWMSPath);
		strRequest.append("?VERSION=1.0.0&REQUEST=GetCapabilities&SERVICE=WMS&");

		SocketSend(strRequest); 

		std::string strPath("c:\\pyxis_wms_data");
		std::string strHostFileName;
		strHostFileName = m_strHost;

		utilities::replaceAll(strHostFileName, '.', '_');

		strPath.append("\\");
		strPath.append(strHostFileName);

		std::string strEnding(".txt");

		std::string strFileName("capabilities");
		bool bResult = SocketReceiveFile(strPath, strFileName, strEnding );

		shutdown(m_hSocket, SD_SEND); 

		closesocket(m_hSocket);

		m_hSocket = 0;
	}
}

bool HTMLSocket::getFile(std::string& strRequest, std::string& strPath, std::string& strFileName, std::string& strEnding)
{
	if (ConnectSocket() != 0)
	{
		SocketSend(strRequest); 

		bool bResult = SocketReceiveFile(strPath, strFileName, strEnding );

		shutdown(m_hSocket, SD_SEND); 

		closesocket(m_hSocket);

		m_hSocket = 0;

		return bResult;
	}

	return false;
}

