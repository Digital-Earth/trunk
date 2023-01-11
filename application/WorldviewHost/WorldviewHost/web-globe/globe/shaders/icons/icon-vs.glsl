

attribute float size;
attribute float opacity;
attribute vec3 ca;

varying float vOpacity;
varying vec3 vColor;

//  TODO :: something more custom

void main() {

	vColor = ca;
	vOpacity = opacity;

	vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );

	gl_PointSize = size;
	// gl_PointSize = size * ( 300.0 / length( mvPosition.xyz ) );

	gl_Position = projectionMatrix * mvPosition;

}