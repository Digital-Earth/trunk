
/// FragmentState settings ///
#extension GL_OES_standard_derivatives : enable

precision highp float;
precision highp int;

/// Phong settings ///
#define CUSTOM_NORMAL

#define MAX_DIR_LIGHTS 2
#define MAX_POINT_LIGHTS 0
#define MAX_SPOT_LIGHTS 0
#define MAX_HEMI_LIGHTS 0
#define MAX_SHADOWS 2

#define GAMMA_INPUT
#define GAMMA_OUTPUT
#define GAMMA_FACTOR 2


/// FragmentState vertex only definitions ///
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


///REMOVE THIS WHEN POSSILBE
uniform vec4 lightingParams;
uniform float time;

#define EARTH_SCALE 6371007.0


/// HexagonSampling definitions ///
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

	coord = (floor(coord)+findNearestHexagonCoord(fract(coord))+0.5)/244.0;
	coord.y = 1.0 - coord.y;
	return coord;
}

vec4 sampleHexNearest(sampler2D tex,vec2 st)
{
	return texture2D(tex,hexagonTexCoord(st));
}

vec4 sampleHexLinear(sampler2D tex,vec2 st)
{
	st*=243.0;
	vec2 stfloor = floor(st);
	vec2 uv = st-stfloor;
	st = stfloor+0.5;

	vec2 u_step=vec2(1.0,0.0);
	vec2 v_step=vec2(0.0,1.0);

	if (uv.s>uv.t)
	{
		return mix( mix(texture2D(tex,st/244.0), texture2D(tex,(st+u_step)/244.0), uv.s), texture2D(tex,(st+u_step+v_step)/244.0), uv.t);
	}
	else
	{
		return mix( mix(texture2D(tex,st/244.0), texture2D(tex,(st+v_step)/244.0), uv.t), texture2D(tex,(st+u_step+v_step)/244.0), uv.s);
	}
}


/// geosource-8be6a2ec-5110-49cf-a295-1008f8e9a21b vertex only definitions ///
attribute float value_1;


/// geosource-8be6a2ec-5110-49cf-a295-1008f8e9a21b definitions ///
uniform sampler2D texture_1;

vec4 sampleColor_1(vec2 uv) {
    return texture2D(texture_1,uv);
}

float sampleValueNormalized_1(vec2 uv) {
    vec4 color = sampleColor_1(uv);
    float value = (256.0 * color.x +16.0*color.y + color.z) / 256.0;
    return value;
}

float sampleValue_1(vec2 uv) {
    float value = sampleValueNormalized_1(uv);
    value = mix(-20.0,5052.0,value);
    return value;
}

float sampleHex_1(vec2 uv) {
   return sampleValue_1(hexagonTexCoord(uv));
}
uniform vec4 uvTrans_1;

float sampleUvTrans_1(vec2 uv) {
   vec2 uv2 = uv * uvTrans_1.xy + uvTrans_1.zw;
   return sampleHex_1(uv2);
}


/// value-to-normal-geosource-8be6a2ec-5110-49cf-a295-1008f8e9a21b vertex only definitions ///

float sampleValueVertex_1(vec2 uv) {
   return value_1;
}


/// elevation-geosource-8be6a2ec-5110-49cf-a295-1008f8e9a21b definitions ///
uniform float elevationScale_1;


/// graident-geosource-8be6a2ec-5110-49cf-a295-1008f8e9a21b definitions ///
vec4 gradient_1(vec2 uv) {
    float value = sampleUvTrans_1(uv);
    vec4 color = vec4(0.0);
    color += mix(vec4(0.0,0.0,0.0,0.0),vec4(0.0,0.0,0.0,1.0),clamp((value - -20.0)/(0.0 - -20.0),0.0,1.0)) * (1.0 - step(0.0,value));
    color += mix(vec4(0.0,0.0,0.0,1.0),vec4(0.06666666666666667,0.06666666666666667,0.06666666666666667,1.0),clamp((value - 0.0)/(3341.0 - 0.0),0.0,1.0)) * step(0.0,value) * (1.0 - step(3341.0,value));
    color += mix(vec4(0.06666666666666667,0.06666666666666667,0.06666666666666667,1.0),vec4(0.06666666666666667,0.06666666666666667,0.06666666666666667,1.0),clamp((value - 3341.0)/(5052.0 - 3341.0),0.0,1.0)) * step(3341.0,value);
    return color;
}

