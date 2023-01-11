_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
FragmentState = require('../jit/fragment-state')


class Color extends Jit.JitModule
	constructor: (@source) ->
		super("color-#{@source.name}")

	## TODO: this implementation require uniform vUv
	compile: (shader) ->
		float = shader.require("GLSL").float
		source = shader.require(@source)
		state = shader.require('FragmentState')

		color = shader.getUniqueVariable("color");

		return {
			fragmentBody: """
				vec4 #{color} = #{source.sample}(vUv);
				#{state.color}.rgb = mix(#{state.color}.rgb,#{color}.rgb,#{color}.a);
				#{state.color}.a = min(1.0, #{state.color}.a + #{color}.a);
			"""
		}

module.exports = Color