uniform sampler2D fb0;
uniform sampler2D fb1;
uniform sampler2D fb2;
uniform sampler2D fb3;
uniform vec4 fbMask;

varying vec2 vUv;

void main() {
	vec3 color =  texture2D( fb0, vUv ).rgb * fbMask.x 
				+ texture2D( fb1, vUv ).rgb * fbMask.y 
				+ texture2D( fb2, vUv ).rgb * fbMask.z 
				+ texture2D( fb3, vUv ).rgb * fbMask.w;
	float avg = fbMask.x + fbMask.y + fbMask.z + fbMask.w;
	gl_FragColor = vec4(color / avg, 1.0);
}