_ = require 'underscore'
angles = require('../../../core/angles')
THREE = require 'three'
Jit = require('../../jit-shader')
Color = require('color')

###*
 * WGS84 JitModule - a fast version of wgs84 module on GLSL
 * @type {String}
###
Jit.JitShader.register('WGS84',
	{
		definitions:  """
const float kfFlattening = 1.0 / 298.257223563;
const float kfOneMinusFSqr = (1.0 - kfFlattening) * (1.0 - kfFlattening);
const float kfDefaultDoublePrecision = 1.0e-10;

vec2 xyzToWgs84(vec3 xyz) {
	xyz = normalize(xyz);

	if ( abs( 1.0 - xyz.y) < kfDefaultDoublePrecision ) { return vec2( #{angles.const.degrees90InRadians},0.0); }
	if ( abs(-1.0 - xyz.y) < kfDefaultDoublePrecision ) { return vec2(-#{angles.const.degrees90InRadians},0.0); }
	
	//geocentric latlon
	float lat = asin(xyz.y);
	float lon = -atan(xyz.z,xyz.x);

	//convert to wgs84 lat.
	return vec2(atan(tan(lat)/kfOneMinusFSqr),lon);
}
	"""
		,
		exports: {
			xyzToWgs84:'xyzToWgs84'
		}
	})

###*
 * Wgs84BoundingBox jit module. render a color or a texture based on Wgs84 coordinates
 *
 * this is a source module thar return a color based on the pixel lat/lon.
 *
 * Wgs84BoundingBox([{lat:0,lon:0},{lat:45,lon:45}]) will return #fff for every pixel inside the bounding box,
 * and will return transparent color for the pixels outside.
 *
 * Wgs84BoundingBox(coordinates, { color: "#f00" }) - will change the color of the bounding box
 *
 * bbox = Wgs84BoundingBox(coordinates, { dynamic: true }) - will create a uniform based on the coordiantes.
 * bbox.update(coordinate); // update the uniform value
 *
 * Wgs84BoundingBox(coordinates, { texture: {@THREE.Texture} texture }) - will reurn a value based on the texture.
 * 
 * Wgs84BoundingBox(coordinates, { texture: {@THREE.Texture} texture, textureBBox: coordinates }) - use different bbox for the texture uv translation.
 *
###
class Wgs84BoundingBox extends Jit.JitModule
	constructor: (@coordinates, @options) ->
		@options ?= {}
		@color = Color(@options.color || "#fff")
		@latMin = angles.radians(Math.min(@coordinates[0].lat,@coordinates[1].lat))
		@latMax = angles.radians(Math.max(@coordinates[0].lat,@coordinates[1].lat))
		@lonMin = angles.radians(Math.min(@coordinates[0].lon,@coordinates[1].lon))
		@lonMax = angles.radians(Math.max(@coordinates[0].lon,@coordinates[1].lon))
		
		if @options.texture
			if @options.textureBBox
				@textureLatMin = angles.radians(Math.min(@options.textureBBox[0].lat,@options.textureBBox[1].lat))
				@textureLatMax = angles.radians(Math.max(@options.textureBBox[0].lat,@options.textureBBox[1].lat))
				@textureLonMin = angles.radians(Math.min(@options.textureBBox[0].lon,@options.textureBBox[1].lon))
				@textureLonMax = angles.radians(Math.max(@options.textureBBox[0].lon,@options.textureBBox[1].lon))
			else
				@textureLatMin = @latMin
				@textureLatMax = @latMax
				@textureLonMin = @lonMin
				@textureLonMax = @lonMax

		if @options.dynamic
			@bboxUniformValue = new THREE.Vector4(@latMin,@latMax,@lonMin,@lonMax)
			if @options.texture
				@textureBboxUniformValue = new THREE.Vector4(@textureLatMin,@textureLatMax,@textureLonMin,@textureLonMax)

		super("wgs84-bounding-box-[#{@latMin}..#{@latMax},#{@lonMin}..#{@lonMax}]")

	update: (coordinates) ->
		@coordinates = coordinates

		@latMin = angles.radians(Math.min(@coordinates[0].lat,@coordinates[1].lat))
		@latMax = angles.radians(Math.max(@coordinates[0].lat,@coordinates[1].lat))
		@lonMin = angles.radians(Math.min(@coordinates[0].lon,@coordinates[1].lon))
		@lonMax = angles.radians(Math.max(@coordinates[0].lon,@coordinates[1].lon))
		
		if @options.texture
			if !@options.textureBBox
				@textureLatMin = @latMin
				@textureLatMax = @latMax
				@textureLonMin = @lonMin
				@textureLonMax = @lonMax

		if @options.dynamic
			@bboxUniformValue.set(@latMin,@latMax,@lonMin,@lonMax)
			
			if @options.texture
				@textureBboxUniformValue.set(@textureLatMin,@textureLatMax,@textureLonMin,@textureLonMax)

	compile: (shader) ->
		float = shader.require("GLSL").float
		value = shader.require("GLSL").value
		xyzToWgs84 = shader.require('WGS84').xyzToWgs84
		
		compiledModule = new Jit.JitCompiledModule(this,shader)
		
		getColor = "#{value(@color)}"

		if @options.texture
			textureSampler = compiledModule.uniform("texture",'t',@options.texture,true)

		if @options.dynamic
			bboxUniform = compiledModule.uniform("bbox",'v4',@bboxUniformValue,true)
			
			if @options.texture
				textureBboxUniform = compiledModule.uniform("tbbox",'v4',@textureBboxUniformValue,true)
				
				result.exports.textureBboxUniform = textureBboxUniform
				getColor = "texture2D(#{textureSampler},vec2( (latlon.y - #{textureBboxUniform}.z)/(#{textureBboxUniform}.w - #{textureBboxUniform}.z), (latlon.x - #{textureBboxUniform}.x)/(#{textureBboxUniform}.y - #{textureBboxUniform}.x) ))"

			compiledModule.exportFunction('sample',"""
vec4 sample(vec2 uv)	{
	vec2 latlon = #{xyzToWgs84}(vWorldPosition);

	if (latlon.x >= #{bboxUniform}.x && latlon.x <= #{bboxUniform}.y && latlon.y >= #{bboxUniform}.z && latlon.y <= #{bboxUniform}.w) {
		return #{getColor};
	} else {
		return vec4(0.0);
	}
}
""")
		else
			if @options.texture
				getColor = "texture2D(#{textureSampler},vec2( (latlon.y - #{float(@textureLonMin)})/(#{float(@textureLonMax)} - #{float(@textureLonMin)}), (latlon.x - #{float(@textureLatMin)})/(#{float(@textureLatMax)} - #{float(@textureLatMin)}) ))"
		
			compiledModule.exportFunction('sample',"""
vec4 sample(vec2 uv)	{
	vec2 latlon = #{xyzToWgs84}(vWorldPosition);

	if (latlon.x >= #{float(@latMin)} && latlon.x <= #{float(@latMax)} && latlon.y >= #{float(@lonMin)} && latlon.y <= #{float(@lonMax)}) {
		return #{getColor};
	} else {
		return vec4(0.0);
	}
}
""")

		return compiledModule

module.exports = Wgs84BoundingBox