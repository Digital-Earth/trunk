_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
FragmentState = require('../jit/fragment-state')


# I switched this to an ejs template so I could use {if / else} style conditionals
bodyTemplate = """
vec4 <%= color %> = <%= source.sample %>(vUv);


<% if (options.applyToLandAlpha) { %>
	<%= state.landAlpha %> = <%= color %>.a;
<% } %>

<% if (options.landOnly) { %>
	<%= color %>.a = min(<%= state.landAlpha %>, <%= color %>.a );
<% } %>

<% if (options.globalAlpha > 0 && options.globalAlpha < 1) { %>
	<%= color %>.a *= <%= options.globalAlpha %>;
<% } %>


<%= state.color %>.rgb = mix(<%= state.color %>.rgb, <%= color %>.rgb * 2.0 * <%= layerUniforms.tint %>.rgb, <%= color %>.a * <%= layerUniforms.tint %>.a);
<%= state.color %>.a = min(1.0, <%= state.color %>.a + <%= color %>.a);
"""

bodyTemplate = _.template(bodyTemplate)



###
	This is basically the same as Color, but provides methods for reading the 
	theme to provide custom layer blending.
###
class ColorBlend extends Jit.JitModule
	constructor: (@source, @options) ->
		super("color-blend-#{@source.name}")

	## TODO: this implementation require uniform vUv  
	compile: (shader) ->
		@options or= {layerIndex: 0}

		templateOptions = 
			float: shader.require("GLSL").float
			source: shader.require(@source)
			state: shader.require('FragmentState')
			color: shader.getUniqueVariable("color")
			options: @options
			layerUniforms: 
				tint: "layer#{@options.layerIndex}Tint_1" 

		return {
			fragmentBody: bodyTemplate(templateOptions)
		}

module.exports = ColorBlend