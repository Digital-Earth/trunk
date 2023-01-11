#ifndef PIPE_BUILDER_H
#define PIPE_BUILDER_H
/******************************************************************************
pipe_builder.h

begin      : 08/04/2007 1:07:01 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "pyxlib.h"

// local includes
#include "process.h"

// pyxlib includes
#include "pyxis/data/catalog.h"
#include "pyxis/procs/path.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/pyxcom.h"

// boost includes
#include <boost/filesystem/operations.hpp>
#include <boost/intrusive_ptr.hpp>

/*!
IPipeBuilder interface. The PipeBuilder interface is used for writing custom PipeBuilders,
A PipeBuilder is a PYXCOM class which knows how to build Pipelines for various file 
formats. 
EX: A Pyxis PipeBuilder knows how to build Pipeline files for the ppl file types.
*/
//! IPipeBuilder interface.
struct PYXLIB_DECL IPipeBuilder : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Various data source checking options
	enum eCheckOptions
	{
		//! Partial checking - only the main file must be present
		knPartial = 0,

		//! Lenient checking - the main file and all required files must be present
		knLenient = 1,

		//! Strict checking - the main file, all required files and all optional files must be present
		knStrict = 2
	};

	/*!
	Determine if a specific data source can be read by this PipeBuilder. Implementations of this method
	may check file extensions, the content of directories or the contents of specific files to
	determine if the resource contains a data format which can be loaded.

	\param strPath	The path to the data source. May be a file, directory or url.
	\param options	The check options.

	\return true if the data source is supported
	*/
	virtual bool STDMETHODCALLTYPE isDataSourceSupported(const std::string& strPath, eCheckOptions options) const = 0;

	//! Build the catalog that describes the data source.
	virtual PYXPointer<const PYXCatalog> STDMETHODCALLTYPE buildCatalog(const std::string& strPath) const = 0;

	//! Set configuration options for pipeline builder. See specific class for options
	virtual void setConfig(const std::string & key, const std::string value);

	//! Build a pipeline for the data set.
	virtual PYXPointer<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const = 0;
};

/*!
Base class implementation for IPipeBuilder. The base class provides in a thread safe manner
the tracking of which files have been processed and adding a processed file to that list. It
abstracts away the need for individual PipeBuilders to handle this and focus on building pipelines.
*/
//! Base class implementation for IPipeBuilder
class PYXLIB_DECL PipeBuilderBase : public IPipeBuilder
{
public:

	//! Constants
	static const std::string kstrPipeBuilderIni;
	static const std::string kstrPipeBuilderPropScope;
	static const std::string kstrPipeBuilderPropKey;
	static const std::string kstrPipeBuilderDefaultPropValue;
	static const std::string kstrPipeBuilderPropDescription;

	//Default Constructor
	PipeBuilderBase();

	//! Determine if a specific data source can be read.
	virtual bool STDMETHODCALLTYPE isDataSourceSupported(const std::string& strPath, eCheckOptions options) const;

	//! Build the catalog that describes the data source.
	virtual PYXPointer<const PYXCatalog> STDMETHODCALLTYPE buildCatalog(const std::string& strPath) const;

	//! Set configuration options for pipeline builder. See specific class for options
	virtual void setConfig(const std::string & key, const std::string value);

	//! Build a pipeline for the data set.
	virtual PYXPointer<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const = 0;

	//! Get the default sampler from the properties file.
	/* config: default_sampler */
	virtual boost::intrusive_ptr<IProcess> getDefaultSampler() const;

protected:

	bool isZipFile(const std::string& strPath) const;

	//! Find any missing required files for a data set (all files must be present)
	std::auto_ptr< std::vector<std::string> > findMissingRequiredFilesAllOf(const std::string& strPath) const;

	//! Find any missing required files for a data set (one file must be present)
	std::auto_ptr< std::vector<std::string> > findMissingRequiredFilesOneOf(const std::string& strPath) const;

	//! Find any missing optional files for a data set
	std::auto_ptr< std::vector<std::string> > findMissingOptionalFiles(const std::string& strPath) const;

	//! Add any required and optional supporting files to the pipeline.
	virtual void addSupportingFiles(
		const boost::intrusive_ptr<IProcess>& process,
		const boost::filesystem::path& path) const;

	//! File extensions for all the supported single file data sources
	std::vector<std::string> m_vecSupportedExtensions;

	//! Required file extensions for multi-file data sources mapped to the main file extension (all files required)
	std::map<std::string, std::vector<std::string> > m_vecRequiredFilesByExtensionAllOf;

	//! Required file extensions for multi-file data sources mapped to the main file extension (one files required)
	std::map<std::string, std::vector<std::string> > m_vecRequiredFilesByExtensionOneOf;

	//! Optional file extensions for multi-file data sources mapped to the main file extension
	std::map<std::string, std::vector<std::string> > m_vecOptionalFilesByExtension;

private:
	//! Add existing files to the path proc by extension.
	void addSupportingFilesByExtension(
		const boost::intrusive_ptr<IPath>& pathProc,
		const boost::filesystem::path& path,
		const std::vector<std::string>& vecExtensions	) const;

private:
	//! config key "default_sampler"
	std::string m_defaultSamplerGuid;

};

#endif //guard