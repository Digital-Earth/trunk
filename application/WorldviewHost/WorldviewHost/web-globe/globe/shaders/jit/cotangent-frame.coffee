_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')

module.exports = Jit.JitShader.register('CotangentFrame',
	{
		fragmentDefinitions: require('./cotangent-frame.glsl')
		exports: {
			cotangentFrame: 'cotangentFrame'
		}
	})