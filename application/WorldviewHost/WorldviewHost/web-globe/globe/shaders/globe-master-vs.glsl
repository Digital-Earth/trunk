precision highp float;
precision highp int;

#define VERTEX_TEXTURES
#define GAMMA_INPUT
#define GAMMA_OUTPUT
#define GAMMA_FACTOR 2
#define MAX_DIR_LIGHTS 2
#define MAX_POINT_LIGHTS 0
#define MAX_SPOT_LIGHTS 0
#define MAX_HEMI_LIGHTS 0
#define MAX_SHADOWS 2
#define MAX_BONES 251





uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;
uniform vec3 cameraPosition;
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







//  START non-preprocessor  --------------



#define PHONG

#define EARTH_SCALE 6371007.0

uniform sampler2D t0;
uniform sampler2D t1;
uniform sampler2D t2;
uniform sampler2D t3;
uniform sampler2D t4;
uniform sampler2D t5;
uniform sampler2D t6;
uniform sampler2D t7;
uniform sampler2D t8;
uniform sampler2D t9;

uniform mat4 t0m1;
uniform mat4 t1m1;
uniform mat4 t2m1;
uniform mat4 t3m1;


// some elevation related uniforms
uniform float useElevation;
uniform float elevationMultichannel;
uniform vec2 elevationRange;
uniform sampler2D elevationMap;
uniform vec4 elevationMapUvTrans;
uniform float heightMultiplier;
uniform vec4 baseValues[3];

varying vec3 vViewPosition;
varying vec2 vUv;
varying vec3 vEyeVector;

uniform float normalmap_strength;

#ifndef FLAT_SHADED

	varying vec3 vNormal;

#endif

<%= THREE.ShaderChunk[ "common" ] %>
<%= THREE.ShaderChunk[ "map_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "lightmap_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "envmap_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "lights_phong_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "color_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "morphtarget_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "skinning_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "shadowmap_pars_vertex" ] %>
<%= THREE.ShaderChunk[ "logdepthbuf_pars_vertex" ] %>

void main() {
	vec3 root= vec3(0,0,0);

	// 
	// vec4 Ca = texture2D(t0, uv);
	// vec4 Cb = texture2D(t1, uv);
	// vec4 Cc = texture2D(t2, uv);
	// vec4 Cd = texture2D(t3, uv);
	// vec4 lighting = t0m1*Ca+t1m1*Cb+t2m1*Cc+t3m1*Cd+baseValues[1];
	// float intensity = 1.0 + 0.1 * (lighting.w);

	// replaced the matrix input to just use t0 directly
	float height = 1.0;

	if ( useElevation == 1.0 ){
		float normaldef = normal.x;
		height = 1.0 + heightMultiplier * mix(elevationRange.x,elevationRange.y,normaldef) / EARTH_SCALE;		
	}


	vec3 heightmapped_position = position * height;

	<%= THREE.ShaderChunk[ "map_vertex" ] %>
	<%= THREE.ShaderChunk[ "lightmap_vertex" ] %>
	<%= THREE.ShaderChunk[ "color_vertex" ] %>

	<%= THREE.ShaderChunk[ "morphnormal_vertex" ] %>
	<%= THREE.ShaderChunk[ "skinbase_vertex" ] %>
	<%= THREE.ShaderChunk[ "skinnormal_vertex" ] %>
	<%= THREE.ShaderChunk[ "defaultnormal_vertex" ] %>

	transformedNormal = normalMatrix * position;

#ifndef FLAT_SHADED // Normal computed with derivatives when FLAT_SHADED

	vNormal = normalize( transformedNormal );

#endif

	vec4 mvPosition = modelViewMatrix * vec4( heightmapped_position, 1.0 );

	


	gl_Position = projectionMatrix * mvPosition;
	<%= THREE.ShaderChunk[ "logdepthbuf_vertex" ] %>

	vViewPosition = -mvPosition.xyz;

	<%= THREE.ShaderChunk[ "worldpos_vertex" ] %>
	<%= THREE.ShaderChunk[ "envmap_vertex" ] %>
	<%= THREE.ShaderChunk[ "lights_phong_vertex" ] %>
	<%= THREE.ShaderChunk[ "shadowmap_vertex" ] %>

	vEyeVector = normalize( vec3( modelViewMatrix * vec4( position, 1.0 ) ) );

	vUv=uv;
}



		
