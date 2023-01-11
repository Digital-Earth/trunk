//  NOTE::  this is only a template using EJS formatting, it will break any
//  glsl compiler (ie glslify) unless the template is executed

#define PHONG
uniform sampler2D t0;
uniform sampler2D t1;
uniform sampler2D t2;
uniform sampler2D t3;

uniform mat4 t0m1;
uniform mat4 t1m1;
uniform mat4 t2m1;
uniform mat4 t3m1;

uniform vec4 baseValues[3];

varying vec3 vViewPosition;
varying vec2 vUv;
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
	vec4 Ca = texture2D(t0, uv);
	vec4 Cb = texture2D(t1, uv);
	vec4 Cc = texture2D(t2, uv);
	vec4 Cd = texture2D(t3, uv);
	vec4 lighting=t0m1*Ca+t1m1*Cb+t2m1*Cc+t3m1*Cd+baseValues[1];
	float intensity=1.0+0.1*(lighting.w);
	
	vec3 heightmapped_position= position*intensity;

	<%= THREE.ShaderChunk[ "map_vertex" ] %>
	<%= THREE.ShaderChunk[ "lightmap_vertex" ] %>
	<%= THREE.ShaderChunk[ "color_vertex" ] %>

	<%= THREE.ShaderChunk[ "morphnormal_vertex" ] %>
	<%= THREE.ShaderChunk[ "skinbase_vertex" ] %>
	<%= THREE.ShaderChunk[ "skinnormal_vertex" ] %>
	<%= THREE.ShaderChunk[ "defaultnormal_vertex" ] %>

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
vUv=uv;
}



		
