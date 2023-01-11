
#define PHONG

<%= THREE.ShaderChunk["common"] %>

uniform float time;

uniform sampler2D t0;
uniform sampler2D t1;
uniform sampler2D t2;
uniform sampler2D t3;

uniform mat4 t0m0;
uniform mat4 t0m1;
uniform mat4 t0m2;
uniform mat4 t1m0;
uniform mat4 t1m1;
uniform mat4 t1m2;
uniform mat4 t2m0;
uniform mat4 t2m1;
uniform mat4 t2m2;
uniform mat4 t3m0;
uniform mat4 t3m1;
uniform mat4 t3m2;
uniform vec4 baseValues[3];
uniform float cutoff;

uniform int masktest;
uniform sampler2D tMatCap;
uniform sampler2D tTest;
//uniform samplerCube tEnvMap;

uniform float reflectAmount;
uniform vec3 globeBackground;
uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 color3;
uniform vec3 color4;
uniform vec3 color5;

uniform float isHighDefRGB;
uniform float funMode;

<%= THREE.ShaderChunk["color_pars_fragment"] %>
<%= THREE.ShaderChunk["map_pars_fragment"] %>
<%= THREE.ShaderChunk["alphamap_pars_fragment"] %>
<%= THREE.ShaderChunk["lightmap_pars_fragment"] %>
//#define ENVMAP_TYPE_CUBE
<%= THREE.ShaderChunk["envmap_pars_fragment"] %>
<%= THREE.ShaderChunk["fog_pars_fragment"] %>
<%= THREE.ShaderChunk["lights_phong_pars_fragment"] %>
<%= THREE.ShaderChunk["shadowmap_pars_fragment"] %>
<%= THREE.ShaderChunk["bumpmap_pars_fragment"] %>
<%= THREE.ShaderChunk["normalmap_pars_fragment"] %>
<%= THREE.ShaderChunk["specularmap_pars_fragment"] %>
<%= THREE.ShaderChunk["logdepthbuf_pars_fragment"] %>
varying vec2 vUv;
varying vec3 vEyeVector;
//#define METAL


float COLOR_MAX = 256.0 * 256.0;


//   --------  GLSLify includes  -------------


// matcap converts cubemap style normal to a 2d spherical reflection matcap
//#pragma glslify: matcap = require(matcap)


