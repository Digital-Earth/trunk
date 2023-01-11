uniform vec2 resolution;
uniform float noise;

uniform vec3 backgroundColor;
uniform vec3 backgroundColor2;

#define VIG_REDUCTION_POWER 0.85
#define VIG_BOOST 1.4

float random(vec3 scale,float seed){return fract(sin(dot(gl_FragCoord.xyz+seed,scale))*43758.5453+seed);}

void main() {
    //vec3 color = vec3( 234. / 255. );
    // vec2 center = resolution * 0.5;
    // float vignette = distance( center, gl_FragCoord.xy ) / resolution.x;
    // vignette = VIG_BOOST - vignette * VIG_REDUCTION_POWER;

    // float n = noise * ( .5 - random( vec3( 1. ), length( gl_FragCoord ) ) );

    // define gradient from lower left corner to upper right 
    float pVal = (gl_FragCoord.x / resolution.x) * 0.5 + (gl_FragCoord.y / resolution.y) * 0.5;
    vec3 c = backgroundColor * (1.0 - pVal) + backgroundColor2 * pVal;

    gl_FragColor = vec4(c, 1.0);
}