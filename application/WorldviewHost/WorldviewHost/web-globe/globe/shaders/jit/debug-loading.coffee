_ = require 'underscore'
Jit = require('../../jit-shader')



###
	Debug allows a component to be visualized on top of existing shaders.
	Right now it is strictly a border around the 
###
class DebugLoadingPriority extends Jit.JitModule
	constructor: (@source) ->
		super("debug-loading-priority")


	compile: (shader) ->

		compiledModule = new Jit.JitCompiledModule(this, shader)

		assetUniforms = {}
		assetUniforms['enabled'] = compiledModule.uniform("debugLoadingEnabled","f", 0.0)
		assetUniforms['amount'] = compiledModule.uniform("debugLoadingAmount","f", 1.0)

		compiledModule.fragmentBody = """
if (#{assetUniforms['enabled']} == 1.0){
	_fragment_state[0].rgb += #{assetUniforms['amount']} * smoothstep(0.05, 0.0, vUv.x);
	_fragment_state[0].rgb += #{assetUniforms['amount']} * smoothstep(0.95, 1.0, vUv.x);
	_fragment_state[0].rgb += #{assetUniforms['amount']} * smoothstep(0.05, 0.0, vUv.y);
	_fragment_state[0].rgb += #{assetUniforms['amount']} * smoothstep(0.95, 1.0, vUv.y);
}
"""
		return compiledModule

module.exports = DebugLoadingPriority