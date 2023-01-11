_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')

#Module function for computing texture mip level
module.exports = Jit.JitShader.register('TextureMiplevel',
	{
		fragmentDefinitions: require('./texture-mip-level.glsl')
		vertexDefinitions: "float textureMiplevel(vec2 uv, vec2 textureSize){return 0.0;}"
		exports: {
			textureMiplevel: 'textureMiplevel'
		}
	})