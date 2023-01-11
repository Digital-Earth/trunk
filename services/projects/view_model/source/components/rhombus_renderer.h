#ifndef VIEW_MODEL__RHOMBUS_RENDERER_H
#define VIEW_MODEL__RHOMBUS_RENDERER_H
/******************************************************************************
rhombus_renderer.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "open_gl_texture.h"
#include "open_gl_resource_pool.h"
#include "open_gl_shader.h"
#include "open_gl_program.h"
#include "surface_fillers.h"
#include "rhombus_bitmap.h"

#include "cml_utils.h"
#include "ray.h"


#include <map>
#include <list>
#include <vector>

/*!

RhombusRenderer - render rhombus on the screen - using raster, elevation and vector data

-- Description:
     - This renderer is strongly depend on Surface and SurfaceMemento of the ViewOpenGLThread class    

-- OpenGL extensions
	 - Multitexture
	 - Shaders

-- Limitations: 
     - None
	 
*/
//! RhombusRenderer - render rhombus on the screen - using raster, elevation and vector data
class RhombusRenderer : public Component
{
protected:

	class RhombusRGBAWithState : public RhombusRGBA
	{
	protected:
		bool m_isValid;
		int  m_tag;

	public:
		static PYXPointer<RhombusRGBAWithState> create(ResolutionType resolutionType) { return PYXNEW(RhombusRGBAWithState,resolutionType); }

		RhombusRGBAWithState(ResolutionType resolutionType) : RhombusRGBA(resolutionType), m_isValid(false), m_tag(0)
		{
		}

		//! get if the data on this RGBA is valid 
		bool isValid() const { return m_isValid; }

		//! get the tag number of this RGBA data - used by the Blender to check if a patch blong to the current ViewPoint state
		int getTag() const { return m_tag; }

		//! set if the data on this RGBA is valid - usually set by the RhombusRenderer when loading low resolutions rhombus to display high resolution rhombus
		void setIsValid(bool valid) { m_isValid = valid; }

		//! set the tag number of this RGBA data - used by the Blender to check if a patch blong to the current ViewPoint state
		void setTag(int tag) { m_tag = tag; }
	};

	/*!
	Blender - helper class to blending rhombus RGBA data.
	
	it can take RGBA from different pipelines and maybe different resolutions and
	generate the resulting RGBA data
	*/
	class Blender : public PYXObject
	{
	public:
		typedef WorkerThreadWithPriorities<Surface::Patch,Surface::Patch::PriorityCompare> WorkingThread;

		/*!
		RenderContext - helper class to generate RGBA data from a pipeline

		RenderContext is create from a pipeline. the render context is responsible
		to generate the right pipeline backend to generate RGBA data.

		This class know how to generate right backend for:
		1. coverages with/without alpha channel
		2. for elevations
		3. for features collection pipelines

		the backend will usually end with a cache (if there is none, a memory cache will be added)

		*/
		class RenderContext : public PYXObject
		{
		public:
			//! types of input pipeline
			enum ContextType
			{
				knElevation,
				knRGBCoverage,
				knStyledVectors
			};

		protected:
			//! process name, used for tracing and reporting errors
			std::string m_processName;

			//! process procRef, used for identify the context.
			ProcRef m_procRef;

			//! the pipeline back-end output coverage 
			boost::intrusive_ptr<ICoverage> m_coverage;

			//! the colorizer to use to convert the coverage data into RGBA
			boost::scoped_ptr<RhombusBitmapColorizer::IColorizer> m_colorizer;

			//! the native resolution - used for blending coverages
			int m_nativeResolution;

			//! the resolution offset for rendering, allow pipelines to be rendered at lower resolutions
			int m_resolutionOffset;

			//! the input pipeline type
			ContextType m_contextType;

			//! back-end working thread for generating tiles
			WorkingThread m_loadingThread;

			//! blender to report about loaded tiles
			Blender * m_blender;

			//! mutex for updating the m_blender attribute
			boost::recursive_mutex m_mutex;

		public:
			static PYXPointer<RenderContext> create(const boost::intrusive_ptr<IProcess> & process)
			{
				return PYXNEW(RenderContext,process);
			}

			RenderContext(const boost::intrusive_ptr<IProcess> & process);
			virtual ~RenderContext();

			void setBlender(Blender * blender)
			{
				boost::recursive_mutex::scoped_lock lock(m_mutex);
				m_blender = blender;
			}

			//! get the right patch to load from using the m_resolutionOffset
			PYXPointer<Surface::Patch> getPatchForLoading(const PYXPointer<Surface::Patch> & patch);

