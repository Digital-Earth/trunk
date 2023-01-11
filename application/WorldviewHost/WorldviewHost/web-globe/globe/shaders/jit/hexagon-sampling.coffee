_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')

module.exports = Jit.JitShader.register('HexagonSampling',
	{
		definitions: require('./hexagon-sampling.glsl')
		exports: {
			nearest: 'hexagonTexCoord'
			sampleNearest: 'sampleHexNearest'
			sampleLinear: 'sampleHexLinear'
		}
	})