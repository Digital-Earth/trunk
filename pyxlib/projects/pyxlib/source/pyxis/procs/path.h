#ifndef PYXIS__PROCS__PATH_H
#define PYXIS__PROCS__PATH_H
/******************************************************************************
path.h

begin      : 20/09/2007 4:40:38 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/procs/url.h"
#include "pyxis/sampling/spatial_reference_system.h"

//! Interface File or directory path processes
struct PYXLIB_DECL IPath : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! request the stored file location.
	virtual const std::string STDMETHODCALLTYPE  getPath(int index = 0) const = 0;

	//! Returns a path to a local copy of the file, if available.
	virtual const std::string STDMETHODCALLTYPE getLocallyResolvedPath(int index = 0) const = 0;

	/*!
	Set a new absolute path for the process. If the path is well formed and
	exists the path will be set. If it is not then the operation will fail.

	\return true if the path is well formed and exists, otherwise false.
	*/
	//! Set the stored file location.
	virtual bool STDMETHODCALLTYPE setPath(const std::string& path, int index = 0) = 0;

	//! Adds a new path to the list of paths.
	virtual bool STDMETHODCALLTYPE addPath(const std::string& strPath) = 0;

	//! Gets the number of paths that have been set.
	virtual int STDMETHODCALLTYPE getLength() const = 0;
};

//! An error that indicates one of the input parameters could not be successfully initialized.
class PYXLIB_DECL PathInitError : public ProcInitErrorBase
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(PathInitError)

	PathInitError()
	{
		m_strError = "Path init error";
	}
};

/*!
This process will only initialize successfully if the specified path
exists.
*/
//! A process that defines a local or network file or directory resource.
class PYXLIB_DECL PathProcess : public ProcessImpl<PathProcess>, public IPath, public IUrl
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

	//! Constructor
	PathProcess() : m_bIsDirectory(false) {}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IPath)
		IUNKNOWN_QI_CASE(IUrl)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(PathProcess,IPath);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IPath*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IPath*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

	virtual std::string STDMETHODCALLTYPE getIdentity() const;

public: // IPath

	virtual const std::string STDMETHODCALLTYPE getPath(int index = 0) const;

	virtual const std::string STDMETHODCALLTYPE getLocallyResolvedPath(int index = 0) const;

	virtual bool STDMETHODCALLTYPE setPath(const std::string& strPath, int index = 0);

	virtual bool STDMETHODCALLTYPE addPath(const std::string& strPath);

	virtual int STDMETHODCALLTYPE getLength() const {return m_paths.size(); }

public: // IUrl

	//! Gets the URL.
	virtual const std::string STDMETHODCALLTYPE getUrl() const;

	//! Sets the URL
	virtual bool STDMETHODCALLTYPE setUrl(const std::string& url);

	virtual std::string STDMETHODCALLTYPE getManifest() const;

	//! Return true if the URL refers to a local file.
	virtual bool STDMETHODCALLTYPE isLocalFile() const { return true; }

private:

	//! Set to true if the first path is a directory and the subsequent paths are files in that directory
	bool m_bIsDirectory;

	std::vector<std::string> m_paths;
	mutable std::vector<std::string> m_resolvedPaths;
	
	//Enable maintaining const correctness from interface.
	mutable std::string m_strManifest;

};

//! Management class to hold notifiers for file objects.
class PYXLIB_DECL FileNotificationManager
{
public:
	/*! notifier the fires when a process would like to have a file
	    that it can't find.
	*/
	static Notifier& getFileNeededNotifier()
	{
		return m_fileNeededNotifier;
	}

	static Notifier& getPipelineFilesDownloadNeededNotifier()
	{
		return m_pipelineFilesDownloadNeededNotifier;
	}

private:
	//! The new cache notifier
	static Notifier m_fileNeededNotifier;

	//! The pipeline supporting files download notifier
	static Notifier m_pipelineFilesDownloadNeededNotifier;
};

//! Base class for all IPath events.
class PYXLIB_DECL FileEvent : public NotifierEvent
{
public:
	//! Creator
	static PYXPointer<FileEvent> create(boost::intrusive_ptr<IPath> spPath, int index = 0)
	{
		return PYXNEW(FileEvent, spPath, index);
	}


	//! The index of the missing file.
	int getIndex() const
	{
		return m_index;
	}

	//! Returns the IPath process that triggered the event.
	boost::intrusive_ptr<IPath> getPath()
	{
		return m_spPath;
	}

	//! Returns true if the download of the file failed.
	const bool getFailed()
	{
		return m_bFailed;
	}

	//! Sets the flag ot indicate whether the download failed.
	void setFailed(const bool bFailed)
	{
		m_bFailed = bFailed;
	}

	//! The locally resolved filepath (or empty if none are available)
	const std::string &getLocalPath()
	{
		return m_localPath;
	}

	//! Sets the locally resolved filepath (or empty if none are available)
	void setLocalPath( const std::string &value)
	{
		m_localPath = value;
	}

protected:

	FileEvent(boost::intrusive_ptr<IPath> spPath, int index = 0) :
		m_spPath(spPath), m_index(index)
	{
		m_bFailed = true;
	}

private:

	//! The process that triggered the event.
	boost::intrusive_ptr<IPath> m_spPath;

	//! The index of the missing path.
	int m_index;

	//! Whether the download of the file failed.
	bool m_bFailed;

	// The locally resolved filepath (or empty if none are available)
	std::string m_localPath;
};

//! Generic request to download all files for a given pipelines
class PYXLIB_DECL PipelineFilesEvent : public NotifierEvent
{
public:
	//! Creator
	static PYXPointer<PipelineFilesEvent> create(boost::intrusive_ptr<IProcess> spProc)
	{
		return PYXNEW(PipelineFilesEvent, spProc);
	}


	//! Returns the IPath process that triggered the event.
	boost::intrusive_ptr<IProcess> getProcess()
	{
		return m_spProc;
	}

	//! Returns true if the download of the file failed.
	const bool getFailed()
	{
		return m_bFailed;
	}

	//! Sets the flag ot indicate whether the download failed.
	void setFailed(const bool bFailed)
	{
		m_bFailed = bFailed;
	}

protected:

	PipelineFilesEvent(boost::intrusive_ptr<IProcess> spProc) :
		m_spProc(spProc)
	{
		m_bFailed = true;
	}

private:

	//! The process that triggered the event.
	boost::intrusive_ptr<IProcess> m_spProc;

	//! Whether the download of the file failed.
	bool m_bFailed;
};

#endif // guard
