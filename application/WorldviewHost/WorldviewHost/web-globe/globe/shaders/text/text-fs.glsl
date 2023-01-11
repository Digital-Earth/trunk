#define SQRT2 1.4142135623730951
#define SMOOTH_MULT 0.00000042
uniform float opacity;
uniform vec3 color;
uniform vec3 borderColor;
uniform sampler2D map;
uniform float smooth;
uniform float border;
uniform float borderAlpha;

varying vec2 vUv;
void main() {
	vec4 texColor = texture2D(map, vUv);

	float dst = texColor.a;
	float afwidth = SMOOTH_MULT * smooth * SQRT2 / (2.0 * gl_FragCoord.w);
	float bufferAmount = 0.5;

	float alpha = smoothstep(bufferAmount - afwidth, bufferAmount + afwidth, dst);
	vec3 c2 = color;

	#ifdef USE_BORDER
		bufferAmount = 0.25;
		float alpha2 = smoothstep(bufferAmount - afwidth, bufferAmount + afwidth, dst);

		alpha2 = alpha + (1.0 - alpha) * alpha2 * borderAlpha;

		// alpha is higher order, alpha2 is border
		c2 = (color * alpha + borderColor * (1.0 - alpha)); 
		alpha = alpha2;
	#endif


	gl_FragColor = vec4(c2, opacity * alpha);

	#ifdef ALPHATEST
		if ( gl_FragColor.a < ALPHATEST ) discard;
	#endif
}