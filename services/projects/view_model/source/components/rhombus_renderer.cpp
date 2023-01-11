/******************************************************************************
rhombus_renderer.cpp

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "rhombus_renderer.h"

// view model includes
#include "camera.h"
#include "view_open_gl_thread.h"
#include "gl_utils.h"
#include "exceptions.h"
#include "stile.h"
#include "vector_utils.h"
#include "garbage_collector.h"
#include "performance_counter.h"

#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/procs/cache.h"

#include <cassert>
#include <map>
#include <vector>


#define CACHE_VECTORS

////////////////////////////////////////////////////////////////////////////////
// RhombusMeshGenerator - helper class to generate the rhombus meshes
////////////////////////////////////////////////////////////////////////////////
class RhombusMeshGenerator
{
protected:
	unsigned short getVertexIndex(int u,int v,int lowerResolution = false) const
	{
		//Veritces are order in the following way:
		// 1) 0-99 - first 100 vertices are 10 by 10 grid.
		// 2) 100-115 - next 16 vertices are 4 by 4 grid of low resolution vertices
		if (lowerResolution && (u%9==0 || v%9==0))
		{
			//we are going to use the lower resolution vertices only for the border vertices.
			return 10*10+(v/3)*4+(u/3);
		}
		return v*10+u;
	}

	void addQuadToVector(int u0,int v0,int u1,int v1,std::vector<unsigned short> & indices)
	{	
		indices.push_back(getVertexIndex(u0,v0));
		indices.push_back(getVertexIndex(u0,v1));
		indices.push_back(getVertexIndex(u1,v0));

		indices.push_back(getVertexIndex(u0,v1));
		indices.push_back(getVertexIndex(u1,v1));
		indices.push_back(getVertexIndex(u1,v0));
	}

	void addPatialQuadToVector(int u0,int v0,int u1,int v1,int borderMask,std::vector<unsigned short> & indices)
	{
		//   +----------+  - creating 6 quads to mimic 1 resolutions offset   +--> v-Axis
		//   | \ 1__/  /|                                                     |
		//   |   +  2 / |                                                     |
		//   |4 /  \ /  |                                                     V  u-Axis
		//   | /5 __+ 3 |
		//   |/__/6   \ |
        //   +----------+

		//       8         - borderMask.
		//      +-+          if a border mask bit is on, then the trinagle that touch that border is been 
		//   1  | |  4       splited into 3 trinagles across that border to be nicely blend with neigbor
 		//      +-+          triangles
		//       2

		if ((borderMask & 8) != 0)
		{
			indices.push_back(getVertexIndex(u0,v0));
			indices.push_back(getVertexIndex(u0,v0+1));
			indices.push_back(getVertexIndex(u0+1,v0+1));

			indices.push_back(getVertexIndex(u0,v0+1));
			indices.push_back(getVertexIndex(u0,v1-1));
			indices.push_back(getVertexIndex(u0+1,v0+1));

			indices.push_back(getVertexIndex(u0,v1-1));
			indices.push_back(getVertexIndex(u0,v1));
			indices.push_back(getVertexIndex(u0+1,v0+1));
		}
		else
		{
			indices.push_back(getVertexIndex(u0,v0,(borderMask & 1) == 0));
			indices.push_back(getVertexIndex(u0,v1,(borderMask & 4) == 0));
			indices.push_back(getVertexIndex(u0+1,v0+1));
		}
		
		indices.push_back(getVertexIndex(u0,v1,(borderMask & (4|8)) == 0));
		indices.push_back(getVertexIndex(u1-1,v1-1));
		indices.push_back(getVertexIndex(u0+1,v0+1));

		if ((borderMask & 4) != 0)
		{
			indices.push_back(getVertexIndex(u0,v1));
			indices.push_back(getVertexIndex(u0+1,v1));
			indices.push_back(getVertexIndex(u1-1,v1-1));

			indices.push_back(getVertexIndex(u0+1,v1));
			indices.push_back(getVertexIndex(u1-1,v1));
			indices.push_back(getVertexIndex(u1-1,v1-1));

			indices.push_back(getVertexIndex(u1-1,v1));
			indices.push_back(getVertexIndex(u1,v1));
			indices.push_back(getVertexIndex(u1-1,v1-1));
		}
		else
		{
			indices.push_back(getVertexIndex(u0,v1,(borderMask & 8) == 0));
			indices.push_back(getVertexIndex(u1,v1,(borderMask & 2) == 0));
			indices.push_back(getVertexIndex(u1-1,v1-1));
		}

		if ((borderMask & 1) != 0)
		{
			indices.push_back(getVertexIndex(u0,v0));
			indices.push_back(getVertexIndex(u0+1,v0+1));
			indices.push_back(getVertexIndex(u0+1,v0));

			indices.push_back(getVertexIndex(u0+1,v0));
			indices.push_back(getVertexIndex(u0+1,v0+1));
			indices.push_back(getVertexIndex(u1-1,v0));

			indices.push_back(getVertexIndex(u1-1,v0));
			indices.push_back(getVertexIndex(u0+1,v0+1));
			indices.push_back(getVertexIndex(u1,v0));
		}
		else
		{
			indices.push_back(getVertexIndex(u0,v0,(borderMask & 8) == 0));
			indices.push_back(getVertexIndex(u0+1,v0+1));
			indices.push_back(getVertexIndex(u1,v0,(borderMask & 2) == 0));
		}

		indices.push_back(getVertexIndex(u0+1,v0+1));
		indices.push_back(getVertexIndex(u1-1,v1-1));
		indices.push_back(getVertexIndex(u1,v0,(borderMask & (1|2)) == 0));
		
		if ((borderMask & 2) != 0)
		{
			indices.push_back(getVertexIndex(u1-1,v1-1));
			indices.push_back(getVertexIndex(u1,v0+1));
			indices.push_back(getVertexIndex(u1,v0));

			indices.push_back(getVertexIndex(u1-1,v1-1));
			indices.push_back(getVertexIndex(u1,v1-1));
			indices.push_back(getVertexIndex(u1,v0+1));

			indices.push_back(getVertexIndex(u1-1,v1-1));
			indices.push_back(getVertexIndex(u1,v1));
			indices.push_back(getVertexIndex(u1,v1-1));
		}
		else
		{
			indices.push_back(getVertexIndex(u1-1,v1-1));
			indices.push_back(getVertexIndex(u1,v1,(borderMask & 4) == 0));
			indices.push_back(getVertexIndex(u1,v0,(borderMask & 1) == 0));
		}
	}

	void addFullQuatToVector(int u0,int v0,int u1,int v1,std::vector<unsigned short> & indices)
	{
		for(int u=u0;u<u1;u++)
		{
			for(int v=v0;v<v1;v++)
			{
				addQuadToVector(u,v,u+1,v+1,indices);
			}
		}
	}

	static int needTiles[3][3];

	bool needAFullQuad(int u,int v,int mask)
	{
		//out of border
		if (u<0||v<0||u>2||v>2)
		{
			return false;
		}
		return (needTiles[u][v] & mask) != 0;
	}

public:
	void generateMeshFromMask(int mask,std::vector<unsigned short> & indices)
	{
		for(int u=0;u<3;u++)
		{
			for(int v=0;v<3;v++)
			{
				if (needAFullQuad(u,v,mask))
				{
					addFullQuatToVector(u*3,v*3,(u+1)*3,(v+1)*3,indices);
				}
				else
				{
					addPatialQuadToVector(u*3,v*3,(u+1)*3,(v+1)*3,
						(int)(needAFullQuad(u,v-1,mask))*1 +
						(int)(needAFullQuad(u+1,v,mask))*2 +
						(int)(needAFullQuad(u,v+1,mask))*4 +
						(int)(needAFullQuad(u-1,v,mask))*8
						,indices);
				}
			}
		}
	}
};

int RhombusMeshGenerator::needTiles[3][3] = { { 1,   1|2,     2   } ,      //     1 --- 2   +-- U axis
											  { 1|8, 1|2|4|8, 2|4 } ,      //     |     |   |
											  {	  8,     4|8,   4 } };     //     8 --- 4   | V axis 

////////////////////////////////////////////////////////////////////////////////
// RhombusRenderer
////////////////////////////////////////////////////////////////////////////////

RhombusRenderer::RhombusRenderer(ViewOpenGLThread & viewThread) : 
	Component(viewThread),
	m_renderLines(false),
	m_usingShaders(false),
	m_powerTwoTextures(true),
	m_vertex_shader(OpenGLShader::knVertexShader),
	m_fragment_shader(OpenGLShader::knFragmentShader)
{
	m_blender = Blender::create(*this);
	getViewThread().getViewPortProcessChangeNotifier().attach(this,&RhombusRenderer::onViewPointProcessChange);
}

RhombusRenderer::~RhombusRenderer(void)
{
	m_blender.reset();
}

bool RhombusRenderer::initialize() 
{
	if (getViewThread().useNonPowerTwoTextures)
	{
		m_powerTwoTextures = ! OpenGLTexture::canHaveTextureWithNonPowerOfTwo();
	}

	m_supportTextureCombine = GLEE_ARB_texture_env_combine == GL_TRUE;
	m_supportMultiTexture   = OpenGLTexture::canSupportMultitextures();
	
	if (!m_supportMultiTexture)
	{
		//if we can't do multi texture - combine is out of the picture.
		m_supportTextureCombine = false;
	}

	m_coordUV = TextureCoordData::create();
	m_coordUV->generate();

	m_coordUV->vbo.setData(sizeof(m_coordUV->data),m_coordUV->data);

	RhombusMeshGenerator meshGenerator;
	for(int mask=0;mask<16;mask++)
	{
		std::vector<unsigned short> indcies;

		meshGenerator.generateMeshFromMask(mask,indcies);

		PYXPointer<OpenGLVBO> vbo = OpenGLVBO::create(OpenGLVBO::knElementArrayBuffer,OpenGLVBO::knStatic);

		vbo->setData(sizeof(unsigned short)*indcies.size(),&indcies[0]);

		m_indicesVBO.push_back(vbo);
		m_indicesCount.push_back(indcies.size());
	}

	m_pyxisGridTexture = OpenGLTexture::create(OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGB);

	if (m_powerTwoTextures)
	{
		m_pyxisGridTexture->setSize(128,128,OpenGLTexture::knDataRGB,OpenGLTexture::knTextelUnsignedShort);
		if (!this->loadTextureRegionFromResource(*m_pyxisGridTexture,"pyxis_grid.png",0,0,82,82))
		{
			return false;
		}
	}
	else
	{
		m_pyxisGridTexture->generateMipmap();

		if (!this->loadTextureFromResource(*m_pyxisGridTexture,"pyxis_grid.png"))
		{
			return false;
		}

		if (getViewThread().useShaders)
		{
			m_usingShaders = initalizeShaders();
		}
	}

	PYXPointer<RhombusRGBA> mask = this->loadRhombusBitmapFromResource("pyxis_grid.png");

	m_blender->setMask(mask);
	m_blender->setMaskAlpha(0); //don't show the grid

	TRACE_INFO("Rhombus Renderer using power two textures: " << m_powerTwoTextures);
	TRACE_INFO("Rhombus Renderer using shaders: " << m_usingShaders);

	m_pyxisGridTexture->setMinFilter(OpenGLTexture::knTextureMinNearestMipMapLinear);
	m_pyxisGridTexture->setMagFilter(OpenGLTexture::knTextureMagLinear);

	return m_pyxisGridTexture && m_coordUV;
}


bool RhombusRenderer::initalizeShaders()
{
	m_vertex_shader.setCode(
"uniform sampler2D cov_tex;\n"
"uniform sampler2D parent_cov_tex;\n"
"uniform sampler2D vec_tex;\n"
"uniform sampler2D parent_vec_tex;\n"
"uniform float vec_blend;\n"
"uniform float cov_blend;\n"
"uniform vec2 vec_u_transform;\n"
"uniform vec2 vec_v_transform;\n"
"uniform vec2 cov_u_transform;\n"
"uniform vec2 cov_v_transform;\n"
"uniform int hex_sample;\n"
"uniform int vec_hex_sample;\n"
"uniform int cov_hex_sample;\n"
//"varying vec3 ec_pos;\n"
"\n"
"void main()\n"
"{\n"
"   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"   gl_FrontColor = gl_Color;\n"
"   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"
//"   ec_pos = gl_Position.xyz;\n"
"}\n"
);

	if (! m_vertex_shader.compile())
	{
		TRACE_INFO( "failed to compile vertex_shader: " + m_vertex_shader.getCompileErrors());
		return false;
	}

	if (! m_vertex_shader.isValid())
	{
		return false;
	}

	m_fragment_shader.setCode(
"uniform sampler2D cov_tex;\n"
"uniform sampler2D parent_cov_tex;\n"
"uniform float cov_blend;\n"
"uniform vec2 cov_u_transform;\n"
"uniform vec2 cov_v_transform;\n"
"uniform int hex_sample;\n"
"uniform int cov_hex_sample;\n"
//"varying vec3 ec_pos;\n"
"\n"
"vec2 findNearestHexagonCoord(vec2 coord)\n"
"{\n"
"  if (coord[0]>coord[1])\n"
"  {\n"
"    if (coord[0]<0.5-coord[1]/2.0)\n"
"       return vec2(0.0,0.0);\n"
"    else if (coord[1]>1.0-coord[0]/2.0)\n"
"       return vec2(1.0,1.0);\n"
"    else\n"
"       return vec2(1.0,0.0);\n"
"  }\n"
"  else\n"
"  {\n"
"    if (coord[1]<0.5-coord[0]/2.0)\n"
"       return vec2(0.0,0.0);\n"
"    else if (coord[0]>1.0-coord[1]/2.0)\n"
"       return vec2(1.0,1.0);\n"
"    else\n"
"       return vec2(0.0,1.0);\n"
"  }\n"
"}\n"
"\n"
"vec2 hexagonTexCoord(vec2 coord)\n"
"{\n"
"   coord*=81.0;\n"
"   return (floor(coord)+findNearestHexagonCoord(fract(coord)))/81.0;\n"
"}\n"
"\n"
"vec4 sampleHexLinear(sampler2D tex,vec2 st)\n"
"{\n"
"	st*=81.0;\n"
"	vec2 stfloor = floor(st);\n"
"	vec2 uv = st-stfloor;\n"
"	st = stfloor+0.5;\n"
"\n"
"	vec2 u_step=vec2(1.0,0.0);\n"
"	vec2 v_step=vec2(0.0,1.0);\n"
"\n"
"	if (uv[0]+uv[1] < 1.0)\n"
"	{\n"
"		return texture2D(tex, st/82.0)*(1.0-uv[0]-uv[1])+texture2D(tex, (st+u_step)/82.0)*(uv[0])+texture2D(tex, (st+v_step)/82.0)*(uv[1]);\n"
"	}\n"
"	else\n"
"	{\n"
"		uv = 1.0-uv;\n"
"		return texture2D(tex, (st + u_step+v_step)/82.0)*(1.0-uv[0]-uv[1])+texture2D(tex, (st+u_step)/82.0)*(uv[1])+texture2D(tex, (st+v_step)/82.0)*(uv[0]);\n"
"	}\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec4 color;\n"
"    vec2 texCoords = gl_TexCoord[0].xy;\n"
"    vec2 parentCovCoords = gl_TexCoord[0].xy;\n"
"	 parentCovCoords.x = parentCovCoords.x*cov_u_transform[0]+cov_u_transform[1];\n"
"	 parentCovCoords.y = parentCovCoords.y*cov_v_transform[0]+cov_v_transform[1];\n"
"\n"
"    if (hex_sample!=0)\n"
"      texCoords = hexagonTexCoord(texCoords);\n"
"    if (cov_hex_sample!=0)\n"
"      parentCovCoords = hexagonTexCoord(parentCovCoords);\n"
"\n"
"    texCoords = (texCoords*81.0+0.5)/82.0;\n"
"    parentCovCoords = (parentCovCoords*81.0+0.5)/82.0;\n"
"\n"
"    color = gl_Color;\n"
"\n"
"    float blend = clamp(length(fwidth(texCoords))*45-1.2,0.0,1.0);\n"
"\n"
"    if (cov_blend==0.0)\n"
"    {\n"
"        color *= texture2D(cov_tex,texCoords);\n"
"    }\n"
"    else if (cov_blend==1.0)\n"
"    {\n"
"        color *= texture2D(parent_cov_tex,parentCovCoords);\n"
"    }\n"
"    else\n"
"    {\n"
"        float blend = clamp(length(fwidth(texCoords))*45-1.2,0.0,1.0);\n"
"        color *= mix(texture2D(cov_tex,texCoords),texture2D(parent_cov_tex,parentCovCoords),blend);\n"
"    }\n"
"\n"
//"    vec3 ec_normal = normalize(cross(dFdx(ec_pos), dFdy(ec_pos)));\n"
//"    float light = 0.75+0.5*dot(normalize(vec3(0,-1,1)),ec_normal);\n"
//"    gl_FragColor = color*light;\n"
"    gl_FragColor = color;\n"
"}\n"
);

	if (! m_fragment_shader.compile())
	{
		TRACE_INFO( "failed to compile fragment_shader: " + m_fragment_shader.getCompileErrors());
		return false;
	}

	if (! m_fragment_shader.isValid())
	{
		return false;
	}

	m_program.attach(m_vertex_shader);
	m_program.attach(m_fragment_shader);
	
	if (! m_program.link())
	{
		TRACE_INFO( "failed to link shader program: " + m_program.getLinkErrors());
		return false;
	}

	if (! m_program.isValid())
	{
		return false;
	}


	m_covTex = m_program.getUniformVariable("cov_tex");
	m_parentCovTex = m_program.getUniformVariable("parent_cov_tex");

	m_covBlend = m_program.getUniformVariable("cov_blend");
	m_lastCovBlend = -1.0f;

	m_hexSample  = m_program.getUniformVariable("hex_sample");
	m_lastHexSample = -1;
	m_covHexSample  = m_program.getUniformVariable("cov_hex_sample");
	m_lastCovHexSample = -1;

	m_covUTransform = m_program.getUniformVariable("cov_u_transform");
	m_covVTransform = m_program.getUniformVariable("cov_v_transform");

	return true;
}


void RhombusRenderer::render()
{		
	PerformanceCounter::getTimePerformanceCounter("Start RhombusRenderer",0.5f,0.5f,0.5f)->makeMeasurement();

	if (m_usingShaders)
	{
		renderWithShaderState();
	}
	else
	{
		renderWithNativeOpenGL();
	}

	PerformanceCounter::getTimePerformanceCounter("End RhombusRenderer",0.5f,0.5f,0.5f)->makeMeasurement();

	getViewThread().setFrameTimeMeasurement("render-terrain");
}


void RhombusRenderer::renderWithNativeOpenGL()
{
	int visiblePatchCount = 0;
	int readyPatchCount = 0;

	getViewThread().applyCamera();
	double altitudeFactor = 1 + getViewThread().getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius;

	// Use this to control local coordinate transformations
	glPushMatrix();
	vec3 localOrigin(0, 0, 0); // default local origin
	glTranslated(-0, -0, -altitudeFactor ); // default base origin

	// Use this matrix to rotate local origin
	mat4 m;
	cml::matrix_rotation_quaternion(m, getViewThread().getCamera().getOrbitalRotation());
	glMultMatrixd(m.data());

	PYXPointer<SurfaceMemento<VersionedMemento<Surface::Patch::VertexBuffer>>> elevations = getViewThread().getElevations();

	Surface::PatchVector & visible = m_surface->getVisiblePatches();
	Surface::PatchVector::iterator it = visible.begin();

	PerformanceCounter::getValuePerformanceCounter("Visible Rhombus",1.0f,1.0f,1.0f)->setMeasurement(visible.size());

	//set texture cordinates

	//raster texture:
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, m_coordUV->data);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	float blendColor[4] = {1,1,1,1};

	if (m_supportTextureCombine)
	{
		glActiveTexture(GL_TEXTURE1);
		glClientActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, m_coordUV->data);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);   //Interpolate RGB with RGB
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
		//GL_CONSTANT refers to the call we make with glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mycolor)
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
		//------------------------	
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);   //Interpolate ALPHA with ALPHA
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
		//GL_CONSTANT refers to the call we make with glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mycolor)
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		//half pixel text offset at the boundries would be removed
		if (m_powerTwoTextures)
		{
			glTranslated(0.5/RasterData::Spec::PowerTwoWidth,0.5/RasterData::Spec::PowerTwoHeight,0);
			glScaled((RasterData::Spec::Width-1.0)/RasterData::Spec::PowerTwoWidth,(RasterData::Spec::Height-1.0)/RasterData::Spec::PowerTwoHeight,0);
		}
		else
		{
			glTranslated(0.5/RasterData::Spec::Width,0.5/RasterData::Spec::Height,0);
			glScaled((RasterData::Spec::Width-1.0)/RasterData::Spec::Width,(RasterData::Spec::Height-1.0)/RasterData::Spec::Height,0);
		}

		glMatrixMode(GL_MODELVIEW);
	}

	bool vectorTextureEnabled = false;
	

	if (m_supportMultiTexture)
	{
		glActiveTexture(GL_TEXTURE2);
		glClientActiveTexture(GL_TEXTURE2);
		
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, m_coordUV->data);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

		glActiveTexture(GL_TEXTURE0);
		glClientActiveTexture(GL_TEXTURE0);
	}

	while(it != visible.end())
	{

		Surface::Patch & patch = (**it);
		int index = patch.getIndex(); //used for coloring the tiles

		PatchRenderState * state = 0;
		PatchRenderState * parentState = 0;
		OpenGLTexture * rasterData = 0;
		OpenGLTexture * parentRasterData = 0;
		Surface::Patch * parentRasterPatch = 0;

		PatchRenderStateMap::iterator textureIt = m_states.find(*it);

		if (textureIt != m_states.end())
		{
			state = textureIt->second.get();
			rasterData = state->getTexture().get();
			parentRasterData = state->getParentTexture().get();
			parentRasterPatch = state->getParentPatch().get();
		}
		else
		{
			PYXTHROW(PYXException,"RhombusRenderer internal state is corrupted");
		}

		if (parentRasterPatch == 0)
		{
			//we have no parent with data...

			//then, for future blending make easy. the current patch is "Parent" and the patch has nothing - so, just render the parent anyway
			parentRasterData = rasterData;
			parentRasterPatch = (*it).get();
			rasterData = 0;
		}

		if (rasterData != 0 && !m_supportTextureCombine)
		{
			//we can't support texture blending, the just display the rasterData...
			parentRasterData = rasterData;
			parentRasterPatch = (*it).get();
			rasterData = 0;
		}
		
		//validate elevation is loading
		elevations->get(*it)->validate();

		//borrow elevation if needed
		patch.borrowElevation();

		if (patch.getParentPtr() != nullptr)
		{
			//validate parent elevation...
			elevations->get(patch.getParent())->validate();

			//borrow lower elevation from parents matchs.
			if (patch.vertices && patch.getParentPtr()->vertices)
			{
				patch.vertices->borrowLowerResolutionVertices(patch.getKey().getLastIndex(),patch.getParentPtr()->vertices);
			}
		}

		//copy vertices for safe keeping
		if (state->getVertices() != patch.vertices)
		{
			state->setVertices(patch.vertices);
		}
		
		if (state->getVertices())
		{
			int mask = 0;

			for(int i=0;i<4;i++)
			{
				if ((*it)->hasFullVisibleVertex(i))
				{
					mask += 1<<i;
				}
			}

			m_indicesVBO[mask]->startUsing();

			visiblePatchCount++;
			
			if (rasterData != 0 && state->getTextureValid() && ! (*it)->isTooBig() && (*it)->elevations)
			{
				readyPatchCount++;
			}
			else
			{
				if ( (*it)->isTooBig() )
				{
					visiblePatchCount+=8;
				}
			}

			Surface::Patch::VertexBuffer * buffer = state->getVertices().get(); //dynamic_cast<VertexBuffer*>(patch.getVisualData(m_knVertexBufferName).get());

			if (buffer->zero != localOrigin)
			{
				// Local origin has changed so redo the transformations
				glPopMatrix();
				glPushMatrix();
				localOrigin = buffer->zero;
				vec3 v = localOrigin;
				v = cml::transform_vector(m, v);
				v -= vec3(0, 0, altitudeFactor );
				glTranslated(v[0], v[1], v[2]);
				glMultMatrixd(m.data());
			}


			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3,GL_FLOAT,0,buffer->vertices_floats);


			if (!m_renderLines)
			{
				glColor3f(1,1,1);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				if (parentRasterData != NULL)
				{
					parentRasterData->startUsing();
				}
				else
				{
					parentRasterPatch = patch.getParent().get();
					m_pyxisGridTexture->startUsing();
				}

				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();

				//half pixel text offset at the boundries would be removed
				if (m_powerTwoTextures)
				{
					glTranslated(0.5/RasterData::Spec::PowerTwoWidth,0.5/RasterData::Spec::PowerTwoHeight,0);
					glScaled((RasterData::Spec::Width-1.0)/RasterData::Spec::PowerTwoWidth,(RasterData::Spec::Height-1.0)/RasterData::Spec::PowerTwoHeight,0);
				}
				else
				{
					glTranslated(0.5/RasterData::Spec::Width,0.5/RasterData::Spec::Height,0);
					glScaled((RasterData::Spec::Width-1.0)/RasterData::Spec::Width,(RasterData::Spec::Height-1.0)/RasterData::Spec::Height,0);
				}

				if (parentRasterPatch != NULL)
				{
					//covert the current patch coordinates to the rasterPatch cordinates
					Surface::UVOffset uv_offset(patch.getKey(),parentRasterPatch->getKey());

					//translate parent
					glTranslated(uv_offset.getUOffset(),uv_offset.getVOffset(),0);
					glScaled(uv_offset.getUscale(),uv_offset.getVscale(),0);
				}

				glMatrixMode(GL_MODELVIEW);

				if (m_supportTextureCombine)
				{
					glActiveTexture(GL_TEXTURE1);
					if (rasterData != 0)
					{
						glEnable(GL_TEXTURE_2D);
						rasterData->startUsing();
						float blend = 0;

						if (patch.getSizeOnScreen() < 30)
						{
							blend = 1;
						}
						else if (patch.getSizeOnScreen() < 100)
						{
							blend = static_cast<float>(1-(patch.getSizeOnScreen()-30)/70);
						}
						
						blendColor[0] = blendColor[1] = blendColor[2] = blendColor[3] = blend;

						glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blendColor);
					}
					else
					{
						if (parentRasterData == 0 && patch.getParent())
						{
							//this is a special case when both the parent and the sone have no textures. therefore, we are bleding the pyxis_grid_textures

							glEnable(GL_TEXTURE_2D);
							m_pyxisGridTexture->startUsing();
							float blend = 0;

							if (patch.getSizeOnScreen() < 30)
							{
								blend = 1;
							}
							else if (patch.getSizeOnScreen() < 80)
							{
								blend = static_cast<float>(1-(patch.getSizeOnScreen()-30)/50);
							}
							
							blendColor[0] = blendColor[1] = blendColor[2] = blendColor[3] = blend;

							glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blendColor);
						}
						else
						{
							glDisable(GL_TEXTURE_2D);
						}
					}
				}

				if (!m_supportMultiTexture)
				{
					//we don't support multi texture - but we need to render both vector texture and raster texture.
					//So, we do it in two parts, first - we render the raster with polygon offest
					//Second, we render the vector texture with DECAL blending and without polygon offset
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1, 10);
				}
				
				glDrawElements(GL_TRIANGLES, m_indicesCount[mask], GL_UNSIGNED_SHORT, 0);

				if (!m_supportMultiTexture)
				{
					//disable - we are about to render vector texture
					glDisable(GL_POLYGON_OFFSET_FILL);
				}

				if (parentRasterData != NULL)
				{
					parentRasterData->stopUsing();
				}
				else
				{
					m_pyxisGridTexture->stopUsing();
				}
			}
			else 
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f((index%6+1) / 6.0f,(index%5+1) / 5.0f,(index%4+1) / 4.0f);

				glDrawElements(GL_TRIANGLES, m_indicesCount[mask], GL_UNSIGNED_SHORT, 0);

			}

			m_indicesVBO[mask]->stopUsing();

			glDisableClientState(GL_VERTEX_ARRAY);
		}

		++it;
	}

	//pop local origin transformation
	glPopMatrix();

	//reset vector texture settings
	glActiveTexture(GL_TEXTURE2);
	glClientActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//reset raster texture settings	
	glActiveTexture(GL_TEXTURE1);
	glClientActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (visiblePatchCount>0)
	{
		getViewThread().getView().setStreamingProgress("rhombus",(int)(readyPatchCount*100/visiblePatchCount));
	}
}

void RhombusRenderer::drawPatch(const PYXPointer<Surface::Patch> & patch)
{
	m_visiblePatches++;

	PatchRenderState * state = 0;
	PatchRenderState * parentState = 0;
	OpenGLTexture * rasterData = 0;
	OpenGLTexture * parentRasterData = 0;
	Surface::Patch * parentRasterPatch = 0;

	PatchRenderStateMap::iterator textureIt = m_states.find(patch);

	if (textureIt != m_states.end())
	{
		state = textureIt->second.get();
		rasterData = state->getTexture().get();
		parentRasterData = state->getParentTexture().get();
		parentRasterPatch = state->getParentPatch().get();
	}
	else
	{
		PYXTHROW(PYXException,"RhombusRenderer internal state is corrupted");
	}

	//validate elevation is loading
	getViewThread().getElevations()->get(patch)->validate();

	//borrow elevation if needed
	patch->borrowElevation();

	if (patch->getParentPtr() != nullptr)
	{
		//validate parent elevation...
		getViewThread().getElevations()->get(patch->getParent())->validate();
		
		//borrow lower elevation from parents patch.
		if (patch->vertices && patch->getParentPtr()->vertices)
		{
			patch->vertices->borrowLowerResolutionVertices(patch->getKey().getLastIndex(),patch->getParentPtr()->vertices);
		}
	}

	//copy vertices for safe keeping
	if (state->getVertices() != patch->vertices)
	{
		state->setVertices(patch->vertices);		
	}

	Surface::Patch::VertexBuffer * vertices = state->getVertices().get();

	if (vertices != 0)
	{
		if (state->getTextureValid() && ! patch->isTooBig() && patch->elevations )
		{
			m_loadedPatches++;
		}
		else if ( patch->isTooBig() )
		{
			//it shell be divided...
			m_visiblePatches+=8;
		}


		if (vertices->zero != m_localOrigin)
		{
			// Local origin has changed so redo the transformations
			glPopMatrix();
			glPushMatrix();
			m_localOrigin = vertices->zero;
			vec3 v = m_localOrigin;
			v = cml::transform_vector(m_localMatrix, v);
			v -= vec3(0, 0, m_altitudeFactor );
			glTranslated(v[0], v[1], v[2]);
			glMultMatrixd(m_localMatrix.data());
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		
		state->getVBO()->startUsing();
		glVertexPointer(3,GL_FLOAT,0,0);
		state->getVBO()->stopUsing();		
		//glVertexPointer(3,GL_FLOAT,0,vertices->vertices_floats);

		if (m_renderLines)
		{
			int index = patch->getIndex(); //used for coloring the tiles
			glColor3f((index%6+1) / 6.0f,(index%5+1) / 5.0f,(index%4+1) / 4.0f);

			
			int mask = 0;

			for(int i=0;i<4;i++)
			{
				if (patch->hasFullVisibleVertex(i))
				{
					mask += 1<<i;
				}
			}

			m_indicesVBO[mask]->startUsing();
			glDrawElements(GL_TRIANGLES, m_indicesCount[mask], GL_UNSIGNED_SHORT, 0);
			m_indicesVBO[mask]->stopUsing();
		}
		else
		{
			if (patch->getSizeOnScreen() > 200)
			{
				if (m_lastHexSample != 1) 
				{
					m_lastHexSample = 1;
					m_hexSample.set(1);
				}
			}
			else
			{
				if (m_lastHexSample != 0) 
				{
					m_lastHexSample = 0;
					m_hexSample.set(0);
				}
			}

			if (parentRasterPatch != 0 && parentRasterPatch->getSizeOnScreen() > 300)
			{
				if (m_lastCovHexSample != 1) 
				{
					m_lastCovHexSample = 1;
					m_covHexSample.set(1);
				}
			}
			else
			{
				if (m_lastCovHexSample != 0) 
				{
					m_lastCovHexSample = 0;
					m_covHexSample.set(0);
				}
			}

			auto blend = 0.0f;

			if (patch->getParentPtr() != 0)
			{
				if (rasterData == 0 && parentRasterData != 0)
				{
					blend = 1.0f;
				}
				else
				{
					blend = 0.5f;
				}
			}

			if (blend != m_lastCovBlend) {
				m_lastCovBlend = blend;
				m_covBlend.set(blend);
			}
			
			if (blend < 1.0f) {
				if (rasterData)
				{
					m_covTex.attachTexture(*rasterData);
				}
				else
				{
					m_covTex.attachTexture(*m_pyxisGridTexture);
				}
			}

			if (blend > 0.0f) {
				if (parentRasterData!=0)
				{
					m_parentCovTex.attachTexture(*parentRasterData);
				}
				else
				{
					m_parentCovTex.attachTexture(*m_pyxisGridTexture);
				}

				if (parentRasterPatch != 0)
				{
					//covert the current patch coordinates to the rasterPatch coordinates
					Surface::UVOffset uv_offset(patch->getKey(),parentRasterPatch->getKey());

					m_covUTransform.set(static_cast<float>(uv_offset.getUscale()),static_cast<float>(uv_offset.getUOffset()));
					m_covVTransform.set(static_cast<float>(uv_offset.getVscale()),static_cast<float>(uv_offset.getVOffset()));
				}
				else if (patch->getParentPtr() != 0)
				{
					//covert the current patch coordinates to the rasterPatch coordinates
					Surface::UVOffset uv_offset(patch->getKey(),patch->getParentPtr()->getKey());

					m_covUTransform.set(static_cast<float>(uv_offset.getUscale()),static_cast<float>(uv_offset.getUOffset()));
					m_covVTransform.set(static_cast<float>(uv_offset.getVscale()),static_cast<float>(uv_offset.getVOffset()));
				}
				else
				{
					m_covUTransform.set(1.0f,0.0f);
					m_covVTransform.set(1.0f,0.0f);
				}
			}

			int mask = 0;

			for(int i=0;i<4;i++)
			{
				if (patch->hasFullVisibleVertex(i))
				{
					mask += 1<<i;
				}
			}

			m_indicesVBO[mask]->startUsing();
			glDrawElements(GL_TRIANGLES, m_indicesCount[mask], GL_UNSIGNED_SHORT, 0);
			m_indicesVBO[mask]->stopUsing();
		}

		glDisableClientState(GL_VERTEX_ARRAY);
	}
}


void RhombusRenderer::renderWithShaderState()
{
	m_visiblePatches = 0;
	m_loadedPatches = 0;
	
	m_surface = getViewThread().getSurface();
	m_elevations = getViewThread().getElevations();

	getViewThread().applyCamera();
	m_altitudeFactor = 1 + getViewThread().getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius;

	// Use this to control local coordinate transformations
	glPushMatrix();
	m_localOrigin = vec3(0.0, 0.0, 0.0); // default local origin
	glTranslated(-0, -0, - m_altitudeFactor); // default base origin

	// Use this matrix to rotate local origin
	cml::matrix_rotation_quaternion(m_localMatrix, getViewThread().getCamera().getOrbitalRotation());
	glMultMatrixd(m_localMatrix.data());

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	m_coordUV->vbo.startUsing();
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	m_coordUV->vbo.stopUsing();	

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 5);

	if (m_renderLines)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		m_program.startUsing();

		glColor3f(1,1,1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_TEXTURE_2D);

		m_program.definePriorityTexture(*m_pyxisGridTexture);		
	}

	Surface::PatchVector & visiblePatches = m_surface->getVisiblePatches();

	for(Surface::PatchVector::iterator it = visiblePatches.begin();it != visiblePatches.end();++it)
	{
		drawPatch(*it);
	}

	if (!m_renderLines)
	{
		m_program.stopUsing();
	}

	m_indicesVBO[0]->stopUsing();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glPopMatrix();

	m_elevations.reset();

	PerformanceCounter::getValuePerformanceCounter("Visible Rhombus",1.0f,1.0f,1.0f)->setMeasurement(m_surface->getVisiblePatches().size());

	if (m_visiblePatches>0)
	{
		getViewThread().getView().setStreamingProgress("rhombus",(int)(m_loadedPatches*100/m_visiblePatches));		
	}	
}

/*! 
return the progress of loading a specific process identified by ProcRef. 
return number between 0 and 100 (fully loaded) or -1 if not information found about the given process.
*/
int RhombusRenderer::getProcessLoadingProgress(const ProcRef & procRef) const
{
	if (m_visiblePatches > 0)
	{
		auto loading = m_blender->getLoadingJobsForProcess(procRef);
		if (loading != -1)
		{
			return std::max(0, (int)((m_visiblePatches - loading) * 100 / (m_visiblePatches)));		
		}
	}
	return -1;
}