/// Phong definitions ///
uniform sampler2D tRoughMatCap;
uniform sampler2D tGlossMatCap;
uniform sampler2D tNoiseMap;
uniform float reflectAmount;

// 	 matcap converts cubemap style normal to a 2d spherical reflection matcap
vec2 matcap_1_0(vec3 eye, vec3 normal) {
  vec3 reflected = reflect(eye, normal);

  float m = 2.0 * sqrt(
    pow(reflected.x, 2.0) +
    pow(reflected.y, 2.0) +
    pow(reflected.z + 1.0, 2.0)
  );

  return reflected.xy / m + 0.5;
}


#if MAX_DIR_LIGHTS > 0

	uniform vec3 directionalLightColor[ MAX_DIR_LIGHTS ];
	uniform vec3 directionalLightDirection[ MAX_DIR_LIGHTS ];

#endif

#if MAX_HEMI_LIGHTS > 0

	uniform vec3 hemisphereLightSkyColor[ MAX_HEMI_LIGHTS ];
	uniform vec3 hemisphereLightGroundColor[ MAX_HEMI_LIGHTS ];
	uniform vec3 hemisphereLightDirection[ MAX_HEMI_LIGHTS ];

#endif

#if MAX_POINT_LIGHTS > 0

	uniform vec3 pointLightColor[ MAX_POINT_LIGHTS ];

	uniform vec3 pointLightPosition[ MAX_POINT_LIGHTS ];
	uniform float pointLightDistance[ MAX_POINT_LIGHTS ];

#endif

#if MAX_SPOT_LIGHTS > 0

	uniform vec3 spotLightColor[ MAX_SPOT_LIGHTS ];
	uniform vec3 spotLightPosition[ MAX_SPOT_LIGHTS ];
	uniform vec3 spotLightDirection[ MAX_SPOT_LIGHTS ];
	uniform float spotLightAngleCos[ MAX_SPOT_LIGHTS ];
	uniform float spotLightExponent[ MAX_SPOT_LIGHTS ];

	uniform float spotLightDistance[ MAX_SPOT_LIGHTS ];

#endif

// vWorldPosition already defined above
//#if MAX_SPOT_LIGHTS > 0 || defined( USE_BUMPMAP ) || defined( USE_ENVMAP )

//	varying vec3 vWorldPosition;

//#endif

#ifdef WRAP_AROUND

	uniform vec3 wrapRGB;

#endif

void phong_1(inout vec4[10] _fragment_state) {
    vec3 viewPosition = normalize( vViewPosition );
    vec3 normal = _fragment_state[1].xyz;
    vec3 diffuse = _fragment_state[4].xyz * _fragment_state[0].xyz;
    vec3 specular = _fragment_state[3].xyz;
    float shininess = _fragment_state[7].y;
    float specularStrength = _fragment_state[7].z;
#ifndef CUSTOM_NORMAL
	#ifndef FLAT_SHADED

		// inline phong lighting calculations
		vec3 normal = normalize( vNormal );
		vec3 viewPosition = normalize( vViewPosition );


		#ifdef DOUBLE_SIDED

			normal = normal * ( -1.0 + 2.0 * float( gl_FrontFacing ) );

		#endif

		#ifdef USE_NORMALMAP

			normal = perturbNormal2Arb( -vViewPosition, normal );

		#elif defined( USE_BUMPMAP )

			normal = perturbNormalArb( -vViewPosition, normal, dHdxy_fwd() );

		#endif

	#else
		vec3 fdx = dFdx( vViewPosition );
		vec3 fdy = dFdy( vViewPosition );
		vec3 normal = normalize( cross( fdx, fdy ) );

		vec3 viewPosition = normalize( vViewPosition );
	#endif
#endif




#if MAX_POINT_LIGHTS > 0

	vec3 pointDiffuse = vec3( 0.0 );
	vec3 pointSpecular = vec3( 0.0 );

	for ( int i = 0; i < MAX_POINT_LIGHTS; i ++ ) {

		vec4 lPosition = viewMatrix * vec4( pointLightPosition[ i ], 1.0 );
		vec3 lVector = lPosition.xyz + vViewPosition.xyz;

		float lDistance = 1.0;
		if ( pointLightDistance[ i ] > 0.0 )
			lDistance = 1.0 - min( ( length( lVector ) / pointLightDistance[ i ] ), 1.0 );

		lVector = normalize( lVector );

				// diffuse

		float dotProduct = dot( normal, lVector );

		#ifdef WRAP_AROUND

			float pointDiffuseWeightFull = max( dotProduct, 0.0 );
			float pointDiffuseWeightHalf = max( 0.5 * dotProduct + 0.5, 0.0 );

			vec3 pointDiffuseWeight = mix( vec3( pointDiffuseWeightFull ), vec3( pointDiffuseWeightHalf ), wrapRGB );

		#else

			float pointDiffuseWeight = max( dotProduct, 0.0 );

		#endif

		pointDiffuse += diffuse * pointLightColor[ i ] * pointDiffuseWeight * lDistance;

				// specular

		vec3 pointHalfVector = normalize( lVector + viewPosition );
		float pointDotNormalHalf = max( dot( normal, pointHalfVector ), 0.0 );
		float pointSpecularWeight = specularStrength * max( pow( pointDotNormalHalf, shininess ), 0.0 );

		float specularNormalization = ( shininess + 2.0 ) / 8.0;

		vec3 schlick = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( lVector, pointHalfVector ), 0.0 ), 5.0 );
		pointSpecular += schlick * pointLightColor[ i ] * pointSpecularWeight * pointDiffuseWeight * lDistance * specularNormalization;

	}