			//! try to generate a Rhombus RGBA if all data is ready (in cache)
			PYXPointer<RhombusRGBA> getRhombusBitmap(const PYXPointer<Surface::Patch> & patch,bool & valid);

			//! cancel a loading task in backend thread if exists
			void forgetPatch(const PYXPointer<Surface::Patch> & patch);

			//! cancel all loading tasks in backend thread
			void forgetAll();

			int getLoadingJobsCount() const { return m_loadingThread.getWaitingJobsCount(); }
			const ProcRef & getProcRef() const { return m_procRef; }
			const std::string & getName() const { return m_processName; }
			ContextType getContextType() const { return m_contextType; }
			int getNativeResolution() const { return m_nativeResolution; }

		protected:
			void loadRhombusBitmap(const PYXPointer<Surface::Patch> & patch);
			PYXPointer<RhombusRGBA> generateRhombusBitmap(const PYXPointer<Surface::Patch> & patch,bool fast,bool & valid); 

			void dataChanged(PYXPointer<NotifierEvent> spEvent);
		};

	protected:
		typedef std::vector<PYXPointer<RenderContext>> RenderContextVector;
		typedef std::map<boost::intrusive_ptr<IProcess>,PYXPointer<RenderContext>> RenderContextMap;

		//! cache of RenderContexts
		RenderContextMap	m_renderContextsMap;

		//! orderer list of the RenderContexts, from back most to front
		RenderContextVector m_renderContexts;

		//! mutex to protected m_renderContexts
		boost::recursive_mutex m_coveragesMutex;

		//! count changes on m_renderContexts
		int m_tag;

		RhombusRenderer & m_renderer;

		WorkingThread m_blendingThread;

		PYXPointer<RhombusRGBA> m_maskBitmap;
		PYXPointer<RhombusRGBA> m_maskBitmapZommedIn;
		int m_maskAlpha;


	public:
		Blender(RhombusRenderer & renderer);
		virtual ~Blender();

		static PYXPointer<Blender> create(RhombusRenderer & renderer)
		{
			return PYXNEW(Blender,renderer);
		};

		void setMask(const PYXPointer<RhombusRGBA> & mask);
		void setMaskAlpha(int maskAlpha);

	public:
		void forgetAll();
		void forgetPatch(const PYXPointer<Surface::Patch> & patch);
		void loadPatch(const PYXPointer<Surface::Patch> & patch);

		void setCoverages(const boost::intrusive_ptr<IViewPoint> & viewPoint);

		//! return the number of waiting jobs for blending.
		int getBlendingJobsCount() const { return m_blendingThread.getWaitingJobsCount(); }

		//! return the number of waiting jobs for a specific process identified by ProcRef. or -1 if no render context found that process
		int getLoadingJobsForProcess(const ProcRef & procRef) const;

		int getTag() { return m_tag; }

		void dataChanged(PYXPointer<NotifierEvent> spEvent);

	protected:
		//blending thread job
		void blendPatch(PYXPointer<Surface::Patch> patch);

	protected:
		double getBlendFactor(int covResolution,int visibleResolution) const;
		PYXPointer<RhombusRGBAWithState> generatePatch(const PYXPointer<Surface::Patch> & patch,bool fast);		
	};

	class TextureAllocator 
	{
	private:
		std::vector<PYXPointer<OpenGLTexture>> m_textures;

	public:
		PYXPointer<OpenGLTexture> createTexture(const PYXPointer<RhombusRenderer::RhombusRGBAWithState> & texture);
		void releaseTexture(PYXPointer<OpenGLTexture> & texture);
	};

	class PatchRenderState : public PYXObject, ObjectMemoryUsageCounter<PatchRenderState>
	{
	private:
		PYXPointer<Surface::Patch> m_patch;
		PYXPointer<PatchRenderState> m_parentState;
		PYXPointer<OpenGLTexture> m_texture;
		bool m_textureValid;
		PYXPointer<Surface::Patch::VertexBuffer> m_vertices;
		PYXPointer<OpenGLVBO> m_vbo;

		static PYXPointer<PatchRenderState> s_emptyState;

	public:
		static PYXPointer<PatchRenderState> create(const PYXPointer<Surface::Patch> & patch)
		{
			return PYXNEW(PatchRenderState,patch);
		}

		PatchRenderState (const PYXPointer<Surface::Patch> & patch);
		
		const PYXPointer<Surface::Patch> & getPatch() { return m_patch; }
		const PYXPointer<OpenGLTexture> & getTexture() { return m_texture; }
		bool getTextureValid() { return m_textureValid; }
		const PYXPointer<Surface::Patch> & getParentPatch();
		const PYXPointer<OpenGLTexture> & getParentTexture();
		const PYXPointer<Surface::Patch::VertexBuffer> & getVertices() { return m_vertices; }
		const PYXPointer<OpenGLVBO> & getVBO() { return m_vbo; }

