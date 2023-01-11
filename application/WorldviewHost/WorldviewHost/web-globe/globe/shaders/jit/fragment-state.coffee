_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')

variableName = "_fragment_state"
stateIndex = 0

exports = {
	variable: variableName
}

registerVec4 = (name) ->
	index = "#{variableName}[#{stateIndex}]"
	exports[name] = index if name
	stateIndex++
	return index

registerFloats = (name1,name2,name3,name4) ->
	mixedItem = registerVec4()
	exports[name1] = mixedItem + ".x" if name1
	exports[name2] = mixedItem + ".y" if name2
	exports[name3] = mixedItem + ".z" if name3
	exports[name4] = mixedItem + ".w" if name4

registerVec4("color")
registerVec4("normal")
registerVec4("env")
registerVec4("specular")
registerVec4("diffuse")
registerVec4("emissive")
registerVec4("ambient")

registerFloats("diffuseStrength", "shininess", "specularStrength", "landAlpha")
registerFloats("elevation")
registerFloats("roughness")

exports.arraySize = stateIndex

module.exports = Jit.JitShader.register('FragmentState',
	{
		vertexSettings: """
precision highp float;
precision highp int;
"""
		fragmentSettings: """
#extension GL_OES_standard_derivatives : enable

precision highp float;
precision highp int;
"""
		vertexDefinitions: """
#define PHONG

attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;
attribute vec2 uv2;

#ifdef USE_COLOR
	attribute vec3 color;
#endif
#ifdef USE_MORPHTARGETS
	attribute vec3 morphTarget0;
	attribute vec3 morphTarget1;
	attribute vec3 morphTarget2;
	attribute vec3 morphTarget3;
	#ifdef USE_MORPHNORMALS
		attribute vec3 morphNormal0;
		attribute vec3 morphNormal1;
		attribute vec3 morphNormal2;
		attribute vec3 morphNormal3;
	#else
		attribute vec3 morphTarget4;
		attribute vec3 morphTarget5;
		attribute vec3 morphTarget6;
		attribute vec3 morphTarget7;
	#endif
#endif
#ifdef USE_SKINNING
	attribute vec4 skinIndex;
	attribute vec4 skinWeight;
#endif

uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;
uniform vec3 cameraPosition;

varying vec3 vWorldPosition;
varying vec3 vViewPosition;

#ifndef FLAT_SHADED
varying vec3 vNormal;
#endif
varying vec2 vUv;
varying vec3 vEyeVector;
#define METAL


///REMOVE THIS WHEN POSSIBLE
uniform vec4 lightingParams;

uniform vec2 time;
uniform vec3 globeBackground;

#define EARTH_SCALE 6371007.0

"""
		fragmentDefinitions: """
uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;
uniform vec3 cameraPosition;

varying vec3 vWorldPosition;
varying vec3 vViewPosition;

#ifndef FLAT_SHADED
varying vec3 vNormal;
#endif
varying vec2 vUv;
varying vec3 vEyeVector;
#define METAL


///REMOVE THIS WHEN POSSIBLE 
uniform vec4 lightingParams;

uniform vec2 time;
uniform vec3 globeBackground;

#define EARTH_SCALE 6371007.0

"""
		fragmentBody: """

	// ignore this -- it minimizes a Warning
	vec3 foo = vWorldPosition;
	foo = vEyeVector;


	gl_FragColor=vec4(1.0);

	vec4 #{variableName}[#{exports.arraySize}];
	#{exports.color} = vec4(0.0,0.0,0.0,0.0);		//color
	#{exports.normal}.xyz = normalize(vNormal);		// normal
	#{exports.normal}.w = 1.0;						//
	#{exports.env} = vec4(0.0);						//env
	#{exports.specular} = vec4(lightingParams.x);	//specular
	#{exports.diffuse} = vec4(lightingParams.z);	//diffuse
	#{exports.emissive} = vec4(0.0);				//emissive
	#{exports.diffuseStrength} = 1.0; 				//diffuseStrength lightingParams.w;
	#{exports.shininess} = lightingParams.y;		//shininess
	#{exports.specularStrength} = 1.0; 				//specularStrength
	#{exports.elevation} = 0.0; 					//elevation
	#{exports.landAlpha} = 1.0; 					//landAlpha

"""
		fragmentPost: "gl_FragColor = #{exports.color};",

		vertexBody: """
	float elevationValue = 0.0; 

	//make sure vUv is setup before the rest of later ocde
	vUv=uv;
"""
		vertexPost: """
	//float heightFactor = #{exports.elevation} / EARTH_SCALE + 1.0;
	float heightFactor = elevationValue / EARTH_SCALE + 1.0;

	vec3 heightmapped_position = position * heightFactor;
	vec4 mvPosition = modelViewMatrix * vec4( heightmapped_position, 1.0 );
	vNormal = normalize(normalMatrix * position);

	gl_Position = projectionMatrix * mvPosition;
	vViewPosition = -mvPosition.xyz;

	vec4 worldPosition = modelMatrix * vec4( heightmapped_position, 1.0 );

	vWorldPosition = worldPosition.xyz;

	vEyeVector = normalize( vec3( modelViewMatrix * vec4( heightmapped_position, 1.0 ) ) );

"""
		exports: exports
	})