//called by the Blender blending thread when a new rhombus is read for display
void RhombusRenderer::onPatchLoaded(const PYXPointer<Surface::Patch> & patch,const PYXPointer<RhombusRGBAWithState> & layer)
{
	schedulePostProcessing(&RhombusRenderer::addTexture,patch,layer);
}

void RhombusRenderer::addTexture(PYXPointer<Surface::Patch> patch,PYXPointer<RhombusRGBAWithState> layer)
{
	Performance::ScopedTimer funcTime("RhombusRenderer::addTexture",0.10);
	PatchRenderStateMap::iterator it = m_states.find(patch);
	if (it != m_states.end())
	{
		if (!layer)
		{
			it->second->setTexture(nullptr, m_textureAllocator);
			it->second->setTextureValid(true);
			return;
		}

		if (layer && layer->getTag() != m_blender->getTag())
		{
			//this is layer not belong here anymore...
			//layer.reset();
			return;
		}

		it->second->setTexture(layer, m_textureAllocator);

		if (it->second->getTextureValid() && patch->isVisible() && patch->isDivided())
		{
			Surface::Visitor::visitVisibleChildren(patch,boost::bind(&RhombusRenderer::reloadUpdatedPatch,this,_1));
		}
	}
}

void RhombusRenderer::onViewPointProcessChange(PYXPointer<NotifierEvent> e)
{
	//the surface has been changed - reset everything...
	if (m_surface != getViewThread().getSurface())
	{
		TRACE_INFO("NEW SURFACE!!!!");

		if (m_surface)
		{
			m_surface->getPatchBecomeVisible().detach(this,&RhombusRenderer::onPatchBecomeVisible);
			m_surface->getPatchBecomeNotVisible().detach(this,&RhombusRenderer::onPatchBecomeHidden);
		}

		m_states.clear();
		m_blender->forgetAll();
		m_surface = getViewThread().getSurface();

		m_blender->setCoverages(getViewThread().getViewPointProcess()->QueryInterface<IViewPoint>());

		m_surface->getPatchBecomeVisible().attach(this,&RhombusRenderer::onPatchBecomeVisible);
		m_surface->getPatchBecomeNotVisible().attach(this,&RhombusRenderer::onPatchBecomeHidden);
	}
	else
	{
		m_blender->setCoverages(getViewThread().getViewPointProcess()->QueryInterface<IViewPoint>());
		invalidateAllRhombus();
	}
}

