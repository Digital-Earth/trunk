_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')

class Gradient extends Jit.JitModule
	constructor: (@source,@palette) ->
		super("gradient")

	compile: (shader) ->
		float = shader.require("GLSL").float
		source = shader.require(@source)

		variable = shader.getUniqueVariable("color");

		colors = _.map @palette.Steps, (step) ->
			c = Color(step.Color)
			return "vec4(#{float(c.red()/255.0)},#{float(c.green()/255.0)},#{float(c.blue()/255.0)},#{float(c.alpha())})"

		values = _.map @palette.Steps, (step) -> step.Value

		min = @palette.Steps[0].Value
		max = @palette.Steps[@palette.Steps.length-1].Value

		lines = []
		lines.push "vec4 #{variable} = vec4(0.0);"

		for i in [0...colors.length-1] by 1
			line = "#{variable} += mix(#{colors[i]},#{colors[i+1]},clamp((#{source.value} - #{float(values[i])})/(#{float(values[i+1])} - #{float(values[i])}),0.0,1.0))"
			line += " * step(#{float(values[i])},#{source.value})" if i > 0
			line += " * (1.0 - step(#{float(values[i+1])},#{source.value}))" if i+1 < colors.length-1
			lines.push line + ";"

		return {
			body: lines.join '\n'
			exports:
				"color" : variable
		}

module.exports = Gradient