		void setTexture(const PYXPointer<RhombusRGBAWithState> & texture,TextureAllocator & allocator);
		void setTextureValid(bool valid) { m_textureValid = valid; }
		void setVertices(const PYXPointer<Surface::Patch::VertexBuffer> & vertices);
		void setParentState(const PYXPointer<PatchRenderState> & state);
	};

public:
	static PYXPointer<RhombusRenderer> create(ViewOpenGLThread & viewThread)
	{
		return PYXNEW(RhombusRenderer,viewThread);
	}

	RhombusRenderer(ViewOpenGLThread & viewThread);
	virtual ~RhombusRenderer(void);

	virtual bool initialize();
	virtual void render();

public:
	const PYXPointer<PatchRenderState> & generateState(const PYXPointer<Surface::Patch> & patch);

	void loadPatch(const PYXPointer<Surface::Patch> & patch);
	void reloadUpdatedPatch(const PYXPointer<Surface::Patch> & patch);
	void loadUpdatedPatch(const PYXIcosIndex & index, const PYXPointer<Surface::Patch> & patch);

	void onPatchLoaded(const PYXPointer<Surface::Patch> & patch,const PYXPointer<RhombusRGBAWithState> & layer);
	
private:
	void onViewPointProcessChange(PYXPointer<NotifierEvent> e);
	void onPatchBecomeVisible(PYXPointer<NotifierEvent> e);
	void onPatchBecomeHidden(PYXPointer<NotifierEvent> e);	

	void removePatchState(const PYXPointer<Surface::Patch> & patch);

	void addTexture(PYXPointer<Surface::Patch> patch,PYXPointer<RhombusRGBAWithState> layer);

	void invalidateUpdatedRhombus(PYXIcosIndex index);
	void invalidateAllRhombus();

private:

	//! the current ProcRef that we are rendering
	ProcRef	m_currentProcRef;
	
	PYXPointer<Blender> m_blender;

	typedef std::map<PYXPointer<Surface::Patch>,PYXPointer<PatchRenderState>> PatchRenderStateMap;

	PatchRenderStateMap m_states;

	TextureAllocator m_textureAllocator;

	PYXPointer<Surface>	m_surface;
	bool				m_renderLines;

	//! indicates if the GPU support shaders and the user allow us to use them.
	bool				m_usingShaders;

	// if shader not supported, we check for the following OpenGL extenations, and use them if available
	bool				m_powerTwoTextures; // create textures with size that is not power of two
	bool				m_supportTextureCombine; // to be able combine two textures with a given param
	bool				m_supportMultiTexture; // to support multi textures

	bool initalizeShaders();

	void renderWithNativeOpenGL();
	void renderWithShader();
	void renderWithShaderState();

	OpenGLShader m_vertex_shader;
	OpenGLShader m_fragment_shader;
	OpenGLProgram m_program;

	OpenGLUniformVariable m_covTex;
	OpenGLUniformVariable m_parentCovTex;

	float m_lastCovBlend;
	OpenGLUniformVariable m_covBlend;

	int m_lastHexSample;
	int m_lastCovHexSample;
	OpenGLUniformVariable m_hexSample;
	OpenGLUniformVariable m_covHexSample;
	
	OpenGLUniformVariable m_covUTransform;
	OpenGLUniformVariable m_covVTransform;

	PYXPointer<TextureCoordData> m_coordUV;
	PYXPointer<OpenGLTexture> m_pyxisGridTexture;

	std::vector<PYXPointer<OpenGLVBO>> m_indicesVBO;
	std::vector<int> m_indicesCount;

public:
	bool getRenderLinesOnly() const { return m_renderLines; }
	void setRenderLinesOnly(bool value) { m_renderLines = value; }

	void setGridAlpha(int alpha);

	void handleDataChangeEvent(PYXPointer<NotifierEvent> spEvent);

	/*! 
	return the progress of loading a specific process identified by ProcRef. 
	return number between 0 and 100 (fully loaded) or -1 if not information found about the given process.
	*/
	int  getProcessLoadingProgress(const ProcRef & procRef) const;

protected:

	double m_altitudeFactor;
	vec3 m_localOrigin;
	mat4 m_localMatrix;

	bool m_gotAllElevations;
	int  m_visiblePatches;
	int  m_loadedPatches;

	PYXPointer<SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>> m_elevations;
	void drawPatch(const PYXPointer<Surface::Patch> & state);
};


#endif