const PYXPointer<RhombusRenderer::PatchRenderState> & RhombusRenderer::generateState(const PYXPointer<Surface::Patch> & patch)
{
	PatchRenderStateMap::iterator it = m_states.find(patch);
	if (it == m_states.end())
	{
		it = m_states.insert(std::make_pair(patch,PatchRenderState::create(patch))).first;

		if (patch->getParent())
		{
			it->second->setParentState(generateState(patch->getParent()));
		}

	}
	return it->second;
}

void RhombusRenderer::loadPatch(const PYXPointer<Surface::Patch> & patch)
{
	PYXPointer<RhombusRenderer::PatchRenderState> state = generateState(patch);	
	state->setTextureValid(false);
	m_blender->loadPatch(patch);
}

void RhombusRenderer::onPatchBecomeVisible(PYXPointer<NotifierEvent> e)
{
	Surface::Event * surfaceEvent = dynamic_cast<Surface::Event*>(e.get());

	if (surfaceEvent)
	{
		loadPatch(surfaceEvent->getPatch());
	}
}

void RhombusRenderer::onPatchBecomeHidden(PYXPointer<NotifierEvent> e)
{
	Surface::Event * surfaceEvent = dynamic_cast<Surface::Event*>(e.get());

	if (surfaceEvent)
	{
		removePatchState(surfaceEvent->getPatch());
	}
}

