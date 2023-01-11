###
	Palette transformations and utility functions  
###

_ = require 'underscore'
Color = require('color')



###
	There are a number of cases where a palette is being created from a null object,
	I am building it so it just returns white in that case but I think it's a bug?
###
class Palette
	constructor: (@palette) ->
		if @palette
			# these assignments assume that the palette is in order
			@min = @palette.Steps[0].Value
			@max = @palette.Steps[@palette.Steps.length-1].Value
		else
			@min = 0.0
			@max = 1.0


	###*
	 * Return the evaluated color for a specific value
	 * @param  {object} options
	 * @return {string|object} color string or object
	###
	evaluate: (value, options) ->
		options or= {}

		if not @palette
			return '#ffffff' if options.hexString
			return 'rgba(255,255,255,1.0)' if options.rgbaString
			return Color('#ffffff')

		steps = @palette.Steps
		lowVal = steps[0]
		highVal = steps[0]
		blendValue = 0

		for step, i in steps
			if value is step.Value
				lowVal = step
				highVal = step
				break

			if value > step.Value and steps[i+1] and value < steps[i+1].Value
				lowVal = step
				highVal = steps[i+1]
				blendValue = (value - lowVal.Value) / (highVal.Value - lowVal.Value)
				break

		lowColor = Color(lowVal.Color)
		highColor = Color(highVal.Color)
		newColor = lowColor.mix(highColor, 100 * blendValue)

		if options.hexString
			return newColor.hexString()
		else if options.rgbaString
			return newColor.rgbaString()
		
		return newColor


	###*
	 * Return the evaluated color given a float value between 0 and 1
	 * @param  {object} options
	 * @return {string|object} color string or object
	###
	evaluateNormalized: (value, options) ->
		value = value * (@max - @min) + @min
		return @evaluate(value, options)


	###*
	 * 	Given a palette object, which may have hex color values and real world values
	 *  and return an array of objects with floating point RGBA values and
	 *  a normalized value
	 *
	 *  Array of [ value, red, green, blue, alpha ]
	 *
	 * @return {array} colors
	###
	normalizedSteps: ->

		newPalette = _.map @palette.Steps, (step) =>
			c = Color(step.Color)
			normalizedValue = ((step.Value - @min)/ @max)

			# [value, red, green, blue, alpha]
			return [normalizedValue, c.red()/255.0, c.green()/255.0, c.blue()/255.0, c.alpha()/255.0 ]

		return newPalette


	###*
	 * 	Compile palette to a GLSL expression for color evalutation
	 *
	 * @return {array} colors
	###
	glslCompile: (variable, value) ->
		lines = []
		lines.push "#{variable} = vec4(0.0);"

		makeFloat = (n) ->
				s = "" + n
				s += ".0" if s.indexOf('.') == -1
				return s


		colors = _.map @palette.Steps, (step) ->
			c = Color(step.Color)

			return "vec4(#{makeFloat(c.red()/255.0)},#{makeFloat(c.green()/255.0)},#{makeFloat(c.blue()/255.0)},#{makeFloat(c.alpha())})"


		lines.push "#{value} = #{value} * ( #{makeFloat(@max)} - #{makeFloat(@min)} ) + #{makeFloat(@min)};"

		values = _.map @palette.Steps, (step) -> step.Value

		for i in [0...colors.length-1] by 1
			line = "#{variable} += mix(#{colors[i]},#{colors[i+1]},clamp((#{value} - #{makeFloat(values[i])})/(#{makeFloat(values[i+1])} - #{makeFloat(values[i])}),0.0,1.0))"
			line += " * step(#{makeFloat(values[i])},#{value})" if i > 0
			line += " * (1.0 - step(#{makeFloat(values[i+1])},#{value}))" if i+1 < colors.length-1
			lines.push line + ";"

		return lines.join '\n'




module.exports = Palette
