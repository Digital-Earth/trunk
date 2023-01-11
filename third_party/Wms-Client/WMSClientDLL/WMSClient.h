#pragma once

#include "utilities.h"
#include "pyxnamedpipe.h"
#include <vector>



class WMSClient
	{
	public:
		WMSClient(int nInstance, std::string& strPipeName);

		~WMSClient();

		void processRequests();

		void processPipeMessages();

	protected:

	private:

	int getNumBytesFromHeader(std::string& strHeader);

	bool SocketReceiveFile(std::string& strPath, std::string& strFileName, std::string& strEnding);

	void getCapabilities();

	// Get the image that contains the specified lat/lon.
	bool getImage(std::string& strHost, std::string& strHostPath, std::string& strLayerName, std::string& strStyle, std::string& strFormat,
		          std::string& strImageDirectory, std::string& strPipeName, double fImageSize, int nLat, int nLon, int nWidth, int nLod,
				  bool bCheckForFile);

	// Get the image that contains the specified lat/lon.
	bool findInfo(std::string& strHost, std::string& strToFind);

	bool performRequest(std::string& strRequest, bool bCheckForFile);

	std::string getNextRequest();

	void addRequestToList(std::string& strRequest);

	void updateStats();

	std::vector<std::string>	m_vecRequests;
	std::vector<std::string>	m_vecTemp;

	HANDLE m_hProcessPipeThread;
	HANDLE m_hProcessRequestsThread;
	int m_nInstance;
	bool m_bTerminate;
	std::string m_strPipeName;
	pyxSemaphore m_pyxSemaphore;
	pyxMutex m_pyxMutex; 
	pyxSemaphore m_processPipeMessagesSemaphore;
	pyxSemaphore m_processRequestsSemaphore;
	PYXNamedPipe	m_serverPipe;

};