void main(void)
{
	float opacity=0.0;
	vec3 c;
	vec4 Ca = texture2D(t0, vUv);
	vec4 Cb = texture2D(t1, vUv);
	vec4 Cc = texture2D(t2, vUv);
	vec4 Cd = texture2D(t3, vUv);
	
	vec4 col=t0m0*Ca+t1m0*Cb+t2m0*Cc+t3m0*Cd+baseValues[0];
	vec4 lighting=t0m1*Ca+t1m1*Cb+t2m1*Cc+t3m1*Cd+baseValues[1];
	lighting=clamp(lighting,0.0,10.0);
	col=clamp(col,0.0,10.0);
	vec4 data=t0m2*Ca+t1m2*Cb+t2m2*Cc+t3m2*Cd+baseValues[2];
   	vec3 emissive= lighting.z*col.xyz;
	vec3 specular=vec3(lighting.x);//1.5,0.5,0.5);
   	float shininess=lighting.y;

	//float specularStrength=5.0;
	
	gl_FragColor=vec4(0.0);	

	vec3 diffuse = col.rgb;
	
	diffuse=vec3(1,1,1);
	specular=vec3(0.7,0.7,0.7);
	emissive=vec3(0,0,0);
	shininess=0.7;
	reflectivity=0.9;

	if (isHighDefRGB == 1.0){
		// 256.0 * 256.0 * 256.0
		// float highdef = (256.0 * Ca.x +16.0*Ca.y + Ca.z) / 256.0;
		float highdef = (Ca.z  + Ca.y / 256.0 + Ca.x / (256.0*256.0) )/COLOR_MAX * 1.5;   // div instead of mult?
        float RVAL = 	smoothstep(0.1, 0.0, highdef) * color0.x +
        				smoothstep(0.0, 0.1, highdef) * smoothstep(0.1, 0.25, highdef) *    color1.x +
        				smoothstep(0.25, 0.4, highdef) * smoothstep(0.55, 0.4, highdef) *   color2.x +
        				smoothstep(0.55, 0.65, highdef) * smoothstep(0.75, 0.65, highdef) * color3.x +
        				smoothstep(0.75, 0.85, highdef) * smoothstep(0.95, 0.85, highdef) * color4.x +
        				smoothstep(0.85, 1.0, highdef) * color5.x;

        float GVAL = 	smoothstep(0.1, 0.0, highdef) * color0.y +
        				smoothstep(0.0, 0.1, highdef) * smoothstep(0.1, 0.25, highdef) *    color1.y +
        				smoothstep(0.25, 0.4, highdef) * smoothstep(0.55, 0.4, highdef) *   color2.y +
        				smoothstep(0.55, 0.65, highdef) * smoothstep(0.75, 0.65, highdef) * color3.y +
        				smoothstep(0.75, 0.85, highdef) * smoothstep(0.95, 0.85, highdef) * color4.y +
        				smoothstep(0.85, 1.0, highdef) * color5.y;

        float BVAL = 	smoothstep(0.1, 0.0, highdef) * color0.z +
        				smoothstep(0.0, 0.1, highdef) * smoothstep(0.1, 0.25, highdef) *    color1.z +
        				smoothstep(0.25, 0.4, highdef) * smoothstep(0.55, 0.4, highdef) *   color2.z +
        				smoothstep(0.55, 0.65, highdef) * smoothstep(0.75, 0.65, highdef) * color3.z +
        				smoothstep(0.75, 0.85, highdef) * smoothstep(0.95, 0.85, highdef) * color4.z +
        				smoothstep(0.85, 1.0, highdef) * color5.z;
        
        //float RVAL = highdef;
        //float GVAL = highdef;
        //float BVAL = highdef;

       	if ( funMode == 1.0 ) {
       		RVAL += vUv.x *(0.5 + sin(time*3.0));
       		GVAL += vUv.y * (0.5 + cos(time*3.0));
       		BVAL += vUv.x * (0.5 + sin(time*3.0));
       	}

		diffuse = vec3(RVAL, GVAL, BVAL);
		

		diffuse = mix(
					mix(
						mix(
							mix(
								mix(color0.rgb,
									color1.rgb,
									smoothstep(0.0,0.2,highdef)),
								color2.rgb,
								smoothstep(0.2,0.4,highdef)),
							color3.rgb,
							smoothstep(0.4,0.6,highdef)),
						color4.rgb,
						smoothstep(0.6,0.8,highdef)),
					color5.rgb,
					smoothstep(0.8,1.0,highdef));
	}
	
	vec3 outgoingLight = vec3( 0.0 );	// outgoing light does not have an alpha, the surface does
	vec4 diffuseColor = vec4( diffuse, opacity );
	

	
<%= THREE.ShaderChunk["logdepthbuf_fragment"] %>
<%= THREE.ShaderChunk["map_fragment"] %>
<%= THREE.ShaderChunk["color_fragment"] %>
<%= THREE.ShaderChunk["alphamap_fragment"] %>
<%= THREE.ShaderChunk["alphatest_fragment"] %>
<%= THREE.ShaderChunk["specularmap_fragment"] %>

<%= THREE.ShaderChunk["lights_phong_fragment"] %>

<%= THREE.ShaderChunk["lightmap_fragment"] %>
<%= THREE.ShaderChunk["envmap_fragment"] %>
<%= THREE.ShaderChunk["shadowmap_fragment"] %>

<%= THREE.ShaderChunk["linear_to_gamma_fragment"] %>

<%= THREE.ShaderChunk["fog_fragment"] %>
gl_FragColor = vec4( outgoingLight, diffuseColor.a );

}
