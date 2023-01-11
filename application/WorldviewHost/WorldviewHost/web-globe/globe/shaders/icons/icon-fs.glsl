uniform vec3 color;
uniform vec4 borderColorAndIconAlpha;
uniform sampler2D texture;


varying float vOpacity;
varying vec3 vColor;

void main() {
	// we need to flip it in the y-direction
    vec4 sample = texture2D( texture, vec2(gl_PointCoord.x, 1.0 - gl_PointCoord.y));

	vec4 color = vec4( color * vColor, vOpacity ) * sample;
	float colorVal = (sample.x + sample.y + sample.z) / 3.0;

    color.xyz = color.xyz + clamp(1.0 - colorVal*1.0, 0.0, 1.0) * borderColorAndIconAlpha.rgb;
    
        
	if ( color.w < 0.03 ) discard;

	color.w *= borderColorAndIconAlpha.a;

	gl_FragColor = color;

}