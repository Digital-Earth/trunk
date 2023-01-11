uniform sampler2D tDiffuse;
uniform vec2 resolution;
uniform float blendValue;

varying vec2 vUv;

void main() {
	vec4 color = texture2D( tDiffuse, vUv );
	color.a = blendValue;
	gl_FragColor = color;
}