#ifndef PYXIS__PROCS__BITMAP_H
#define PYXIS__PROCS__BITMAP_H
/******************************************************************************
bitmap.h

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"


// boost includes
#include <boost/scoped_array.hpp>


/*! PYXBitmap - represnt a raw format RGBA.

the creation of PYXBitmap is using BitmapServerProvider. 
Therefore, a PYXBitmap can be created only after BitmapServerProvider was set.
*/
//! PYXBitmap - represnt a raw format RGBA.
class PYXLIB_DECL PYXBitmap : public PYXObject
{
public:
	static PYXPointer<PYXBitmap> createFromDefinition(const std::string & definition);
	static PYXPointer<PYXBitmap> createFromPath(const std::string & path);
	static PYXPointer<PYXBitmap> create(const unsigned int & width,const unsigned int & height);

public:
	PYXBitmap(const unsigned int & width,const unsigned int & height);
	virtual ~PYXBitmap();

protected:
	unsigned int m_width;
	unsigned int m_height;
	boost::scoped_array<unsigned char> m_data;

public:
	const unsigned int & getWidth() const { return m_width; }
	const unsigned int & getHeight() const { return m_height; }
	const unsigned char * getData() const { return m_data.get(); }
	unsigned char * getData() { return m_data.get(); }
};

/*!

Interface Bitmap processes

a Bitmap process represent a single (or multiple) bitmaps.

the bitmap data is in RAW format of 24Bit-RGB or 32Bit-RGBA  

*/
//! Interface Bitmap processes
struct PYXLIB_DECL IBitmap : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Returns the number bitmaps provided by process
	virtual int getBitmapCount() = 0;
	
	//! Returns bitmap width in pixels
	virtual int STDMETHODCALLTYPE getWidth(int index = 0) = 0;

	//! Returns bitmap height in pixels
	virtual int STDMETHODCALLTYPE getHeight(int index = 0) = 0;

	//! Returns a pointer to the RAW bitmap
	virtual const PYXPointer<PYXBitmap> STDMETHODCALLTYPE getRawBitmap(int index = 0) = 0;
	
	//! Returns an XML with a defintion of the bitmap
	virtual std::string getBitmapDefinition(int index = 0) = 0;

};



#endif // guard
