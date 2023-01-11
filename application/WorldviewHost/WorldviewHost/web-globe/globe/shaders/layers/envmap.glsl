



// vec3 r = reflect( viewPosition, normal );
// float m = 2.0 * sqrt( r.x * r.x + r.y * r.y + ( r.z + 1.0 ) * ( r.z+1.0 ) );
// vec2 calculatedNormal = vec2( r.x / m + 0.5,  r.y / m + 0.5 );


// TODO :: sort out glslify includes
vec2 matcapUV = matcap_1_0( vEyeVector, normal);
vec3 roughMap = texture2D( tRoughMatCap, matcapUV ).rgb * reflectAmount;
vec3 glossMap = texture2D( tGlossMatCap, matcapUV ).rgb * reflectAmount * 0.56;

float noiseVal = texture2D( tNoiseMap, vUv ).r;
roughMap += vec3(1.0, 1.0, 1.0) * 0.1 * (-0.5 + noiseVal);
//envMap.r += calculatedNormal.x;
//envMap.g += calculatedNormal.y;

// vec3 envMap = textureCube( tEnvMap, normal ).xyz;
//vec3 envMap = vec3(1.0, 0.0, 0.0);



// top highlight
// envMap.r += normal.y * (1.0 - totalDiffuse.r) * 0.42;
// envMap.g += normal.y * (1.0 - totalDiffuse.g) * 0.4;
// envMap.b += normal.y * (1.0 - totalDiffuse.b) * 0.4;

// bottom highlight
// envMap.r += (0.6 - normal.y) * (1.0 - totalDiffuse.r) * 0.42;
// envMap.g += (0.6 - normal.y) * (1.0 - totalDiffuse.g) * 0.4;
// envMap.b += (0.6 - normal.y) * (1.0 - totalDiffuse.b) * 0.4;