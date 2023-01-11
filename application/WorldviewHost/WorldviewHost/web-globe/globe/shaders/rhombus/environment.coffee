_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
FragmentState = require('../jit/fragment-state')
Color = require 'color'
SharedUniforms = require('../jit/shared-uniforms')



fragmentDefinitions = '''
// 	 matcap converts cubemap style normal to a 2d spherical reflection matcap
vec2 matcap_1_0(vec3 eye, vec3 normal) {
  vec3 reflected = reflect(eye, normal);

  float m = 2.0 * sqrt(
    pow(reflected.x, 2.0) +
    pow(reflected.y, 2.0) +
    pow(reflected.z + 1.0, 2.0)
  );

  return reflected.xy / m + 0.5;
}

vec2 multiFrame(vec2 baseUV, float frameIndex, float frameSize)
{
	return baseUV * vec2(frameSize, 1.0) + vec2(frameIndex*frameSize, 0.0);
}
//frameSize, frame0, frame1, blendValue
vec3 sampleMatcap(sampler2D matCap, vec2 baseUV, vec4 frameData)
{
	float frameSize = frameData.x;
	float frameIndex0 = frameData.y;
	float frameIndex1 = frameData.z;
	float frameBlend = frameData.w;

	if (frameBlend <= 0.0)
		return texture2D(matCap, multiFrame(baseUV, frameIndex0, frameSize)).rgb;
	else if (frameBlend >= 1.0)
		return texture2D(matCap, multiFrame(baseUV, frameIndex1, frameSize)).rgb;
	return mix(texture2D(matCap, multiFrame(baseUV, frameIndex0, frameSize)).rgb,
		texture2D(matCap, multiFrame(baseUV, frameIndex1, frameSize)).rgb,
		frameBlend);
}

''' 

class Environment extends Jit.JitModule
	constructor: () -> super("environment")

	## TODO: this implementation require uniform vUv 
	compile: (shader) ->
		self = this

		state = shader.require('FragmentState')
		theme = shader.theme # TODO :: via require

		compiledModule = new Jit.JitCompiledModule(this, shader)

		###
			tRoughMatCap: { type: 't', value: window.GC.assets.roughMatcap }, 
			tGlossMatCap: { type: 't', value: window.GC.assets.glossMatcap },
			tNoiseMap: { type: 't', value: window.GC.assets.noiseTexture }, 
		###
		emptyTexture = shader.require('EmptyTexture').texture

		# these keys correspond to theme values
		assetUniforms = {}
		assetUniforms['roughMatcap'] =  compiledModule.uniform("tRoughMatCap","t", emptyTexture)
		assetUniforms['glossMatcap'] = compiledModule.uniform("tGlossMatCap","t", emptyTexture)
		assetUniforms['noiseMap'] = compiledModule.uniform("tNoiseMap","t", emptyTexture) 

		assetUniforms['environment_and_noise_multiplier'] = compiledModule.uniform("environment_and_noise_multiplier","v2", 
			new THREE.Vector2(theme.environmentMultiplier, theme.noiseMultiplier), shared:true )

		assetUniforms['basemapRoughness_and_LightingMultiplier'] = compiledModule.uniform("basemapRoughness_and_LightingMultiplier","v2", new THREE.Vector2(theme.basemapRoughness, theme.basemapLightingMultiplier), shared:true )
		assetUniforms['surfaceRoughness_and_LightingMultiplier'] = compiledModule.uniform("surfaceRoughness_and_LightingMultiplier","v2", new THREE.Vector2(theme.surfaceRoughness, theme.surfaceLightingMultiplier), shared:true )

		roughMatcapFrameData = new THREE.Vector4()
		theme.initRoughMatCapFrameData(roughMatcapFrameData)
		assetUniforms['roughMatcapFrameData'] = compiledModule.uniform("roughMatcapFrameData", "v4", roughMatcapFrameData, shared:true )
		glossMatcapFrameData = new THREE.Vector4()
		theme.initGlossMatCapFrameData(glossMatcapFrameData)
		assetUniforms['glossMatcapFrameData'] = compiledModule.uniform("glossMatcapFrameData", "v4", glossMatcapFrameData, shared:true )
	

		# TODO :: move these out of environment
		assetUniforms['layer0Tint'] = compiledModule.uniform("layer0Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer0Tint, theme.layer0Alpha), true) 
		assetUniforms['layer1Tint'] = compiledModule.uniform("layer1Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer1Tint, theme.layer1Alpha), true) 
		assetUniforms['layer2Tint'] = compiledModule.uniform("layer2Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer2Tint, theme.layer2Alpha), true) 
		assetUniforms['layer3Tint'] = compiledModule.uniform("layer3Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer3Tint, theme.layer3Alpha), true) 
		assetUniforms['layer4Tint'] = compiledModule.uniform("layer4Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer4Tint, theme.layer4Alpha), true) 
		assetUniforms['layer5Tint'] = compiledModule.uniform("layer5Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer5Tint, theme.layer5Alpha), true) 
		assetUniforms['layer6Tint'] = compiledModule.uniform("layer6Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer6Tint, theme.layer6Alpha), true) 
		assetUniforms['layer7Tint'] = compiledModule.uniform("layer7Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer7Tint, theme.layer7Alpha), true) 
		assetUniforms['layer8Tint'] = compiledModule.uniform("layer8Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer8Tint, theme.layer8Alpha), true) 
		assetUniforms['layer9Tint'] = compiledModule.uniform("layer9Tint","v4", SharedUniforms.convertColorAndAlphaToVector4(theme.layer9Tint, theme.layer9Alpha), true) 


		# used for reference later
		compiledModule.assetUniforms = assetUniforms


		compiledModule.fragmentDefinitions = fragmentDefinitions
		compiledModule.fragmentBody = """
vec2 matcapUV = matcap_1_0( vEyeVector, _fragment_state[1].xyz);
vec3 roughMap = sampleMatcap(#{assetUniforms['roughMatcap']}, matcapUV, #{assetUniforms['roughMatcapFrameData']});
vec3 glossMap = sampleMatcap(#{assetUniforms['glossMatcap']}, matcapUV, #{assetUniforms['glossMatcapFrameData']});

_fragment_state[0].xyz += globeBackground * (1.0 - _fragment_state[0].a);

// for background 1.0, top layer at 0.3
float roughnessStrength = #{assetUniforms['basemapRoughness_and_LightingMultiplier']}.x*(1.0 - _fragment_state[0].a) * #{assetUniforms['basemapRoughness_and_LightingMultiplier']}.y + 
	#{assetUniforms['surfaceRoughness_and_LightingMultiplier']}.x * (_fragment_state[0].a) * #{assetUniforms['surfaceRoughness_and_LightingMultiplier']}.y;

float noiseVal = texture2D( #{assetUniforms['noiseMap']}, vUv ).r;
// yes this is magic for the moment
roughMap += vec3(1.0, 1.0, 1.0) * #{assetUniforms['environment_and_noise_multiplier']}.y * (-0.5 + noiseVal);

#{state.env}.xyz = (glossMap*(1.0 - roughnessStrength) + roughMap * roughnessStrength) * #{assetUniforms['environment_and_noise_multiplier']}.x;

		"""



		compiledModule.init = (attributes, uniforms, context) ->
			_.each ['roughMatcap', 'glossMatcap', 'noiseMap'], (name) ->
				uniformName = assetUniforms[name]
				shader.theme.loadAssetByName name, (texture) ->
					uniforms[uniformName].value = texture
					context.emit 'updated', self

		return compiledModule

module.exports = Environment