#endif

#if MAX_SPOT_LIGHTS > 0

	vec3 spotDiffuse = vec3( 0.0 );
	vec3 spotSpecular = vec3( 0.0 );

	for ( int i = 0; i < MAX_SPOT_LIGHTS; i ++ ) {

		vec4 lPosition = viewMatrix * vec4( spotLightPosition[ i ], 1.0 );
		vec3 lVector = lPosition.xyz + vViewPosition.xyz;

		float lDistance = 1.0;
		if ( spotLightDistance[ i ] > 0.0 )
			lDistance = 1.0 - min( ( length( lVector ) / spotLightDistance[ i ] ), 1.0 );

		lVector = normalize( lVector );

		float spotEffect = dot( spotLightDirection[ i ], normalize( spotLightPosition[ i ] - vWorldPosition ) );

		if ( spotEffect > spotLightAngleCos[ i ] ) {

			spotEffect = max( pow( max( spotEffect, 0.0 ), spotLightExponent[ i ] ), 0.0 );

					// diffuse

			float dotProduct = dot( normal, lVector );

			#ifdef WRAP_AROUND

				float spotDiffuseWeightFull = max( dotProduct, 0.0 );
				float spotDiffuseWeightHalf = max( 0.5 * dotProduct + 0.5, 0.0 );

				vec3 spotDiffuseWeight = mix( vec3( spotDiffuseWeightFull ), vec3( spotDiffuseWeightHalf ), wrapRGB );

			#else

				float spotDiffuseWeight = max( dotProduct, 0.0 );

			#endif

			spotDiffuse += diffuse * spotLightColor[ i ] * spotDiffuseWeight * lDistance * spotEffect;

					// specular

			vec3 spotHalfVector = normalize( lVector + viewPosition );
			float spotDotNormalHalf = max( dot( normal, spotHalfVector ), 0.0 );
			float spotSpecularWeight = specularStrength * max( pow( spotDotNormalHalf, shininess ), 0.0 );

			float specularNormalization = ( shininess + 2.0 ) / 8.0;

			vec3 schlick = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( lVector, spotHalfVector ), 0.0 ), 5.0 );
			spotSpecular += schlick * spotLightColor[ i ] * spotSpecularWeight * spotDiffuseWeight * lDistance * specularNormalization * spotEffect;

		}

	}

#endif

