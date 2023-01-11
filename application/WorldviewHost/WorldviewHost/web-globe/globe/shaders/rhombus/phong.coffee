_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')
FragmentState = require('../jit/fragment-state')



class Phong extends Jit.JitModule
	constructor: () -> super("phong")

	compile: (shader) ->
		self = this

		state = shader.require('FragmentState')
		theme = shader.theme

		compiledModule = new Jit.JitCompiledModule(this, shader)

		settings = """
#define CUSTOM_NORMAL

#define MAX_DIR_LIGHTS 2
#define MAX_POINT_LIGHTS 0
#define MAX_SPOT_LIGHTS 0
#define MAX_HEMI_LIGHTS 0
#define MAX_SHADOWS 2

#define GAMMA_INPUT
#define GAMMA_OUTPUT
#define GAMMA_FACTOR 2
"""
		definitions = require('../layers/phong-header.glsl')
		phongBody = require('../layers/phong.glsl')

		phongFunctionName = shader.getUniqueVariable('phong')

		phongStrength = compiledModule.uniform("phongStrength", "f", theme.phongStrength)

		phongFunction = [
			"void #{phongFunctionName}(inout vec4 #{state.variable}[#{state.arraySize}]) {",
			"    vec3 viewPosition = normalize( vViewPosition );",
			"    vec3 normal = #{state.normal}.xyz;",
			"    vec3 diffuse = #{state.diffuse}.xyz * #{state.color}.xyz;",
			"    vec3 specular = #{state.specular}.xyz;",
			"    float shininess = #{state.shininess};",
			"    float specularStrength = #{state.specularStrength};",
			phongBody,
			"    totalDiffuse += #{state.env}.xyz;"
			"    #{state.color}.xyz = mix(#{state.color}.xyz,totalDiffuse,#{phongStrength} * #{state.diffuseStrength}) + totalSpecular;"
			"}"
			].join "\n"


		compiledModule.vertexSettings = settings
		compiledModule.fragmentSettings = settings
		compiledModule.definitions += "\n" + definitions + "\n" + phongFunction
		compiledModule.fragmentBody = "#{phongFunctionName}(#{state.variable});"

		return compiledModule


module.exports = Phong