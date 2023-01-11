#ifndef FEATURE_FIELD_RASTERIZER_H
#define FEATURE_FIELD__RASTERIZER_H
/******************************************************************************
feature_field_rasterizer.h

begin		: 2011-11-17
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/region/region.h"

/*!
This process converts a feature collection input into a rasterized coverage.
each cell of the coverage will get the min/max value specificed field of all features intersect that cell.
plus, a buffer can be applied on the features (in metters)
*/
//! Styled feature rasterizer coverage process.
class MODULE_ANALYSIS_PROCS_DECL FeatureFieldRasterizer : public ProcessImpl<FeatureFieldRasterizer>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeatureFieldRasterizer();

	//! Destructor
	~FeatureFieldRasterizer();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(		const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex = 0	) const;

public:
	static void test();

private:
	virtual void createGeometry() const;

private:

	class RasterState : public PYXObject
	{
	protected:
		PYXPointer<PYXTableDefinition> m_definition;
		boost::intrusive_ptr<IFeatureCollection> m_spFC;
		ProcRef m_procRef;

	protected:
		int		 m_fieldIndex;
		bool     m_max;
		double   m_buffer;

	protected:
		void loadStyleParameters();

	public:
		RasterState(const boost::intrusive_ptr<IFeatureCollection> & spFC,
					const ProcRef & procRef,
					const std::string & fieldName,
					const std::string & aggregate,
					double buffer);

		static PYXPointer<RasterState> create(
					const boost::intrusive_ptr<IFeatureCollection> & spFC,
					const ProcRef & procRef,
					const std::string & fieldName,
					const std::string & aggregate,
					double buffer)
		{
			return PYXNEW(RasterState,spFC,procRef,fieldName,aggregate,buffer);
		}

	public:
		PYXPointer<PYXTableDefinition> getOutputDefinition();

		boost::intrusive_ptr<IFeatureCollection> getFeatureCollection() { return m_spFC; }

		PYXPointer<const PYXTableDefinition> getDefinition() { return m_definition; }

		static const int knFieldCount = -2;
		static const int knIntesection = -1;

		PYXValue aggregate(const PYXValue & a,const PYXValue & b) const;
		PYXValue getValue(const PYXPointer<IFeature> & feature) const;

		double getBuffer() const { return m_buffer; } //in radians

		bool isFeatureCount() const { return m_fieldIndex == knFieldCount; }
	};

	class RasterContext
	{
	protected:
		PYXPointer<RasterState> m_state;
		PYXIcosIndex m_rootIndex;
		int m_nResolution;

		struct Location
		{
			PYXIcosIndex index;
			PYXCoord3DDouble location;
		};
		boost::scoped_array<Location> m_locationCache;

		double   m_lineWidth;

		//! errorThreshold to speed up rastering is 20% of a the resolution cells radius
		double   m_errorThreshold;

		int m_getDistanceCount;
		int m_getDistanceOptimizedCount;
		int m_makeOptimizedCount;
		int m_getPointContainedCount;
		int m_setValueCount;

		PYXPointer<PYXValueTile> m_resultValueTile;

	public:
		RasterContext(const PYXPointer<RasterState> & state,
					  const PYXIcosIndex & rootIndex,
					  int resolution);

	public:
		void raster();

		const Location & getLocation(int nPos);

		PYXPointer<PYXValueTile> getResultValueTile();

	protected:
		void rasterizeRegion(const PYXPointer<PYXVectorRegion> & region, const PYXValue & value);

		void rasterizeGeometry(const PYXGeometry & geom, const PYXValue & value);
	};

private:
	mutable boost::intrusive_ptr<IFeatureCollection> m_spFC;

	PYXPointer<RasterState> m_state;

	std::string m_fieldName;

	std::string m_aggregate;

	double m_buffer; //in metters
};

#endif // guard