#if MAX_DIR_LIGHTS > 0

	vec3 dirDiffuse = vec3( 0.0 );
	vec3 dirSpecular = vec3( 0.0 );

	for( int i = 0; i < MAX_DIR_LIGHTS; i ++ ) {

		vec4 lDirection = viewMatrix * vec4( directionalLightDirection[ i ], 0.0 );
		vec3 dirVector = normalize( lDirection.xyz );

				// diffuse

		float dotProduct = dot( normal, dirVector );

		#ifdef WRAP_AROUND

			float dirDiffuseWeightFull = max( dotProduct, 0.0 );
			float dirDiffuseWeightHalf = max( 0.5 * dotProduct + 0.5, 0.0 );

			vec3 dirDiffuseWeight = mix( vec3( dirDiffuseWeightFull ), vec3( dirDiffuseWeightHalf ), wrapRGB );

		#else

			float dirDiffuseWeight = max( dotProduct, 0.0 );

		#endif

		dirDiffuse += diffuse * directionalLightColor[ i ] * dirDiffuseWeight;

		// specular

		vec3 dirHalfVector = normalize( dirVector + viewPosition );
		float dirDotNormalHalf = max( dot( normal, dirHalfVector ), 0.0 );
		float dirSpecularWeight = specularStrength * max( pow( dirDotNormalHalf, shininess ), 0.0 );

		/*
		// fresnel term from skin shader
		const float F0 = 0.128;

		float base = 1.0 - dot( viewPosition, dirHalfVector );
		float exponential = pow( base, 5.0 );

		float fresnel = exponential + F0 * ( 1.0 - exponential );
		*/

		/*
		// fresnel term from fresnel shader
		const float mFresnelBias = 0.08;
		const float mFresnelScale = 0.3;
		const float mFresnelPower = 5.0;

		float fresnel = mFresnelBias + mFresnelScale * pow( 1.0 + dot( normalize( -viewPosition ), normal ), mFresnelPower );
		*/

		float specularNormalization = ( shininess + 2.0 ) / 8.0;

		// 		dirSpecular += specular * directionalLightColor[ i ] * dirSpecularWeight * dirDiffuseWeight * specularNormalization * fresnel;

		vec3 schlick = specular + vec3( 1.0 - specular ) * pow( max( 1.0 - dot( dirVector, dirHalfVector ), 0.0 ), 5.0 );
		dirSpecular += schlick * directionalLightColor[ i ] * dirSpecularWeight * dirDiffuseWeight * specularNormalization;


	}

#endif


vec3 totalDiffuse = vec3( 0.0 );
vec3 totalSpecular = vec3( 0.0 );

#if MAX_DIR_LIGHTS > 0

	totalDiffuse += dirDiffuse;
	totalSpecular += dirSpecular;

#endif

#if MAX_HEMI_LIGHTS > 0

	totalDiffuse += hemiDiffuse;
	totalSpecular += hemiSpecular;

#endif

#if MAX_POINT_LIGHTS > 0

	totalDiffuse += pointDiffuse;
	totalSpecular += pointSpecular;

#endif

#if MAX_SPOT_LIGHTS > 0

	totalDiffuse += spotDiffuse;
	totalSpecular += spotSpecular;

#endif



vec2 matcapUV = matcap_1_0( vEyeVector, normal);
vec3 roughMap = texture2D( tRoughMatCap, matcapUV ).rgb;
vec3 glossMap = texture2D( tGlossMatCap, matcapUV ).rgb * 0.56;
float roughnessStrength = 0.5;

float noiseVal = texture2D( tNoiseMap, vUv ).r;
// yes this is magic for the moment
roughMap += vec3(1.0, 1.0, 1.0) * 0.1 * (-0.5 + noiseVal);

totalDiffuse += glossMap*(1.0 - roughnessStrength) + roughMap * roughnessStrength;


    _fragment_state[0].xyz = mix(_fragment_state[0].xyz,totalDiffuse,_fragment_state[7].x) + totalSpecular;
}

void main(void) {

/// FragmentState code ///
float elevationValue = 0.0; 

//make sure vUv is setup before the rest of later ocde
vUv=uv;

/// elevation-geosource-8be6a2ec-5110-49cf-a295-1008f8e9a21b code ///
elevationValue = sampleValueVertex_1(vUv) * elevationScale_1;


/// FragmentState code ///
//float heightFactor = _fragment_state[8].x / EARTH_SCALE + 1.0;
float heightFactor = elevationValue / EARTH_SCALE + 1.0;

vec3 heightmapped_position = position * heightFactor;
vec4 mvPosition = modelViewMatrix * vec4( heightmapped_position, 1.0 );
vNormal = normalize(normal);

gl_Position = projectionMatrix * mvPosition;
vViewPosition = -mvPosition.xyz;

vec4 worldPosition = modelMatrix * vec4( heightmapped_position, 1.0 );

vWorldPosition = worldPosition.xyz;

vEyeVector = normalize( vec3( modelViewMatrix * vec4( heightmapped_position, 1.0 ) ) );


}