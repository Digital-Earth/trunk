#ifndef PIPE_FORMATER_H
#define PIPE_FORMATER_H
/******************************************************************************
pipe_formater.h

begin      : 15/11/2012 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "pyxlib.h"

// local includes
#include "process.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/pyxcom.h"

// boost includes
#include <boost/filesystem/operations.hpp>
#include <boost/intrusive_ptr.hpp>

/*!
IPipeFormater interface. The PipeFormater interface is used for writing custom pipeline formaters.
A PipeBuilder is a PYXCOM class which knows how to format a give pipeline into various file formats. 
EX: A OSWContextPipeFormatr knows to export a pipeline into an OWS Context
*/
//! IPipeBuilder interface.
struct PYXLIB_DECL IPipeFormater : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! check if the pipeline formater can format 
	virtual bool canFormatPipeline(boost::intrusive_ptr<IProcess> pipeline) const = 0;

	virtual std::string formatPipeline(boost::intrusive_ptr<IProcess> pipeline) const = 0;
};

#endif //guard