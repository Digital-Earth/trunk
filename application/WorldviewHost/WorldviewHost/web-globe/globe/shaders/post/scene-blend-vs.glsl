uniform sampler2D tDiffuse;
uniform vec2 resolution;
uniform float blendValue;

varying vec2 vUv;

void main() {

	//vUv = uv;
	vUv = position.xy * vec2(0.5,0.5) + vec2(0.5,0.5);
	gl_Position = vec4(position.xy,0,1);
}