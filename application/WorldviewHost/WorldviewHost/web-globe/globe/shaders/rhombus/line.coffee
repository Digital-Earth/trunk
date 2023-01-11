_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
FragmentState = require('../jit/fragment-state')


# I switched this to an ejs template so I could use {if / else} style conditionals
bodyTemplate = """
vec4 <%= color %> = <%= source.sample %>(vUv);

<%= state.color %>.rgb = mix(<%= state.color %>.rgb, <%= color %>.rgb, <%= color %>.a * lineOpacity);
<%= state.color %>.a = min(1.0, <%= state.color %>.a + <%= color %>.a);
"""

bodyTemplate = _.template(bodyTemplate)


class Line extends Jit.JitModule
	constructor: (@source, @options) ->
		super("line-#{@source.name}")

	## TODO: this implementation require uniform vUv  
	compile: (shader) ->

		templateOptions = 
			float: shader.require("GLSL").float
			source: shader.require(@source)
			state: shader.require('FragmentState')
			color: shader.getUniqueVariable("color")
			options: @options or {}

		return {
			fragmentBody: bodyTemplate(templateOptions)
		}

module.exports = Line