uniform sampler2D tDiffuse;
uniform float tiltshiftValue;
uniform float tiltshiftRes;

uniform vec3 gammaValue;
uniform vec3 colorTint;
uniform vec3 colorBoost;

varying vec2 vUv;

void main() {

	vec4 sum = vec4( 0.0 );

	float vv = tiltshiftValue * abs( tiltshiftRes - vUv.y );

	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 4.0 * vv ) ) * 0.051;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 3.0 * vv ) ) * 0.0918;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 2.0 * vv ) ) * 0.12245;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y - 1.0 * vv ) ) * 0.1531;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y ) ) * 0.1633;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 1.0 * vv ) ) * 0.1531;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 2.0 * vv ) ) * 0.12245;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 3.0 * vv ) ) * 0.0918;
	sum += texture2D( tDiffuse, vec2( vUv.x, vUv.y + 4.0 * vv ) ) * 0.051;

	vec3 colorMultiplier = colorTint * 2.0;

	sum.rgb = colorMultiplier * pow( ( sum.rgb + colorBoost ), gammaValue ); 

	gl_FragColor = sum;

}