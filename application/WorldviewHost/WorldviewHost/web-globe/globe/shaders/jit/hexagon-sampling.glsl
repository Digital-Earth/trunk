vec2 findNearestHexagonCoord(vec2 coord)
{
	if (coord[0]>coord[1])
	{
		if (coord[0]<0.5-coord[1]/2.0)
			return vec2(0.0,0.0);
		else if (coord[1]>1.0-coord[0]/2.0)
			return vec2(1.0,1.0);
		else
		return vec2(1.0,0.0);
	}
	else
	{
		if (coord[1]<0.5-coord[0]/2.0)
			return vec2(0.0,0.0);
		else if (coord[0]>1.0-coord[1]/2.0)
			return vec2(1.0,1.0);
		else
			return vec2(0.0,1.0);
	}
}

vec2 hexagonTexCoord(vec2 coord)
{
	coord.y = 1.0 - coord.y;
	coord*=243.0;

	coord = (floor(coord)+findNearestHexagonCoord(fract(coord))+0.5)/244.0;
	coord.y = 1.0 - coord.y;
	return coord;
}

vec4 sampleHexNearest(sampler2D tex,vec2 st)
{
	return texture2D(tex,hexagonTexCoord(st));
}

vec4 sampleHexLinear(sampler2D tex,vec2 st)
{
	st*=243.0;
	vec2 stfloor = floor(st);
	vec2 uv = st-stfloor;
	st = stfloor+0.5;

	vec2 u_step=vec2(1.0,0.0);
	vec2 v_step=vec2(0.0,1.0);

	if (uv.s>uv.t)
	{
		return mix( mix(texture2D(tex,st/244.0), texture2D(tex,(st+u_step)/244.0), uv.s), texture2D(tex,(st+u_step+v_step)/244.0), uv.t);
	}
	else
	{
		return mix( mix(texture2D(tex,st/244.0), texture2D(tex,(st+v_step)/244.0), uv.t), texture2D(tex,(st+u_step+v_step)/244.0), uv.s);
	}
}