void RhombusRenderer::removePatchState(const PYXPointer<Surface::Patch> & patch)
{
	m_blender->forgetPatch(patch);

	if (!patch->getKey().isPrimResolution())
	{
		auto stateIt = m_states.find(patch);
		if (stateIt != m_states.end()) {
			stateIt->second->setTexture(nullptr,m_textureAllocator);
			m_states.erase(stateIt);
		}
	}
}

void RhombusRenderer::setGridAlpha(int alpha)
{
	m_blender->setMaskAlpha(alpha);
	invalidateAllRhombus();
}

void RhombusRenderer::handleDataChangeEvent(PYXPointer<NotifierEvent> spEvent)
{
	PYXPointer<ProcessDataChangedEvent> processDataChangedEvent =
		boost::dynamic_pointer_cast<ProcessDataChangedEvent>(spEvent);

	PYXPointer<PYXGeometry> geom = processDataChangedEvent->getGeometry();
	PYXTile * tile = dynamic_cast<PYXTile*>(geom.get());
	if (tile != NULL)
	{
		schedulePostProcessing(&RhombusRenderer::invalidateUpdatedRhombus,tile->getRootIndex());
	}
	else 
	{
		schedulePostProcessing(&RhombusRenderer::invalidateAllRhombus);
	}
}

