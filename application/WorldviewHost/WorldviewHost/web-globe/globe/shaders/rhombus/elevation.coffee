_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')
FragmentState = require('../jit/fragment-state')


class Elevation extends Jit.JitModule
	# keep the scale at 1.0 until we have elevation auto-adjusting
	constructor: (@source, @scale = 1) ->
		super("elevation-#{@source.name}")

	## TODO: this implementation require uniform vUv
	compile: (shader) ->
		self = this
		float = shader.require("GLSL").float
		state = shader.require('FragmentState')

		sourceAsNormal = shader.require(@source.asNormal(20))

		compiledModule = new Jit.JitCompiledModule(this,shader)
		elevationScale = compiledModule.uniform("elevationScale","f", @scale)

		elevationFunctionName = compiledModule.function("elevation",[
			"void elevation(inout vec4 #{state.variable}[#{state.arraySize}]) {"
			"    float sampleValid = #{sourceAsNormal.sampleValid}(vUv);"
			"    vec3 normal = #{sourceAsNormal.sample}(vUv);"
			"    float elevation = #{sourceAsNormal.sampleValue}(vUv);"
			"    #{state.normal}.xyz = normalize(mix(#{state.normal}.xyz, normal, sampleValid));"
			"    #{state.elevation} = mix(#{state.elevation}, elevation, sampleValid);"
			"}"
			].join("\n"),"fragment")

		compiledModule.fragmentBody = "#{elevationFunctionName}(#{state.variable});"
		#TODO We can't map special function for vertex shader only, so use unique
		# compiledModule.vertexBody = "#{state.elevation} = #{sourceAsNormal.sampleValueVertex}(vUv) * #{elevationScale};"
		compiledModule.vertexBody = [
			"if (#{sourceAsNormal.sampleValueVertexValid}(vUv)) {"
			"    elevationValue = #{sourceAsNormal.sampleValueVertex}(vUv) * #{elevationScale};"
			"}"
			"if (#{sourceAsNormal.sampleValueVertexSkirt}(vUv)) {"
			"	elevationValue = min(elevationValue, min( #{float(@source.min)} * #{elevationScale}, -300000.0));"
			"}"
			].join("\n")

		emitVertexElevation = (attributes, uniforms, context) ->
			attributeName = sourceAsNormal.value
			valuesWithoutScale = attributes[attributeName].array

			return unless valuesWithoutScale

			vertexElevations = _.map valuesWithoutScale, (value) -> value * self.scale
			
			context.emit 'elevationChanged', {
				module: self,
				vertexElevations: vertexElevations
			}

		compiledModule.init = (attributes, uniforms, context) ->
			emitVertexElevation(attributes, uniforms, context)
			context.on 'updated' , (source) ->
				if source == self.source
					emitVertexElevation(attributes, uniforms, context) 

					

		return compiledModule

module.exports = Elevation