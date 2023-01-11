
float textureMiplevel(vec2 uv, vec2 textureSize)
{
  //rate of change of the pixels in u and v with respect to window space
  //approximate to au/ax, au/ay, av/ax, av/ay
  vec2 dx = dFdx( uv * textureSize.x);
  vec2 dy = dFdy( uv * textureSize.y);
  
  //select the LOD based on the maximum compression of an edge in texture space.
  //This corresponds to the maximum length of a side in texture space
  //max (sqrt(dUdx*dUdx + dVdx*dVdx),
  //    sqrt(dUdy*dUdy + dVdy*dVdy));
  float d = max( dot (dx, dx), dot( dy, dy) );
  
  //convert d length to power-of-two level of detail
  //formula is changed because we use 1 to 9 instead of 1 to 4
  //now just played with numbers, so this numbers are linked to rhombus.coffee calculateScreenSize function 
  return 0.80*log(d) / log(4.0);
  //return 0.5*log2(d);
}