void RhombusRenderer::invalidateUpdatedRhombus(PYXIcosIndex index)
{
	Performance::ScopedTimer funcTime("RhombusRenderer::invalidateUpdatedRhombus",0.10);
	Surface::Visitor::visitVisible(*m_surface,boost::bind(&RhombusRenderer::loadUpdatedPatch,this,boost::ref(index),_1));
}

void RhombusRenderer::reloadUpdatedPatch(const PYXPointer<Surface::Patch> & patch)
{
	PYXPointer<RhombusRenderer::PatchRenderState> state = generateState(patch);
	if (state->getTexture() && !state->getTextureValid())
	{
		m_blender->forgetPatch(patch);
		m_blender->loadPatch(patch);
	}	
}

void RhombusRenderer::loadUpdatedPatch(const PYXIcosIndex & index, const PYXPointer<Surface::Patch> & patch)
{
	if (patch->intersects(index))
	{
		PYXPointer<RhombusRenderer::PatchRenderState> state = generateState(patch);
		state->setTextureValid(false);
		m_blender->forgetPatch(patch);
		m_blender->loadPatch(patch);
	}
}


void RhombusRenderer::invalidateAllRhombus()
{
	Performance::ScopedTimer funcTime("RhombusRenderer::invalidateAllRhombus",0.10);
	m_blender->forgetAll();
	Surface::Visitor::visitVisible(*m_surface,boost::bind(&RhombusRenderer::loadPatch,this,_1));
}

