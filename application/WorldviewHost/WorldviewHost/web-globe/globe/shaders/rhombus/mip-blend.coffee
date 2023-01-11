_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
FragmentState = require('../jit/fragment-state')
TextureMiplevel = require('../jit/texture-mip-level')
GeoSource = require('../jit/geo-source')

###
	Module for blending separate geosources values depending on the mip level
	
	sources - array of geoSources 
###
class MipBlend extends Jit.JitModule
	constructor: (@sources) ->
		super("mipblend")
		if @sources[0].type == "vec4"
			@useAlphaBlend = true 
	gradient: (palette) ->
		return new GeoSource.Gradient(this,palette)
		
	asNormal: (factor) ->
		return @sources[0].asNormal(factor)

	compile: (shader) ->
		float = shader.require("GLSL").float
		sources = _.map(@sources, (source) ->  shader.require(source))
		textureMiplevel = shader.require('TextureMiplevel').textureMiplevel

		functionName = shader.getUniqueVariable("mipblend");

		functionCode = []
		functionCode.push "#{@sources[0].type} #{functionName}(vec2 uv) {"
		functionCode.push "    #{@sources[0].type} result = #{sources[0].sample}(uv);" 
		functionCode.push "    float mipValue = #{textureMiplevel}(uv, vec2(244.0,244.0));"
		functionCode.push "    mipValue = max(0.0, min(mipValue, #{float(sources.length-1)}));"
		
		functionCode.push "    if(mipValue == 0.0){"
		#functionCode.push "        output = output;"
		
		for i in [1...sources.length]
			functionCode.push "    } else if(mipValue < #{float(i)}) {"
			if @useAlphaBlend
				functionCode.push "        vec4 c0 = #{sources[i-1].sample}(uv);"
				functionCode.push "        vec4 c1 = #{sources[i].sample}(uv);"
				functionCode.push "        c0.rgb*=c0.a; c1.rgb*=c1.a;"
				functionCode.push "        result = mix(c0,c1,mipValue);"
				functionCode.push "        result.rgb/=max(result.a,0.0001);"
			else
				functionCode.push "        result = mix(#{sources[i-1].sample}(uv), #{sources[i].sample}(uv), mipValue);"
			functionCode.push "    } else if(mipValue == #{float(i)}) {"
			functionCode.push "        result = #{sources[i].sample}(uv);"
			
		functionCode.push "    }"
		functionCode.push "    return result;"
		functionCode.push "}"
		
		exports = {}
		_.extend exports, sources[0]

		exports.sample = functionName
		delete exports.ready
		delete exports.load
		delete exports.dispose

		return {
			definitions: functionCode.join "\n"
			exports: exports
		}

module.exports = MipBlend