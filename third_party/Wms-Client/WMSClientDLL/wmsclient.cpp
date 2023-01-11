#include "stdafx.h"

#include "wmsclient.h"
#include "htmlsocket.h"
#include <io.h>
#include <fstream>
#include <vector>
#include <string>

pyxMutex myMutex;

static int nInstance = 0;
static std::vector<std::string>	vecPipes;
static std::vector<WMSClient*>  vecWMSClients;

//! Name of the plugin.  Displayed to user when selecting a DLL
//typedef int (*DLLMAKENEWINSTANCE)();
//typedef void (*DLLDELETEINSTANCE)(int nInstance);
//typedef void (*DLLGETPIPENAME)(int nInstance, char* szBuffer, int nMaxSize);

// Sample getCapabilities calls.

	//std::string strHost("wms.jpl.nasa.gov");
	//int nPort = 80;
	//HTMLSocket* pHTMLSocket = new HTMLSocket(strHost, nPort);
	//std::string strPath("wms.cgi");
	//pHTMLSocket->getCapabilities(strPath);
	//delete pHTMLSocket;

	//strHost = "viz.globe.gov"; //;
	//pHTMLSocket = new HTMLSocket(strHost, nPort);
	//strPath = "viz-bin/wmt.cgi";
	//pHTMLSocket->getCapabilities(strPath);
	//delete pHTMLSocket;

