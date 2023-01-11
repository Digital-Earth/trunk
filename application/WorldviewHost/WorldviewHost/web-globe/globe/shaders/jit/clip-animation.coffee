_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
FragmentState = require('../jit/fragment-state')



#Utility class for clip animation
#Clipping is based on normalized Y Axis value in Camera space
#input parameters:
#	clipData.x - sign value, can be -1.0, 0.0, 1.0 - (clip below value, noclipping, clip above value)
#   clipData.y - clipping value, can be [-1.0..1.0]
#   clipData.z - animation power value
class ClipAnimation extends Jit.JitModule
	constructor: (@source) ->
		super("clip-animation")


	compile: (shader) ->
		settings = """
uniform vec3 clipData;
"""

		return {
			fragmentSettings:settings
			fragmentBody: """
vec4 vWorld = viewMatrix * vec4((vWorldPosition.xyz/ EARTH_SCALE),0.0) ;
if(vWorld.y*clipData.x > clipData.y*clipData.x)
	discard;
			"""
		}

module.exports = ClipAnimation
