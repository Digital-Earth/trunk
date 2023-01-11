#pragma once
#ifndef bitmap_grid_process_H
#define bitmap_grid_process_H
/******************************************************************************
bitmap_grid_process.h

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/procs/bitmap.h"

class MODULE_FEATURE_PROCESSING_PROCS_DECL BitmapGridProcess : public ProcessImpl<BitmapGridProcess>, public IBitmap
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	BitmapGridProcess();

	//! Unit testing method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IBitmap)		
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IBitmap*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IBitmap*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IBitmap

	//! Returns bitmap width in pixels
	virtual int STDMETHODCALLTYPE getWidth(int index = 0);

	//! Returns bitmap height in pixels
	virtual int STDMETHODCALLTYPE getHeight(int index = 0);

	//! Returns a pointer to the RAW bitmap
	virtual const PYXPointer<PYXBitmap> STDMETHODCALLTYPE getRawBitmap(int index = 0);	
	
	//! Returns the number bitmaps provided by process
	virtual int getBitmapCount();

	virtual std::string getBitmapDefinition(int index = 0);

protected:
	boost::intrusive_ptr<IBitmap> m_spInputBitmap;

	int m_cols;
	int m_rows;

	int m_inputIndex;
	int m_maximumIndex;
	
	int  m_loadedImageIndex;
	PYXPointer<PYXBitmap> m_bitmap;

protected:
	void loadImage(int index);	
};



#endif // guard
