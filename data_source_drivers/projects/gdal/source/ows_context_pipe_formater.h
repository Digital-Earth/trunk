#ifndef OWS_CONTEXT_PIPE_FORMATER_H
#define OWS_CONTEXT_PIPE_FORMATER_H
/***************************************************************************
ows_context_pipe_formater.h

begin      : 15/11/2012 2:31:48 PM
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
***************************************************************************/
#include "module_gdal.h"
#include "ogr_process.h"
#include "pyxis/pipe/pipe_formater.h"

#include <set>

//TODO: Documentation.
class MODULE_GDAL_DECL OwsContextPipeFormater : public IPipeFormater
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IPipeFormater)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:

	//! Default Constructor
	OwsContextPipeFormater();

	//! Destructor
	~OwsContextPipeFormater(){;}

	//! Create pipelines.
	virtual bool canFormatPipeline(boost::intrusive_ptr<IProcess> pipeline) const;

	//! Create pipelines.
	virtual std::string formatPipeline(boost::intrusive_ptr<IProcess> pipeline) const;

private:
	virtual void addEnteyForPipeline(std::stringstream & stream,const boost::intrusive_ptr<IProcess> & pipeline, std::set<ProcRef> & visitedProcs) const;
};

#endif // end guard