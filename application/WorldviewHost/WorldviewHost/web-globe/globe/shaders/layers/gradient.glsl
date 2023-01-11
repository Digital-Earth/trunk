// This code snippet will be included in the globe-master body, the following variables will be available:
// 		int layerIndex - layer index
// 		vec4 curTexture - texture sampler for the current layer     (using template to swap in variable name)
//      vec3 col
// 		float alpha
// 		vec3 diffuse
// 		vec3 normal
// 		vec3 viewPosition
// 		vec3 emissive
// 		vec3 totalDiffuse
// 		vec3 totalSpecular
// 		vec3 envMap
// 		vec3 ambientLightColor
// 		vec3 ambient


// TODO :: use logic to build gradient from Style
// had to remove float definition
highdef = (256.0 * curTexture.x +16.0*curTexture.y + curTexture.z) / 256.0;

<%= code %>