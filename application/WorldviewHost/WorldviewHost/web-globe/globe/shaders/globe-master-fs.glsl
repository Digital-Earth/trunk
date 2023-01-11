#extension GL_OES_standard_derivatives : enable

precision highp float;
precision highp int;

#define CUSTOM_NORMAL

#define MAX_DIR_LIGHTS 2
#define MAX_POINT_LIGHTS 0
#define MAX_SPOT_LIGHTS 0
#define MAX_HEMI_LIGHTS 0
#define MAX_SHADOWS 2

#define GAMMA_INPUT
#define GAMMA_OUTPUT
#define GAMMA_FACTOR 2



//  NOW USING RawShaderMaterial, which doesn't inject anything above


uniform mat4 viewMatrix;
uniform vec3 cameraPosition;

#define EARTH_SCALE 6371007.0

uniform float time;

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

uniform vec4 t0uvTrans;
uniform vec4 t1uvTrans;
uniform vec4 t2uvTrans;
uniform vec4 t3uvTrans;
uniform vec4 t4uvTrans;
uniform vec4 t5uvTrans;
uniform vec4 t6uvTrans;
uniform vec4 t7uvTrans;
uniform vec4 t8uvTrans;
uniform vec4 t9uvTrans;

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
uniform float cutoff;

uniform int masktest;
uniform sampler2D tRoughMatCap;
uniform sampler2D tGlossMatCap;
uniform sampler2D tNoiseMap;
uniform sampler2D tTest;
uniform samplerCube tEnvMap;

uniform float reflectAmount;
uniform vec3 globeBackground;
uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 color3;
uniform vec3 color4;
uniform vec3 color5;

uniform vec3 ambientLightColor;
uniform float isHighDefRGB;
uniform float loading;

// some elevation related uniforms
uniform float useElevation;
uniform float elevationMultichannel;
uniform vec2 elevationRange;
uniform sampler2D elevationMap;
uniform vec4 elevationMapUvTrans;
uniform float heightMultiplier;
uniform float normalmap_strength;
uniform vec4 fogColor;
uniform vec3 fogParams;//x - height fog end offset, y - fog dist scale, z - camera apply coef

uniform vec4 lightingParams;


//  ------- EJS template includes ---------
//  NOTE:  moved glslify step into the libs directory because
//         the compiler breaks on any EJS tags
<%= libs() %>

<%= phongHeader() %>



vec2 findNearestHexagonCoord(vec2 coord)
{
	if (coord[0]>coord[1])
	{
		if (coord[0]<0.5-coord[1]/2.0)
			return vec2(0.0,0.0);
		else if (coord[1]>1.0-coord[0]/2.0)
			return vec2(1.0,1.0);
		else
		return vec2(1.0,0.0);
	}
	else
	{
		if (coord[1]<0.5-coord[0]/2.0)
			return vec2(0.0,0.0);
		else if (coord[0]>1.0-coord[1]/2.0)
			return vec2(1.0,1.0);
		else
			return vec2(0.0,1.0);
	}
}

vec2 hexagonTexCoord(vec2 coord)
{
	coord.y = 1.0 - coord.y;
	coord*=243.0;

	coord = (floor(coord)+findNearestHexagonCoord(fract(coord)))/243.0;
	coord.y = 1.0 - coord.y;
	return coord;
}

vec4 sampleHexLinear(sampler2D tex,vec2 st)
{
	st*=244.0;
	vec2 stfloor = floor(st);
	vec2 uv = st-stfloor;
	st = stfloor+0.5;

	vec2 u_step=vec2(1.0,0.0);
	vec2 v_step=vec2(0.0,1.0);

	if (uv[0]+uv[1] < 1.0)
	{
		return texture2D(tex, st/244.0)*(1.0-uv[0]-uv[1])+texture2D(tex, (st+u_step)/244.0)*(uv[0])+texture2D(tex, (st+v_step)/244.0)*(uv[1]);
	}
	else
	{
		uv = 1.0-uv;
		return texture2D(tex, (st + u_step+v_step)/244.0)*(1.0-uv[0]-uv[1])+texture2D(tex, (st+u_step)/244.0)*(uv[1])+texture2D(tex, (st+v_step)/244.0)*(uv[0]);
	}
}




