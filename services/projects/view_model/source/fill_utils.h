#pragma once
#ifndef VIEW_MODEL__FILL_UTILS_H
#define VIEW_MODEL__FILL_UTILS_H
/******************************************************************************
fill_utils.h

begin		: 2007-10-05
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"
#include "view_model_api.h"

// view model includes
#include "stile.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"

// standard includes
#include <vector>

// forward declarations
struct IViewPoint;

/*!
*/
//! Filler thread utilities.
class VIEW_MODEL_API FillUtils
{

public:

	static void setFillTiles(
		PYXPointer<IViewModel> view,
		boost::intrusive_ptr<ICoverage> spCoverage, 
		int nMipLevel, 		
		ProcRef procRef, 
		boost::intrusive_ptr<IViewPoint> spViewPoint, 
		const std::vector<PYXPointer<STile>>& vecTiles, 		
		std::string& strError);

	//! get called on app closeing
	static void closeAllResources();

private:

	static void createFillManager(
		PYXPointer<IViewModel> view,
		boost::intrusive_ptr<ICoverage> spCoverage, 
		int nMipLevel, 
		ProcRef procRef, 
		boost::intrusive_ptr<IViewPoint> spViewPoint);

};

#endif
