###
	Here we define the post processing layer to include
	various effects like colorCorrection / tiltShift / FXAA / Bokeh  
###

fragmentShader = require('./scene-post-fs.glsl')
vertexShader = require('./scene-post-vs.glsl')
THREE = require 'three'
EffectComposer = require('three-effectcomposer')(THREE)


generateShaderPass = (theme) ->

	return new EffectComposer.ShaderPass({
		uniforms: theme.generatePostProcessUniforms()
		vertexShader: vertexShader
		fragmentShader: fragmentShader
	}) 


window.EffectComposer = EffectComposer

module.exports = {
	generateShaderPass: generateShaderPass
}