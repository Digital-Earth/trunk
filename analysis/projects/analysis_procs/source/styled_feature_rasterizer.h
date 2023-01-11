#ifndef STYLED_FEATURE_RASTERIZER_H
#define STYLED_FEATURE_RASTERIZER_H
/******************************************************************************
styled_feature_rasterizer.h

begin		: 2010-11-17
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/region/region.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/utility/color_palette.h"
#include "pyxis/utility/thread_pool.h"

/*!
This process converts a feature collection input into a rasterized coverage.
Each cell in the coverage will contain an RGBA color with opacity based on the input FeatureCollection style and Feautre style.
*/
//! Styled feature rasterizer coverage process.
class MODULE_ANALYSIS_PROCS_DECL StyledFeatureRasterizer : public ProcessImpl<StyledFeatureRasterizer>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	StyledFeatureRasterizer();

	//! Destructor
	~StyledFeatureRasterizer();

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

	virtual std::string STDMETHODCALLTYPE StyledFeatureRasterizer::getAttributeSchema() const;

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
	private:
		PYXPointer<const PYXTableDefinition> m_definition;
		boost::intrusive_ptr<IFeatureCollection> m_spFC;
		ProcRef m_procRef;

	private:
		PYXValue m_fillColor;
		bool	 m_hasFillColor;
		PYXValue m_lineColor;
		bool     m_hasLineColor;
		int      m_fillOpacity;
		int		 m_lineOpacity;
		double   m_lineWidth;

		std::string m_paletteString;
		std::string m_paletteField;
		int m_paletteFieldIndex;
		PYXPointer<PYXValueColorPalette> m_palette;

	private:
		void loadStyleParameters();

	public:
		RasterState(const boost::intrusive_ptr<IFeatureCollection> & spFC,
					const ProcRef & procRef,
					const PYXPointer<const PYXTableDefinition> & definition);

		static PYXPointer<RasterState> create(
					const boost::intrusive_ptr<IFeatureCollection> & spFC,
					const ProcRef & procRef,
					const PYXPointer<const PYXTableDefinition> & definition)
		{
			return PYXNEW(RasterState,spFC,procRef,definition);
		}

	private:
		//get a color from the xml style, or generate a random color if now color was specified
		PYXValue getColor(const std::string & styleNode);

		//generate a random color based on the input procRef
		PYXValue createRandomColor() const;

		//get the line width from the style xml
		double getLineWidth(const std::string & styleNode) const;

		//get the fill transpareny from the style xml
		int getFillOpacity(const std::string & styleNode) const;

		//add an alpha change to a value.
		void addAlphaChannel(PYXValue & value,int alphaValue);

	public:
		boost::intrusive_ptr<IFeatureCollection> getFeatureCollection() { return m_spFC; }

		PYXPointer<const PYXTableDefinition> getDefinition() { return m_definition; }

		//check if the definition contains alpha channel as well.
		bool hasAlphaChannel();

		const PYXValue & getFillColor() const { return m_fillColor; }

		const PYXValue & getLineColor() const { return m_lineColor; }

		double getLineWidth() const { return m_lineWidth; }

		bool hasFillColor() const { return m_hasFillColor; }

		bool hasLineColor() const { return m_hasLineColor; }

		bool doesStyleRequireToDrilIntoGroup(const PYXPointer<IFeatureGroup> & group) const;
		void getStyleForFeature(const PYXPointer<IFeature> & feature,const PYXPointer<const PYXGeometry> & geom,int resolution,unsigned char lineColor[4],unsigned char fillColor[4],bool & hasFillColor);
	};

	typedef std::vector<PYXPointer<RasterState>> RasterStateVector;

	class RasterContext
	{
	private:
		class SinglePipelineContext
		{
		private:
			PYXPointer<RasterState> m_state;
			RasterContext & m_context;

		public:
			SinglePipelineContext(RasterContext & context,const PYXPointer<RasterState> & state);

		public:
			void raster();

		private:
			void rasterizeFeature(PYXPointer<IFeature> spF);

			void rasterizeGroup(PYXPointer<IFeatureGroup> group);

			void rasterizeRegion(const PYXPointer<PYXVectorRegion> & region,unsigned char lineColor[4],unsigned char fillColor[4],bool useFillColor);

			void rasterizeGeometry(const PYXGeometry & geom,unsigned char color[4]);

			void rasterizeVectorGeometry(const PYXVectorGeometry2 & geom,unsigned char lineColor[4],unsigned char fillColor[4],bool useFillColor);
		};

	private:
		boost::shared_ptr<RasterStateVector> m_states;

		PYXIcosIndex m_rootIndex;
		int m_nResolution;

		struct ColorState
		{
			int colorSetCount;
			int color [4];

			ColorState() : colorSetCount(0)
			{
				color[0]=color[1]=color[2]=color[3]=0;
			}
		};
		boost::recursive_mutex m_cacheMutex[16];
		boost::scoped_array<ColorState> m_colorCache;

		PYXPointer<PYXVectorGeometry> m_tileVectorGeom;
		PYXBoundingCircle m_tileBoundCircle;

		PYXTaskGroup m_rasterTasks;

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
		RasterContext(	const boost::shared_ptr<RasterStateVector> & states,
						const PYXIcosIndex & rootIndex,
						int resolution);

	public:
		void raster();

		void fillCell(int nPos,unsigned char color[4]);
		void fillCells(int nPos,int nLength,unsigned char color[4]);

		PYXPointer<PYXValueTile> getResultValueTile();
		
	};

private:
	std::vector<boost::intrusive_ptr<IFeatureCollection>> m_FCs;

	boost::shared_ptr<RasterStateVector> m_state;

	bool m_useAlpha;
};

#endif // guard