////////////////////////////////////////////////////////////////////////////////
// RhombusRenderer::Blender::RenderContext
////////////////////////////////////////////////////////////////////////////////

RhombusRenderer::Blender::RenderContext::RenderContext(const boost::intrusive_ptr<IProcess> & process) : m_resolutionOffset(0)
{
	boost::intrusive_ptr<ICoverage> coverage = process->getOutput()->QueryInterface<ICoverage>();

	m_procRef = ProcRef(process);
	m_processName = process->getProcName();

	if (coverage)
	{
		boost::intrusive_ptr<ICoverage> originalCoverage  = coverage;
		coverage = safeAddCacheToPipeline(process)->QueryInterface<ICoverage>();
		auto bandCount = coverage->getCoverageDefinition()->getFieldDefinition(0).getCount();
		if (bandCount == 1)
		{
			m_coverage = coverage;

			if (originalCoverage->getStyle("Coverage/ShowAsElevation").empty())
			{
				if(	coverage->getCoverageDefinition()->getFieldDefinition(0).getContext() == PYXFieldDefinition::knContextElevation ||
					coverage->getCoverageDefinition()->getFieldDefinition(0).getContext() == PYXFieldDefinition::knContextGreyScale)
				{
					m_contextType = knElevation;
				}
				else
				{
					m_contextType = knRGBCoverage;
				}
			}
			else if (StringUtils::fromString<bool>(originalCoverage->getStyle("Coverage/ShowAsElevation"))) 
			{
				m_contextType = knElevation;
			} 
			else 
			{
				m_contextType = knRGBCoverage;
			}

			std::string palette = originalCoverage->getStyle("Coverage/Palette");
			if (palette.size() == 0)
			{
				m_colorizer.reset(new RhombusBitmapColorizer::GrayScaleColorizer(-11000,9000));
			} 
			else
			{
				m_colorizer.reset(new RhombusBitmapColorizer::PaletteColorizer(palette));
			}
		}
		else if (bandCount == 3 )
		{
			m_coverage = coverage;
			m_colorizer.reset(new RhombusBitmapColorizer::RGBConstAlphaColorizer());
			m_contextType = knRGBCoverage;
		}
		else if ( bandCount >= 4 )
		{
			m_coverage = coverage;
			m_colorizer.reset(new RhombusBitmapColorizer::RGBWithAlphaColorizer());
			m_contextType = knRGBCoverage;
		}
		else
		{
			assert(0 && ("Unsupported number of bands:" + bandCount));
		}

		m_nativeResolution = m_coverage->getGeometry()->getCellResolution();
	}
	else
	{
		boost::intrusive_ptr<IFeatureCollection> featureCollection = process->getOutput()->QueryInterface<IFeatureCollection>();

		m_contextType = knStyledVectors;

		if (featureCollection)
		{
			boost::intrusive_ptr<ICoverage> coverage;

			// Wrap with rasterizer.
			boost::intrusive_ptr<IProcess> spRasterizerProc = PYXCOMCreateInstance<IProcess>(strToGuid("{E82F5DB8-3BF9-48b5-B529-64BB7819DD38}"));
			assert(spRasterizerProc);
			spRasterizerProc->getParameter(0)->addValue(process);
			std::map<std::string,std::string> attributes;
			attributes["UseAlpha"] = "1";
			spRasterizerProc->setAttributes(attributes);
			spRasterizerProc->initProc();

			assert(spRasterizerProc->getInitState() == IProcess::knInitialized && "rastersizer was unable to inialized");

			boost::intrusive_ptr<IFeatureCollection> featureCollection = process->getOutput()->QueryInterface<IFeatureCollection>();

			boost::intrusive_ptr<IFeatureGroup> featureGroup = process->getOutput()->QueryInterface<IFeatureGroup>();

			int count =0;

#ifdef CACHE_VECTORS
			if (featureGroup)
			{
				count = featureGroup->getFeaturesCount().middle();
			}
			else 
			{
				PYXPointer<FeatureIterator> fcIt = featureCollection->getIterator();
				while (count < 100 && ! fcIt->end())
				{
					count++;
					fcIt->next();
				}
			}
#endif

			//for small features... just create the texture when needed.
			if (count < 100)
			{
				//add a memory cache to this pipeline
				coverage = safeAddCacheToPipeline(spRasterizerProc,false)->getOutput()->QueryInterface<ICoverage>();
			}
			else
			{
				TRACE_INFO(process->getProcName() << " has more then 100 features, creating coverageCache to speed up the raster.");
				//add a persistence cache to this pipeline
				coverage = safeAddCacheToPipeline(spRasterizerProc,true)->getOutput()->QueryInterface<ICoverage>();				
			}

			//NOTE: enable m_resolutionOffset to make vector little bit larger
			//m_resolutionOffset = 1;
			m_coverage = coverage;
			m_colorizer.reset(new RhombusBitmapColorizer::RGBWithAlphaColorizer(200));
		}
	}

	boost::intrusive_ptr<IProcess> finalProcess = m_coverage->QueryInterface<IProcess>();
	finalProcess->getDataChanged().attach(this,&RhombusRenderer::Blender::RenderContext::dataChanged);
}

void RhombusRenderer::Blender::RenderContext::dataChanged(PYXPointer<NotifierEvent> spEvent)
{

	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (m_blender != NULL)
	{
		m_blender->dataChanged(spEvent);
	}
}

RhombusRenderer::Blender::RenderContext::~RenderContext()
{
	if (m_coverage)
	{
		boost::intrusive_ptr<IProcess> finalProcess = m_coverage->QueryInterface<IProcess>();
		finalProcess->getDataChanged().detach(this,&RhombusRenderer::Blender::RenderContext::dataChanged);
	}

	setBlender(NULL);
	m_loadingThread.stop();
}

PYXPointer<RhombusRGBA> RhombusRenderer::Blender::RenderContext::getRhombusBitmap(const PYXPointer<Surface::Patch> & patch,bool & valid)
{
	//make sure we don't do the same thing over and over again in the back thread...
	forgetPatch(patch);

	PYXPointer<RhombusRGBA> layer = generateRhombusBitmap(patch,true,valid);

	if (!layer || !valid)
	{
		//notify the back thread to load this patch
		m_loadingThread.addJob(patch,boost::bind(&RhombusRenderer::Blender::RenderContext::loadRhombusBitmap,this,patch));
	}

	return layer;
}