float COLOR_MAX = 256.0 * 256.0;

float sample_elevation(sampler2D s, vec2 uv)
{
	float height = 1.0;

	vec4 elevation = texture2D(s, uv);
	
	if ( elevationMultichannel == 1.0 ){
		float highdef = (256.0 * elevation.x + 16.0 * elevation.y + elevation.z) / 256.0;
		height = highdef;//1.0 + heightMultiplier * mix(elevationRange.x,elevationRange.y,highdef) / EARTH_SCALE;			
	} else {
		// treat as grayscale image.. we may want a single channel option
		float lowdef = (elevation.x + elevation.y + elevation.z) / 3.0;
		height = lowdef;//1.0 + heightMultiplier * mix(elevationRange.x,elevationRange.y,lowdef) / EARTH_SCALE;
	}
	return height;
}

mat3 cotangentFrame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = 1.0 / sqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}


void main(void)
{
	vec3 c;

	vec2 t0Uv = vUv * t0uvTrans.xy + t0uvTrans.zw;
	vec2 t1Uv = vUv * t1uvTrans.xy + t1uvTrans.zw;
	vec2 t2Uv = vUv * t2uvTrans.xy + t2uvTrans.zw;
	vec2 t3Uv = vUv * t3uvTrans.xy + t3uvTrans.zw;
	vec2 t4Uv = vUv * t4uvTrans.xy + t4uvTrans.zw;
	vec2 t5Uv = vUv * t5uvTrans.xy + t5uvTrans.zw;
	vec2 t6Uv = vUv * t6uvTrans.xy + t6uvTrans.zw;
	vec2 t7Uv = vUv * t7uvTrans.xy + t7uvTrans.zw;
	vec2 t8Uv = vUv * t8uvTrans.xy + t8uvTrans.zw;
	vec2 t9Uv = vUv * t9uvTrans.xy + t9uvTrans.zw;

	vec4 c0 = texture2D(t0, hexagonTexCoord(t0Uv));
	vec4 c1 = texture2D(t1, hexagonTexCoord(t1Uv));
	vec4 c2 = texture2D(t2, hexagonTexCoord(t2Uv));
	vec4 c3 = texture2D(t3, hexagonTexCoord(t3Uv));
	vec4 c4 = texture2D(t4, hexagonTexCoord(t4Uv));
	vec4 c5 = texture2D(t5, hexagonTexCoord(t5Uv));
	vec4 c6 = texture2D(t6, hexagonTexCoord(t6Uv));
	vec4 c7 = texture2D(t7, hexagonTexCoord(t7Uv));
	vec4 c8 = texture2D(t8, hexagonTexCoord(t8Uv));
	vec4 c9 = texture2D(t9, hexagonTexCoord(t9Uv));
	
	vec4 col = vec4(0.0);
	vec4 curCol = vec4(0.0);
	//vec4 lighting = vec4(0.0);
	//vec4 data = vec4(0.0);
	//vec3 ambient = vec3(lighting.w);  //0.1,0.1,0.1);
	//vec3 emissive = lighting.z * col.xyz;
	vec3 specular = vec3(lightingParams.x);  //1.5,0.5,0.5);
	vec3 diffuse = vec3(lightingParams.z);
	float shininess = lightingParams.y;
	float specularStrength = 1.0;
	vec4 fog = vec4(fogColor.xyz,0.0);

	// THESE are layer template variables
	int layerIndex = 0;
	vec4 curTexture = c0;
	float highdef = 0.0;

	if (loading == 1.0) {
		c0 = vec4(0.3, 0.3, 0.3, 1.0);
	}
	
	// if( col.a < cutoff)
	// 	discard;
	gl_FragColor=vec4(1.0);	

	//use per face normals
#if 0
	vec3 fdx = dFdx( vViewPosition );
	vec3 fdy = dFdy( vViewPosition );
	vec3 normal = normalize( cross( fdx, fdy ) );
#else
	vec3 normal = normalize( vNormal );
#endif

	if ( useElevation == 1.0 )
	{
		vec2 eleUv = vUv * elevationMapUvTrans.xy + elevationMapUvTrans.zw;

		vec2 fuvdx = dFdx( eleUv );
		vec2 fuvdy = dFdy( eleUv );
		float elevationScale = 20.0 * normalmap_strength;//we need some coef to scale elevation

		float s0 = sample_elevation(elevationMap, eleUv);//elevation = texture2D(t0, uv);
		float s1 = sample_elevation(elevationMap, eleUv + fuvdy);
		float s2 = sample_elevation(elevationMap, eleUv + fuvdx);
		float s3 = sample_elevation(elevationMap, eleUv - fuvdy);
		float s4 = sample_elevation(elevationMap, eleUv - fuvdx);

		//TEMP solution for removing hq noise on big elevation differences
		float maxHeightDiff = 0.04;

		vec3 va = vec3( 0.0, 1.0, -clamp((s1 - s0), 0.0, maxHeightDiff)*elevationScale );
	    vec3 vb = vec3( 1.0, 0.0,  -clamp((s2 - s0), 0.0, maxHeightDiff)*elevationScale ); 
	    vec3 vc = vec3( 0.0, -1.0,  -clamp((s3 - s0), 0.0, maxHeightDiff)*elevationScale );
	    vec3 vd = vec3( -1.0, 0.0,  -clamp((s4 - s0), 0.0, maxHeightDiff)*elevationScale );

	    vec3 tNormal = normalize(( cross(va, vb) + cross(vb, vc) + cross(vc, vd) + cross(vd, va) ) / -4.0);
    	mat3 TBN = cotangentFrame(normal, -vViewPosition, eleUv);
    	normal = normalize( TBN * tNormal );

		float highdef = 1.0 + mix(elevationRange.x,elevationRange.y,s0) / EARTH_SCALE;
		float fogHeightBias = fogParams.x / EARTH_SCALE;
		fog.w = clamp(exp(-(highdef-1.0 - fogHeightBias) * pow(10.0,fogParams.y)), 0.0, 1.0);//apply fog from heightmap
		fog.w *= (exp(-vViewPosition.z / pow(10.0, fogParams.z)));//apply attenuation from camera(more closer dist - more fog)
		fog.w *= fogColor.w;//apply fog intensity
	}

	vec3 viewPosition = normalize( vViewPosition );


	


// BEGIN LAYERS
// and render the corresponding shader partial

<% _.each(layers, function(layer, i){ %>


	layerIndex = <%= i %>;
	curTexture = c<%= i %>;

	<%= renderLayer(layer, i) %>

	//----------------------------- 


<%}) %>

// ----------- 
// END LAYERS

	diffuse *= col.xyz;

	<%= phongBody() %>

	<%= envMap() %>

	// now combine everything together
	col.a = clamp(col.a, 0.0, 1.0);
	float alphaMix = col.a;
    vec3 alphaColor = (1.0 - col.a) * globeBackground + roughMap;

    totalDiffuse = mix(col.xyz, totalDiffuse, lightingParams.w);
    totalSpecular*=lightingParams.w;
    gl_FragColor.xyz = (gl_FragColor.xyz * ( totalDiffuse + glossMap + totalSpecular))*alphaMix + alphaColor*(1.0 - alphaMix);
	gl_FragColor.xyz = mix(gl_FragColor.xyz,fog.xyz,fog.w);
}
