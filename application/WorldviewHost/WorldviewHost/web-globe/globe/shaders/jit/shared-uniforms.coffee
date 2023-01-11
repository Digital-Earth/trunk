_ = require 'underscore'
THREE = require 'three'

tempThreeColor = new THREE.Color()

#Shared uniforms give ability to use one value in all instances of the shader
#to enable sharing, supply shared:true in uniform parameters list
#currently not working with POD types(float,bool,int...)
#can be used on THREE.Color, THREE.Vectors, THREE.Matrix, THREE.Texture and Arrays
merge = (uniforms) ->
	merged = {};
	for u in [0...uniforms.length]
		tmp = clone( uniforms[u] )
		for k, v of tmp
			merged[k] = v

	return merged

clone = (uniforms_src) ->
	uniforms_dst = {}
	for u of uniforms_src
		uniforms_dst[u] = {}
		shared = uniforms_src[u].shared
		for p of uniforms_src[u]
			parameter_src = uniforms_src[u][p]

			if ( !shared and (parameter_src instanceof THREE.Color ||
					 parameter_src instanceof THREE.Vector2 ||
					 parameter_src instanceof THREE.Vector3 ||
					 parameter_src instanceof THREE.Vector4 ||
					 parameter_src instanceof THREE.Matrix3 ||
					 parameter_src instanceof THREE.Matrix4 ||
					 parameter_src instanceof THREE.Texture ) )
				uniforms_dst[ u ][ p ] = parameter_src.clone()
			else if ( !shared and (Array.isArray( parameter_src )) )
				uniforms_dst[ u ][ p ] = parameter_src.slice()
			else
				uniforms_dst[ u ][ p ] = parameter_src

	return uniforms_dst

assignColorAndAlphaToVector4 = (targetVec4, color, alpha) ->
	tempThreeColor.set(color)
	targetVec4.set(tempThreeColor.r, tempThreeColor.g, tempThreeColor.b, alpha)

convertColorAndAlphaToVector4 = (color, alpha) ->
	tempThreeColor.set(color)
	return new THREE.Vector4(tempThreeColor.r, tempThreeColor.g, tempThreeColor.b, alpha)

module.exports =
	merge: merge
	clone: clone
	assignColorAndAlphaToVector4: assignColorAndAlphaToVector4
	convertColorAndAlphaToVector4: convertColorAndAlphaToVector4