void RhombusRenderer::Blender::RenderContext::forgetPatch(const PYXPointer<Surface::Patch> & patch)
{
	m_loadingThread.cancelJobs(patch);
}

void RhombusRenderer::Blender::RenderContext::forgetAll()
{
	m_loadingThread.cancelAllJobs();
}

PYXPointer<Surface::Patch> RhombusRenderer::Blender::RenderContext::getPatchForLoading(const PYXPointer<Surface::Patch> & patch)
{
	PYXPointer<Surface::Patch> currentPatch = patch;

	for(int i=0;i<m_resolutionOffset;++i)
	{
		if(currentPatch->getParent())
		{
			currentPatch = currentPatch->getParent();
		}
	}

	return currentPatch;
}


void RhombusRenderer::Blender::RenderContext::loadRhombusBitmap(const PYXPointer<Surface::Patch> & patch)
{
	PerformanceCounter::getValuePerformanceCounter(m_processName,0,1,0)->setMeasurement(m_loadingThread.getWaitingJobsCount());

	RhombusRGBAFiller filler(m_processName,m_coverage,*m_colorizer);
	PYXPointer<RhombusRGBA> layer;
	PYXPointer<Surface::Patch> currentPatch = getPatchForLoading(patch);

	try
	{
		layer = filler.load(currentPatch->getRhombus());
	}
	catch (...)
	{
		TRACE_INFO("Failed to generate a bitmp for a rhombus from pipeline: " << m_processName);
		//if we weren't able to load the layer
	}

	if (layer && patch->isVisible())
	{
		//let the blending thread to re-blend
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (m_blender)
		{
			m_blender->loadPatch(patch);
		}
	}
}

PYXPointer<RhombusRGBA> RhombusRenderer::Blender::RenderContext::generateRhombusBitmap(const PYXPointer<Surface::Patch> & patch,bool fast,bool & valid)
{
	RhombusRGBAFiller filler(m_processName,m_coverage,*m_colorizer);
	PYXPointer<RhombusRGBA> layer;
	PYXPointer<Surface::Patch> currentPatch = getPatchForLoading(patch);

	valid = false;

	if (fast)
	{
		layer = filler.loadFast(currentPatch->getRhombus());
	}
	else
	{
		layer = filler.load(currentPatch->getRhombus());
	}

	if(layer)
	{
		valid = true;
		if (patch == currentPatch)
		{
			return layer;
		}
		else
		{
			return layer->zoomIn(patch->getKey(),currentPatch->getKey(),RhombusRGBA::knEvenResolution);
		}
	}

	currentPatch = currentPatch->getParent();

	while (!layer && currentPatch)
	{
		layer = filler.loadFast(currentPatch->getRhombus());
		if (!layer)
		{
			currentPatch = currentPatch->getParent();
		}
	}

	if (layer)
	{
		return layer->zoomIn(patch->getKey(),currentPatch->getKey(),RhombusRGBA::knEvenResolution);
	}

	return PYXPointer<RhombusRGBA>();
}

////////////////////////////////////////////////////////////////////////////////
// RhombusRenderer::Blender
////////////////////////////////////////////////////////////////////////////////

RhombusRenderer::Blender::Blender(RhombusRenderer & renderer) : m_renderer(renderer), m_blendingThread(2), m_tag(0)
{
}

RhombusRenderer::Blender::~Blender()
{
	for(RenderContextVector::iterator it = m_renderContexts.begin();it!= m_renderContexts.end();++it)
	{
		//detach it from the blender
		(*it)->setBlender(NULL);
		(*it)->forgetAll();

		//let the garbage collector to collect it...
		GarbageCollector::getInstance()->collect(*it);
	}
	m_blendingThread.stop();
}

void RhombusRenderer::Blender::setCoverages(const boost::intrusive_ptr<IViewPoint> & viewPoint)
{
	std::vector<boost::intrusive_ptr<IProcess>> pipelines(viewPoint->getCoveragePipelines());

	RenderContextMap renderContextsMap;
	RenderContextVector renderContexts;
	RenderContextVector elevationContexts;

	for(std::vector<boost::intrusive_ptr<IProcess>>::iterator it = pipelines.begin();it != pipelines.end();++it)
	{
		PYXPointer<RenderContext> context;
		if (m_renderContextsMap.find(*it) != m_renderContextsMap.end())
		{
			context = m_renderContextsMap[*it];
		}
		else
		{
			context = RenderContext::create((*it));
			context->setBlender(this);
		}

		if (context)
		{
			if (context->getContextType() == RenderContext::knElevation)
			{
				elevationContexts.push_back(context);
			}
			else
			{
				renderContexts.push_back(context);
			}

			TRACE_INFO(context->getName() << "context native resolution:" << context->getNativeResolution());

			renderContextsMap[*it] = context;
		}
	}

	//elevation contexts are behind the RGB coverages
	if (elevationContexts.size() > 0)
	{
		//TODO: order elevation by highest native resolution.
		renderContexts.insert(renderContexts.begin(),elevationContexts.begin(),elevationContexts.end());
	}

	pipelines = viewPoint->getFeatureCollectionPipelines();

	for(std::vector<boost::intrusive_ptr<IProcess>>::iterator it = pipelines.begin();it != pipelines.end();++it)
	{
		PYXPointer<RenderContext> context;
		if (m_renderContextsMap.find(*it) != m_renderContextsMap.end())
		{
			context = m_renderContextsMap[*it];
		}
		else
		{
			context = RenderContext::create((*it));
			context->setBlender(this);
		}

		if (context)
		{
			renderContexts.push_back(context);
			renderContextsMap[*it] = context;
		}
	}

	{
		boost::recursive_mutex::scoped_lock lock(m_coveragesMutex);
		std::swap(m_renderContexts,renderContexts);
		std::swap(m_renderContextsMap,renderContextsMap);
		m_tag++;
	}

	bool foundContextsToCollect = false;
	for(RenderContextVector::iterator it = renderContexts.begin();it != renderContexts.end();++it)
	{
		if (std::find(m_renderContexts.begin(),m_renderContexts.end(),*it) == m_renderContexts.end())
		{
			//we found a context no longer been used.
			foundContextsToCollect = true;

			//detach it from the blender
			(*it)->setBlender(NULL);
			(*it)->forgetAll();

			//let the garbage collector to collect it...
			GarbageCollector::getInstance()->collect(*it);
		}
	}

	if (foundContextsToCollect)
	{
		GarbageCollector::getInstance()->startDestroyObjects();
	}
}

void RhombusRenderer::Blender::forgetAll()
{
	for(auto & context : m_renderContexts)
	{
		context->forgetAll();
	}	
	m_blendingThread.cancelAllJobs();
}

void RhombusRenderer::Blender::forgetPatch(const PYXPointer<Surface::Patch> & patch)
{
	for(auto & context : m_renderContexts)
	{
		context->forgetPatch(patch);
	}
	m_blendingThread.cancelJobs(patch);
}

void RhombusRenderer::Blender::loadPatch(const PYXPointer<Surface::Patch> & patch)
{
	m_blendingThread.addJob(patch,boost::bind(&RhombusRenderer::Blender::blendPatch,this,patch));
}

//blending thread job
void RhombusRenderer::Blender::blendPatch(PYXPointer<Surface::Patch> patch)
{
	PerformanceCounter::getValuePerformanceCounter("blendPatch",0,1,0)->setMeasurement(m_blendingThread.getWaitingJobsCount());

	if (patch->isVisible())
	{
		PYXPointer<RhombusRGBAWithState> layer = generatePatch(patch,true);

		m_renderer.onPatchLoaded(patch,layer);
	}
}

//! return the number of waiting jobs for a specific process identified by ProcRef. or -1 if no render context found that process
int RhombusRenderer::Blender::getLoadingJobsForProcess(const ProcRef & procRef) const
{
	for(auto & context : m_renderContexts) 
	{
		if (context->getProcRef() == procRef) 
		{
			return context->getLoadingJobsCount();
		}	
	}
	return -1;
}


double RhombusRenderer::Blender::getBlendFactor(int covResolution,int visibleResolution) const
{
	//The weight function is 1/resolutionDifferent, and 1 if visiblieResolution == covResolution
	if (visibleResolution > covResolution)
	{
		return 1/(double)(visibleResolution-covResolution);
	}
	else if (visibleResolution < covResolution)
	{
		return 1/(double)(covResolution-visibleResolution);
	}
	return 1.0;
}