void addMessage(const char* pWhere, const char* pWhat, const char* pOther = 0)
{
	//! Mutex to serialize concurrent access by multiple threads
	static pyxMutex theMutex;
	
	pyxMutexLock theLock(theMutex);

	std::ofstream ofs;
	ofs.open("c:\\wmsmultidatasourcelog.txt", std::ios::app);

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

int DLLMakeNewInstance()
{
	pyxMutexLock myLock(myMutex);

	std::string strPipeName("\\\\.\\pipe\\WebDataSourceServerPipe");

	int nInstance = int(vecPipes.size());

	strPipeName.append(utilities::toString(nInstance));

	vecPipes.push_back(strPipeName);

	WMSClient* pWMSClient = new WMSClient(nInstance, strPipeName);

	vecWMSClients.push_back(pWMSClient);

	return nInstance;
}

void DLLDeleteInstance(int nInstance)
{
	pyxMutexLock myLock(myMutex);

	if (nInstance < int(vecWMSClients.size()))
	{
		delete vecWMSClients[nInstance];
		vecWMSClients[nInstance] = 0;
	}
}

void DLLGetPipeName(int nInstance, char* szBuffer, int nMaxSize)
{
	pyxMutexLock myLock(myMutex);

	if (nInstance < int(vecPipes.size()))
	{
		if ( int(vecPipes[nInstance].length()) < nMaxSize)
		{
			strcpy_s(szBuffer, nMaxSize, vecPipes[nInstance].c_str());
		}
		else
		{
			szBuffer[0] = 0;
		}
	}
}

/*!
Internal routine (windows specific) started by the thread and used to start
the class specific run routine.

\param	ptr	Pointer to 'this'.
*/
DWORD WINAPI processPipeMessagesHandler(LPVOID ptr)
{
	(reinterpret_cast<WMSClient*>(ptr))->processPipeMessages();
	return 0;
}

/*!
Internal routine (windows specific) started by the thread and used to start
the class specific run routine.

\param	ptr	Pointer to 'this'.
*/
DWORD WINAPI processRequestsHandler(LPVOID ptr)
{
	(reinterpret_cast<WMSClient*>(ptr))->processRequests();
	return 0;
}

WMSClient::WMSClient(int nInstance, std::string& strPipeName)
: m_serverPipe(strPipeName),
  m_nInstance(nInstance)
{
	m_bTerminate = false;

	DWORD threadID;

	m_serverPipe.createServer();

	// Create thread to handle communication back to the data source.
	m_hProcessPipeThread = CreateThread(0, 0, processPipeMessagesHandler, this, 0, &threadID);

	// Create thread to handle requests (ie. downloading files) for the data source.
	m_hProcessRequestsThread = CreateThread(0, 0, processRequestsHandler, this, 0, &threadID);
}


/// <summary>
/// Clean up any resources being used.
/// </summary>
WMSClient::~WMSClient()
{
	m_bTerminate = true;

	updateStats();

	// Terminating pipe will terminate the thread using it.
	m_serverPipe.terminate();

	// Signal request processing thread to terminate.
	m_processRequestsSemaphore.signal();

	HANDLE waitObjects[2];

	waitObjects[0] = m_hProcessRequestsThread;
	waitObjects[1] = m_hProcessPipeThread;

	// Wait for both threads to exit.
	WaitForMultipleObjects(2, waitObjects, TRUE, INFINITE);

	CloseHandle(m_hProcessRequestsThread);

	CloseHandle(m_hProcessPipeThread);
}


// Get the image that contains the specified lat/lon.
bool WMSClient::getImage(std::string& strHost, std::string& strHostPath, std::string& strLayerName, std::string& strStyle, std::string& strFormat,
	          std::string& strImageDirectory, std::string& strPipeName, double fImageSize, int nLat, int nLon, int nWidth, int nLod,
			  bool bCheckForFile)
{
	bool bBlocking = false;

	bool bOK = true;

	double fLat = double (nLat) / 1000000.0;
	double fLon = double (nLon) / 1000000.0;

	double fLatTop = double (nLat + nWidth) / 1000000.0;
	double fLonRight = double (nLon + nWidth) / 1000000.0;

//	fLonRight = min(180.0, fLonRight);
//	fLatTop = min(90.0, fLatTop);

//	fLatTop = min(90.0, fLatTop);
//	fLonRight = min(180.0, fLonRight);

	// Lat and Lon are offset so they are never negative.
	std::string strImageName;

	strImageName = utilities::toString(nLat) + "_" + utilities::toString(nLon);

	utilities::replaceAll(strImageName, '-', 'n');

	std::string strHostName = strHost;

	utilities::replaceAll(strHostName, '.', '_');

	std::string strPath;

	strPath = strImageDirectory;
	strPath.append("\\");
	strPath.append(strHostName);
	strPath.append("\\");
	strPath.append(strLayerName);
	strPath.append("\\");
	strPath.append(strStyle);
	strPath.append("\\");
	strPath.append(strFormat);
	strPath.append("\\");
	strPath.append(utilities::toString(nLod));

	utilities::replaceAll(strPath, ',', '_');
	utilities::replaceAll(strPath, '/', '_');

	std::string strSavedFileName;
	std::string strSavedFileWorldName;

	std::string strEnding;

	ci_string cistrFormat;

	cistrFormat = strFormat.c_str();

	if (cistrFormat.find("jpeg") != std::string::npos || cistrFormat.find("jpg") != std::string::npos)
	{
	strEnding = ".jpg";
	}

	if (cistrFormat.find("tif") != std::string::npos)
	{
	strEnding = ".tif";
	}

	if (cistrFormat.find("png") != std::string::npos)
	{
	strEnding = ".png";
	}

	if (cistrFormat.find("gif") != std::string::npos)
	{
	strEnding = ".gif";
	}

	strSavedFileName = strPath;
	strSavedFileName.append("\\");
	strSavedFileName.append(strImageName);
	strSavedFileName.append(strEnding);

	strSavedFileWorldName = strPath;
	strSavedFileWorldName.append("\\");
	strSavedFileWorldName.append(strImageName);
	strSavedFileWorldName.append(".wld");

	std::string strPipeResponse;

	if (bCheckForFile)
	{
		// We already have the file - let client know.
		if (utilities::isFile(strSavedFileName) && utilities::isFile(strSavedFileWorldName))
		{
			strPipeResponse = "<file> ";
			strPipeResponse.append(utilities::toString(nLon)); 
			strPipeResponse.append(" ");
			strPipeResponse.append(utilities::toString(nLat));
			strPipeResponse.append(" ");
			strPipeResponse.append(utilities::toString(nLod));
			strPipeResponse.append(" ");
			strPipeResponse.append(strSavedFileName);
			m_serverPipe.sendResponse(strPipeResponse);
			return true;
		}		

		if (!bBlocking)
		{
			//<download>,lonMin,LatMin,LonMax,LatMax
			std::string strRequest;

			strRequest = "<download> ";
			strRequest.append(utilities::toString(nLon));
			strRequest.append(" ");
			strRequest.append(utilities::toString(nLat));
			strRequest.append(" ");
			strRequest.append(utilities::toString(nLod));
			strRequest.append(" ");

			m_serverPipe.sendResponse(strRequest);

			return false;
		}
	}

   // Check to see if this file is already in our cache - if so return it.
	if (utilities::isFile(strSavedFileName) && utilities::isFile(strSavedFileWorldName))
	{
		strPipeResponse = "<file> ";
		strPipeResponse.append(utilities::toString(nLon)); 
		strPipeResponse.append(" ");
		strPipeResponse.append(utilities::toString(nLat));
		strPipeResponse.append(" ");
		strPipeResponse.append(utilities::toString(nLod));
		strPipeResponse.append(" ");
		strPipeResponse.append(strSavedFileName);
	}		
	else
	{
		// Not already in cache - request from server.
		std::string strRequest;
		strRequest = strHostPath;
		strRequest.append("?VERSION=1.1.1&REQUEST=GetMap&");
		strRequest.append("SRS=EPSG%3A4326&bbox=");
		strRequest.append(utilities::toString(fLon));
		strRequest.append(",");
		strRequest.append(utilities::toString(fLat));
		strRequest.append(",");
		strRequest.append(utilities::toString(fLonRight));
		strRequest.append(",");
		strRequest.append(utilities::toString(fLatTop));
		strRequest.append("&layers=");
		strRequest.append(strLayerName);
		strRequest.append("&width=");
		strRequest.append(utilities::toString(fImageSize));
		strRequest.append("&height=");
		strRequest.append(utilities::toString(fImageSize));
		strRequest.append("&styles=");
		if (strStyle.find("blank") == std::string::npos)
		{
			strRequest.append(strStyle);
		}
		strRequest.append("&format=");
		strRequest.append(strFormat);
		strRequest.append("&BGCOLOR=0x000000&");

		int nPort = 80;

		HTMLSocket* pHTMLSocket = new HTMLSocket(strHost, nPort, &m_bTerminate);

		bool bResult = pHTMLSocket->getFile(strRequest, strPath, strImageName, strEnding);

		delete pHTMLSocket;

		// If we have to terminate - exit now.
		if (m_bTerminate)
		{
			return false;
		}

		if (bResult)
		{
			std::ifstream ifs;

			std::string strFileName;

			strFileName = strPath;
			strFileName.append("\\");
			strFileName.append(strImageName);
			strFileName.append(strEnding);

			ifs.open(strFileName.c_str());

			char buffer[10];
			ifs.get(buffer, 8);

			ifs.close();

			buffer[5] = 0;

			// Most likely cause of following is busy server.  Slow down requests.
			if(_stricmp(buffer,"<?xml") == 0)
			{
				_unlink(strFileName.c_str());

				//TODO check for various types of errors and act accordingly - ie. close data source if server not available.
				bOK = false;

				bResult = false;
			}
		}

		// Only create world file if we received a file.
		if (bResult)
		{
			// Create world file to go with data file.
			std::ofstream ofs;

			ofs.open(strSavedFileWorldName.c_str());

			ofs << utilities::toString((fLonRight - fLon) / (fImageSize-1)).c_str() << std::endl; // XScale
			ofs << "0.0000000" << std::endl; // Rotation Term
			ofs << "0.0000000" << std::endl; // Rotation Term
			ofs << utilities::toString((fLatTop - fLat) / -(fImageSize-1)).c_str() << std::endl; // Negative Y Scale
			ofs << utilities::toString(fLon) << std::endl;	// X Translation term
			ofs << utilities::toString(fLatTop) << std::endl;	// Y Translation term.

			ofs.close();

			strPipeResponse = "<file> ";
			strPipeResponse.append(utilities::toString(nLon));
			strPipeResponse.append(" ");
			strPipeResponse.append(utilities::toString(nLat));
			strPipeResponse.append(" ");
			strPipeResponse.append(utilities::toString(nLod));
			strPipeResponse.append(" ");
			strPipeResponse.append(strSavedFileName);
		}
		else
		{
			strPipeResponse = "<error> NO DATA FROM SERVER";
			bOK = false;
		}
	}

	if (bBlocking)
	{
		m_serverPipe.sendResponse(strPipeResponse);
		return true;
	}

	// Do not call client if we are terminating.
	if (!m_bTerminate)
	{
		PYXNamedPipe responsePipe(strPipeName);

		std::string strResponse;

		responsePipe.callNamedPipe(strPipeResponse, strResponse);
	}

	return bOK;
}


// Get the image that contains the specified lat/lon.
bool WMSClient::findInfo(std::string& strHost, std::string& strToFind)
{
	std::string strPipeResponse;

	// Not already in cache - request from server.
	std::string strRequest;

	std::string strFindString(strToFind);
	
	utilities::replaceAll(strFindString, ' ', '+');

	strRequest = "http://";
	strRequest.append(strHost);
	strRequest.append("/?locate=");
	strRequest.append(strFindString);
	strRequest.append("&geoit=xml");

	std::string strPath = "c:\\pyxis_wms_data";
	std::string strFileName("locateinfo");
	std::string strEnding(".txt");

	int nPort = 80;

	HTMLSocket* pHTMLSocket = new HTMLSocket(strHost, nPort, &m_bTerminate);

	bool bResult = pHTMLSocket->getFile(strRequest, strPath, strFileName, strEnding);

	delete pHTMLSocket;

	if (bResult)
	{
		std::ifstream ifs;

		std::string strPathAndFile;

		strPathAndFile = strPath;
		strPathAndFile.append("\\");
		strPathAndFile.append(strFileName);
		strPathAndFile.append(strEnding);

		ifs.open(strPathAndFile.c_str());

		char buffer[1000];

		double fLat = 1e99;
		double fLon = 1e99;

		while (!ifs.eof() && (fLat == 1e99 || fLon == 1e99))
		{
			ifs.getline(buffer, 999);

			int nLen = ifs.gcount();

			if (nLen > 0)
			{
				char* szPosn = strstr(buffer, "att>");

				if (szPosn != 0)
				{
					fLat = atof(szPosn + 4);
				}

				szPosn = strstr(buffer, "ongt>");

				if (szPosn !=0)
				{
					fLon = atof(szPosn + 5);
				}
			}
			else
			{
				break;
			}
		}

		ifs.close();

		if (fLat != 1e99 && fLon != 1e99)
		{
			strPipeResponse = "<found> ";
			strPipeResponse.append(utilities::toString(fLat));
			strPipeResponse.append(" ");
			strPipeResponse.append(utilities::toString(fLon));
		}
		else
		{
			strPipeResponse = "<error> NO DATA FROM SERVER";
		}
	}
	else
	{
		strPipeResponse = "<error> NO DATA FROM SERVER";
	}

	if (bResult)
	{
		m_serverPipe.sendResponse(strPipeResponse);
	}

	return bResult;
}

bool WMSClient::performRequest(std::string& strRequest, bool bCheckForFile)
{
	char* pBuffer = new char[strRequest.length() + 1];
	memcpy(pBuffer, strRequest.c_str(), strRequest.length()+1);
	char* pPtrNext;
	char* pPtr = strtok_s(pBuffer, "|", &pPtrNext);

	int nField = 0;

	std::string strCommand;
	std::string strHost;
	std::string strHostPath;
	std::string strLayerName;
	std::string strStyle;
	std::string strFormat;
	std::string strPipeName;
	
	double fImageSize = 0.0;
	int nLat = 0;
	int nLon = 0;
	int nWidth = 0;
	int nLod = 0;

	while (pPtr != 0)
	{
		// Command.
		switch( nField )
		{
		case 0: // Command
			strCommand = pPtr;
			break;
		case 1: // Pipe name to send delayed response to.
			strPipeName = pPtr;
			break;
		case 2: // Host
			strHost = pPtr;
			break;
		case 3: // HostPath
			strHostPath = pPtr;
			break;
		case 4: // LayerName
			strLayerName = pPtr;
			break;
		case 5: // Style
			strStyle = pPtr;
			break;
		case 6: // Format
			strFormat = pPtr;
			break;
		case 7: // Image Size
			fImageSize = atof(pPtr);
			break;
		case 8: // Lat
			nLat = atoi(pPtr);
			break;
		case 9: // Lon
			nLon = atoi(pPtr);
			break;
		case 10: // Width
			nWidth = atoi(pPtr);
			break;
		case 11: // LOD
			nLod = atoi(pPtr);
			break;
		}
		
		++nField;
		pPtr = strtok_s(NULL, "|", &pPtrNext);
	}

	delete[] pBuffer;

	bool bResult = false;

	if (strCommand == "getimage" && nField == 12)
	{
		std::string strImageDirectory("c:\\pyxis_wms_data");

		bResult = getImage(strHost, strHostPath, strLayerName, strStyle, strFormat, strImageDirectory, strPipeName, fImageSize, nLat, nLon, nWidth, nLod, bCheckForFile);
	}
	else
	if (strCommand == "findinfo" && nField == 3)
	{
		bResult = findInfo(strPipeName, strHost);
		bCheckForFile = true;
	}
	else
	if (strCommand == "getcapabilities" && nField == 4)
	{
		int nPort = 80;
		bool bTerminate = false;

		HTMLSocket* pHTMLSocket = new HTMLSocket(strHost, nPort, &bTerminate);
		pHTMLSocket->getCapabilities(strHostPath);
		delete pHTMLSocket;

		std::string strPipeResponse("<complete>");

		m_serverPipe.sendResponse(strPipeResponse);

		bCheckForFile = true;
		bResult = true;
	}
	else
	{
		std::string strPipeResponse("<unknowncommand>");
		m_serverPipe.sendResponse(strPipeResponse);
	}


	// If bCheckForFile is true, bResults will be false if the file does not already exist (ie. has to be downloaded).
	// Do not add request again under this condition as the request will be passed on in other code.
	if (!bResult && !bCheckForFile && !m_bTerminate)
	{
		// Server is busy - wait a couple of seconds and try again.
		Sleep(2000);

		// Add priority to start of request.
		std::string strNewRequest("0");
		strNewRequest.append(strRequest);

		// If the download failed - place request back on stack.
		addRequestToList(strNewRequest);
	}

	return bResult;
}

std::string WMSClient::getNextRequest()
{
	pyxMutexLock mutexLock(m_pyxMutex);

	if (!m_vecRequests.empty())
	{
		std::vector<std::string>::iterator it = m_vecRequests.begin();

		std::vector<std::string>::iterator itNext = it;

		char szPriority = 0;

		while (it != m_vecRequests.end())
		{
			if ((*it)[0] > szPriority)
			{
				szPriority = (*it)[0];
				itNext = it;
			}
			++it;
		}

		std::string strTheRequest = *itNext;
		std::string strRequest(strTheRequest.substr(1).c_str());

		m_vecRequests.erase(itNext);

		updateStats();

		return strRequest;
	}

	updateStats();

	return std::string();
}

void WMSClient::addRequestToList(std::string& strRequest)
{
	// Wait until we get ownership of mutex before continuing.
	pyxMutexLock mutexLock(m_pyxMutex);

	bool bIsEmpty = m_vecRequests.empty();

	m_vecRequests.push_back(strRequest);

	updateStats();

	m_pyxSemaphore.signal();
}

void WMSClient::processRequests()
{
	while (!m_bTerminate)
	{
		bool bWait = false;

		{
			pyxMutexLock mutexLock(m_pyxMutex);
		
			bWait = m_vecRequests.empty();
		}

		// If we have items in the vector - signal the semaphore so we can process them.
		if (!bWait)
		{
			m_pyxSemaphore.signal();
		}

		HANDLE waitHandles[2];

		waitHandles[0] = m_pyxSemaphore.m_hSemaphore;
		waitHandles[1] = m_processRequestsSemaphore.m_hSemaphore;

		DWORD nResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

		// If asked to terminate - exit.
		if (nResult == WAIT_OBJECT_0 + 1)
		{
			break;
		}

		if (nResult == WAIT_OBJECT_0)
		{
			std::string strRequest(getNextRequest());
			
			if (strRequest.length() > 0)
			{
				// This does not have to be done in the mutex and is faster and removes restrictions when not in the mutex.
				performRequest(strRequest, false);
			}
		}
	}
}

void WMSClient::processPipeMessages()
{
    DWORD nBytesRead = 1;
	while (nBytesRead > 0 && !m_bTerminate)
	{
		std::string strResponse;

		bool bResult = m_serverPipe.waitForServerResponse(strResponse);

		if (bResult && !strResponse.empty() && !m_bTerminate)
	    {
		   if (strResponse.find("terminate") != std::string::npos)
		   {
			    m_bTerminate = true;
				m_serverPipe.sendResponse(std::string("OK"));
		   }
		   else
		   {
			  // Do we have to remove all pending requests for this client from our list?
			  if (strResponse.find("removerequests") != std::string::npos)
			  {
				m_serverPipe.sendResponse(std::string("OK"));

				if (strResponse.length() > 16)
				{
					// Get the Mutex.
					pyxMutexLock mutexLock(m_pyxMutex);

					std::string strRequester(strResponse.substr(15));

					m_vecTemp.clear();

					for (size_t x = 0; x < m_vecRequests.size(); ++x)
					{
						if (m_vecRequests[x].find(strRequester) == std::string::npos)
						{
							m_vecTemp.push_back(m_vecRequests[x]);
						}
					}

					m_vecRequests.clear();

					for (size_t x = 0; x < m_vecTemp.size(); ++x)
					{
						m_vecRequests.push_back(m_vecTemp[x]);
					}

					m_vecTemp.clear();

					updateStats();
				}
			  }
			  else
			  {
					// This call will either send the "here is the file" or "downloading" message back to the client.
					// Remember to remove priority from request.
					if (!performRequest(strResponse.substr(1), true) )
					{
						// We do not have the requested area.  Add to list of items to download.
						addRequestToList(strResponse);
					}
			  }
		   }
	   }
   }
}

void WMSClient::updateStats()
{
	std::string strStatsPipe("\\\\.\\pipe\\WebDataSourceStatsPipe");

	PYXNamedPipe responsePipe(strStatsPipe);

	size_t nPendingItems = 0;

	if (!m_bTerminate)
	{
		pyxMutexLock mutexLock(m_pyxMutex);
	
		nPendingItems = m_vecRequests.size();
	}

	std::string strResponse;

	std::string strMessage;

	strMessage = utilities::toString(m_nInstance);
	strMessage.append(",");
	strMessage.append(utilities::toString(nPendingItems));

	responsePipe.callNamedPipe(strMessage, strResponse);
}

