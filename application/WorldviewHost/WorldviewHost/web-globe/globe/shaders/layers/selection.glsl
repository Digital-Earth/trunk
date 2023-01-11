// This code snippet will be included in the globe-master body, the following variables will be available:
// 		int layerIndex - layer index
// 		vec4 curTexture - texture sampler for the current layer     (using template to swap in variable name)
//      vec3 curCol
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

// 0.7 multiplier because otherwise we are overshooting 1


// SELECTION LAYER
// for now we just have a simple line animation

curCol.xyz = curTexture.xyz * 1.0;
curCol.a = curTexture.a * 0.25 * (1.0 + 0.3*sin((vEyeVector.y + vEyeVector.x * 0.5 + time.x*0.003)*1200.0));

col = mix(col, curCol, curCol.a);