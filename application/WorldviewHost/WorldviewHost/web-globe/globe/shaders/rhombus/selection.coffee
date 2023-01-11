_ = require 'underscore'
THREE = require 'three'
Jit = require '../../jit-shader'
FragmentState = require '../jit/fragment-state'
angles = require '../../../core/angles'


# I switched this to an ejs template so I could use {if / else} style conditionals 
bodyTemplate = """
vec4 <%= color %> = <%= source.sample %>(vUv);

<%= color %>.a *= 0.6;
<%= color %>.rgb += 0.2 * (sin(( <%= float(options.yScale) %> * vEyeVector.y + <%= float(options.xScale) %> * vEyeVector.x ) * <%= float(options.length) %> * screenSize.y + <%= float(options.speed) %> * time.x));

<%= state.color %>.rgb = mix(<%= state.color %>.rgb, <%= color %>.rgb, <%= color %>.a);
<%= state.color %>.a = min(1.0, <%= state.color %>.a + <%= color %>.a);
"""

bodyTemplate = _.template(bodyTemplate)


class Selection extends Jit.JitModule
	constructor: (@source, @options) ->
		super("selection-#{@source.name}")

	## TODO: this implementation require uniform vUv  
	compile: (shader) ->
		#angle 0 is up and goes in clock wise direction
		angle = angles.radians( (@options.Angle || 0) - 90);

		templateOptions = 
			float: shader.require("GLSL").float
			source: shader.require(@source)
			state: shader.require('FragmentState')
			color: shader.getUniqueVariable("color")
			options: {
				xScale: -Math.cos(angle)
				yScale: Math.sin(angle) 
				speed: (@options.Speed || 0) * (2 * Math.PI)
				length: (2 * Math.PI) / (@options.Length || 3) / 2
			}

		return {
			definitions: "uniform vec2 screenSize;",
			fragmentBody: bodyTemplate(templateOptions)
		}

module.exports = Selection