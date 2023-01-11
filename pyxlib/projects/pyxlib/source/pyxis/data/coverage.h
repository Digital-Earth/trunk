#ifndef PYXIS__DATA__COVERAGE_H
#define PYXIS__DATA__COVERAGE_H
/******************************************************************************
coverage.h

begin		: 2004-11-11
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/utility/cost.h"


// forward declarations
class PYXTile;
class PYXValueTile;

/*!
*/
//! A feature with values spread over its spatial domain.
struct PYXLIB_DECL ICoverage : public IFeatureCollection
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const = 0;

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() = 0;

	/*!
	Get the coverage value for the specified PYXIS index.

	\param	index		The PYXIS index.
	\param	nFieldIndex	The field index.

	\return	The value.
	*/
	//! Get the coverage value at the specified index.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const = 0;

	/*!
	Get the coverage values for the specified PYXIS tile.  By convention, the caller should never
	directly modify the PYXValueTile that is returned because other people may be using the
	same tile.  If you need to modify the contents of the tile, copy it first.

	\param	index		The root index of the tile.
	\param	nRes		The resolution of the tile.
	\param	nFieldIndex	The field index.

	\return	The values. May return null if no values.
	*/
	//! Get a tile of values at the index of depth nRes for the nFieldIndex channel of data.
	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const = 0;

	/*!
	Get estimated cost for generating a tile for a single field. The caller should call
	this function just before calling getFieldTile. 

	\param	index		The root index of the tile.
	\param	nRes		The resolution of the tile.
	\param	nFieldIndex	The field index.
	
	\return Cost for the action to complete.
	*/
	//! Get estimated cost for generating a tile for a single field.
	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(const PYXIcosIndex& index,
													   int nRes,
													   int nFieldIndex = 0	) const = 0;

	/*!
	Get coverage values for an entire tile (all fields). The caller should never
	directly modify the PYXValueTile that is returned because other people may be using the
	same tile.  If you need to modify the contents of the tile, copy it first.

	\param	tile	Defines requested tile (root index, depth).
	
	\return Shared pointer to value tile.
	*/
	//! Get coverage values for an entire tile for all channels of data.
	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const = 0;

	/*!
	Get estimated cost for generating an entire tile (all fields). The caller should call
	this function just before calling getCoverageTile. 

	\param	tile	Defines requested tile (root index, depth).
	
	\return Cost for the action to complete.
	*/
	//! Get estimated cost for generating an entire tile.
	virtual PYXCost STDMETHODCALLTYPE getTileCost(const PYXTile& tile) const = 0;

	/*!
	Set the coverage value at a specific PYXIS index. The default
	implementation does nothing.

	\param	value		The value
	\param	index		The PYXIS index
	\param	nFieldIndex	The field index.

	\return	Pointer to the field value (ownership retained) or 0 for no value.
	*/
	//! Set the field value at a specific PYXIS index.
	virtual void STDMETHODCALLTYPE setCoverageValue(	const PYXValue& value,	
														const PYXIcosIndex& index,
														int nFieldIndex = 0	) = 0;

	/*!
	Set coverage values for an entire tile (all fields).

	\param	spValueTile	Shared pointer to a PYXValueTile containing value data.
	*/
	//! Set coverage values for an entire tile for all channels of data.
	virtual void STDMETHODCALLTYPE setCoverageTile(PYXPointer<PYXValueTile> spValueTile) = 0;
};

#define ICOVERAGE_IMPL_PROXY(proxy) \
public: \
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const \
	{ \
		return (proxy).getCoverageDefinition(); \
	} \
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() \
	{ \
		return (proxy).getCoverageDefinition(); \
	} \
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,\
															int nFieldIndex = 0	) const \
	{ \
		return (proxy).getCoverageValue(index,nFieldIndex); \
	} \
	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index, \
																		int nRes, \
																		int nFieldIndex = 0	) const \
	{ \
		return (proxy).getFieldTile(index,nRes,nFieldIndex); \
	} \
	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(const PYXIcosIndex& index, \
													   int nRes, \
													   int nFieldIndex = 0	) const \
	{ \
		return (proxy).getFieldTileCost(index,nRes,nFieldIndex); \
	} \
	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const \
	{ \
		return (proxy).getCoverageTile(tile); \
	} \
	virtual PYXCost STDMETHODCALLTYPE getTileCost(const PYXTile& tile) const \
	{ \
		return (proxy).getTileCost(tile); \
	} \
	virtual void STDMETHODCALLTYPE setCoverageValue(	const PYXValue& value,\
														const PYXIcosIndex& index, \
														int nFieldIndex = 0	) \
	{ \
		(proxy).setCoverageValue(value,index,nFieldIndex); \
	} \
	virtual void STDMETHODCALLTYPE setCoverageTile(PYXPointer<PYXValueTile> spValueTile) \
	{ \
		(proxy).setCoverageTile(spValueTile); \
	} 

#endif // guard
