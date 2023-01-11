//   --------  GLSLify includes  -------------
//   list all includes here, and manually reference
//
//   this kind of defeats the purpose of glslify so we may want to take it out

// 	 matcap converts cubemap style normal to a 2d spherical reflection matcap
#pragma glslify: matcap = require(matcap)


// pass in a UV and get noise
float random_noise(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}