PYXPointer<RhombusRenderer::RhombusRGBAWithState> RhombusRenderer::Blender::generatePatch(const PYXPointer<Surface::Patch> & patch,bool fast)
{
	RenderContextVector contexts;
	int tag = 0;
	{
		boost::recursive_mutex::scoped_lock lock(m_coveragesMutex);
		contexts = m_renderContexts;
		tag = m_tag;
	}

	if (contexts.size() == 0)
	{
		return 0;
	}

	PYXPointer<RhombusRGBAWithState> finalResult = RhombusRGBAWithState::create(RhombusRGBA::knEvenResolution);
	finalResult->fillWith(0xcccccc,0xff);
	finalResult->setIsValid(true);
	finalResult->setTag(tag);

	int patchResolution = patch->getRhombus().getIndex(0).getResolution()+8;

	RhombusRGBABlender coverageBlender(RhombusRGBA::knEvenResolution);

	//Step1 : blend all coverages...
	for(RenderContextVector::iterator it = contexts.begin();it != contexts.end();++it)
	{
		RenderContext & context = **it;

		if(context.getContextType() == RenderContext::knRGBCoverage)
		{
			bool valid;
			PYXPointer<RhombusRGBA> layer = context.getRhombusBitmap(patch,valid);
			
			if (!layer || !valid)
			{
				finalResult->setIsValid(false);
			}

			if (layer && !layer->isAllTransparent())
			{
				double coverageBlendFactor = getBlendFactor(context.getNativeResolution(),patchResolution);							
				unsigned char alpha = static_cast<unsigned char>(coverageBlendFactor*255);

				coverageBlender.addRhombusRGBA(*layer,alpha);
			}
		}
	}

	PYXPointer<RhombusRGBA> blendedCoverages = RhombusRGBA::create(RhombusRGBA::knEvenResolution); 
	coverageBlender.toRhombusRGBA(*blendedCoverages);

	//Step 2: do elevation colorization...
	if (!blendedCoverages->isAllOpaque())
	{
		for(RenderContextVector::iterator it = contexts.begin();it != contexts.end();++it)
		{
			RenderContext & context = **it;

			if(context.getContextType() == RenderContext::knElevation)
			{
				bool valid;
				PYXPointer<RhombusRGBA> layer = context.getRhombusBitmap(patch,valid);
				
				if (!layer || !valid)
				{
					finalResult->setIsValid(false);
				}

				if (layer && !layer->isAllTransparent())
				{
					finalResult->overlay(*layer);
				}
			}
		}
	}

	//End of Step 2: overlay the coverages on top of the elevations
	finalResult->overlay(*blendedCoverages);

	//Step 3: overlay vectors:
	{
		for(RenderContextVector::iterator it = contexts.begin();it != contexts.end();++it)
		{
			RenderContext & context = **it;

			if(context.getContextType() == RenderContext::knStyledVectors)
			{
				bool valid;
				PYXPointer<RhombusRGBA> layer = context.getRhombusBitmap(patch,valid);

				if (!layer || !valid)
				{
					finalResult->setIsValid(false);
				}

				if (layer && !layer->isAllTransparent())
				{
					finalResult->overlay(*layer);
				}
			}
		}
	}

	if (m_maskAlpha>0)
	{
		if (finalResult->getBufferCount()==1)
		{
			finalResult->overlay(*m_maskBitmap,m_maskAlpha);
		} 
		else 
		{
			finalResult->overlay(*m_maskBitmapZommedIn,m_maskAlpha);
		}
	}

	return finalResult;
}

void RhombusRenderer::Blender::setMask(const PYXPointer<RhombusRGBA> & mask)
{
	m_maskBitmap = mask;
	m_maskBitmapZommedIn = m_maskBitmap->expandToOddResolution();
}

void RhombusRenderer::Blender::setMaskAlpha(int maskAlpha)
{
	m_maskAlpha = maskAlpha;
}

void RhombusRenderer::Blender::dataChanged(PYXPointer<NotifierEvent> spEvent)
{
	m_renderer.handleDataChangeEvent(spEvent);
}

////////////////////////////////////////////////////////////////////////////////
// RhombusRenderer::TextureAllocator
////////////////////////////////////////////////////////////////////////////////

PYXPointer<OpenGLTexture> RhombusRenderer::TextureAllocator::createTexture(const PYXPointer<RhombusRenderer::RhombusRGBAWithState> & texture)
{
	static bool s_powerTwoTextures = ! OpenGLTexture::canHaveTextureWithNonPowerOfTwo();

	PYXPointer<OpenGLTexture> openGlTexture;

	if (m_textures.empty())
	{
		openGlTexture = OpenGLTexture::create(OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGBA);

		if (s_powerTwoTextures)
		{
			openGlTexture->setSize(128,128,OpenGLTexture::knDataRGBA,OpenGLTexture::knTextelUnsignedByte);
		}
		else
		{
			openGlTexture->setSize(texture->width,texture->height,OpenGLTexture::knDataRGBA,OpenGLTexture::knTextelUnsignedByte);
		}
		openGlTexture->setMagFilter(OpenGLTexture::knTextureMagLinear);
		openGlTexture->setMinFilter(OpenGLTexture::knTextureMinLinear);
		openGlTexture->setWarp(OpenGLTexture::knTextureClampToEdge);
	} 
	else 
	{
		openGlTexture = m_textures.back();
		m_textures.pop_back();
	}	

	openGlTexture->setDataRegion(0,0,texture->width,texture->height,OpenGLTexture::knDataRGBA,OpenGLTexture::knTextelUnsignedByte,texture->getBuffer().rgba);

	return openGlTexture;
}

void RhombusRenderer::TextureAllocator::releaseTexture(PYXPointer<OpenGLTexture> & texture)
{
	if (texture && m_textures.size() < 100)
	{
		m_textures.push_back(texture);
	}
}

////////////////////////////////////////////////////////////////////////////////
// RhombusRenderer::PatchRenderState
////////////////////////////////////////////////////////////////////////////////

PYXPointer<RhombusRenderer::PatchRenderState> RhombusRenderer::PatchRenderState::s_emptyState = PYXNEW(RhombusRenderer::PatchRenderState,NULL);

RhombusRenderer::PatchRenderState::PatchRenderState(const PYXPointer<Surface::Patch> & patch) 
	: m_patch(patch),
	  m_parentState(s_emptyState),
	  m_textureValid(false)
{
}

void RhombusRenderer::PatchRenderState::setTexture(const PYXPointer<RhombusRenderer::RhombusRGBAWithState> & texture,TextureAllocator & allocator)
{
	static bool s_powerTwoTextures = ! OpenGLTexture::canHaveTextureWithNonPowerOfTwo();

	if (texture)
	{
		m_texture = allocator.createTexture(texture);		
		m_textureValid = texture->isValid();
	}
	else
	{
		if (m_texture) {
			allocator.releaseTexture(m_texture);
			m_texture.reset();
		}		
		m_textureValid = false;
	}
}

const PYXPointer<Surface::Patch> & RhombusRenderer::PatchRenderState::getParentPatch()
{
	if (!m_parentState->getPatch())
	{
		return m_parentState->getPatch();
	}
	if (m_parentState->getTexture())
	{
		return m_parentState->getPatch();
	}
	else
	{
		return m_parentState->getParentPatch();
	}
}

const PYXPointer<OpenGLTexture> & RhombusRenderer::PatchRenderState::getParentTexture()
{
	if (!m_parentState->getPatch())
	{
		return m_parentState->getTexture();
	}
	if (m_parentState->getTexture())
	{
		return m_parentState->getTexture();
	}
	else
	{
		return m_parentState->getParentTexture();
	}
}

void RhombusRenderer::PatchRenderState::setVertices(const PYXPointer<Surface::Patch::VertexBuffer> & vertices)
{
	m_vertices = vertices;

	if (!m_vbo) {
		m_vbo = OpenGLVBO::create(OpenGLVBO::knArrayBuffer,OpenGLVBO::knStatic);
	}

	m_vbo->setData(sizeof(m_vertices->vertices_floats),m_vertices->vertices_floats);
}

void RhombusRenderer::PatchRenderState::setParentState(const PYXPointer<RhombusRenderer::PatchRenderState> & state)
{
	m_parentState = state;
}