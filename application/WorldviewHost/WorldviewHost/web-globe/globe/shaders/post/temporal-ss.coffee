THREE = require 'three'
EffectComposer = require('three-effectcomposer')(THREE)


copyFS = require('./temporal-ss-copy-fs.glsl')
copyVS = require('./temporal-ss-copy-vs.glsl')

drawFS = require('./temporal-ss-draw-fs.glsl')
drawVS = require('./temporal-ss-draw-vs.glsl')


class TemporalSuperSamplingPrepass
	constructor: (@pass0, @pass1, @width, @height, overrideMaterial, clearColor, clearAlpha) ->  
		@enabled = true
		@frameCounter = 0
		@offsets = new Array(4)
		#https://msdn.microsoft.com/en-us/library/windows/desktop/cc627092%28v=vs.85%29.aspx
		#see "Multisample Anti-Aliasing Rasterization Rules" topic
		@offsets[0] = new THREE.Vector2( 0.125, 0.375)
		@offsets[1] = new THREE.Vector2(-0.375, 0.125)
		@offsets[2] = new THREE.Vector2(-0.125,-0.375)
		@offsets[3] = new THREE.Vector2( 0.375,-0.125)
		#invalid frame
		@nullOffset = new THREE.Vector2( 0.0, 0.0)

		@validFrames = new THREE.Vector4(0.0, 0.0, 0.0, 0.0)

		@temporalSSFadeTimeInSecs = 0.5
		@temporalSSFadeStart = new Date()

	render: ( renderer, writeBuffer, readBuffer, delta ) ->
		@frameCounter = (@frameCounter + 1) % 4

		frameOffset = @nullOffset

		if @isFrameValid()
			#if frame is valid, we want to start to add offset to do super sampling
			if this.validCount % 4 != 0 or this.validCount > @temporalSSFadeTimeInFrames
				#however, while we do fading, we only want to move all other frames until we finish blending.
				#this.validCount % 4 == 0 - is the frame that we start blending with.
				#therefore, we only change offset to this.validCount % 4 != 0
				frameOffset = @offsets[@frameCounter]
		else
			@resetFrames()

		if @validCount % 4 == 0
			#the prime frame - weight is always 1.
			@validFrames.setComponent(@frameCounter, 1.0)
		else
			#We increase the temporal weight from 0..1 in based on the time delta
			deltaInSeconds = (new Date() - @temporalSSFadeStart) / 1000.0
			weight = Math.min(1.0,deltaInSeconds / @temporalSSFadeTimeInSecs)
			this.validFrames.setComponent(this.frameCounter, weight );

		@validCount++

		#Modify 31 and 32 element in projection matrix for screen space shift
		@pass0.camera.projectionMatrix.elements[8] = frameOffset.x * 2.0 / @width
		@pass0.camera.projectionMatrix.elements[9] = frameOffset.y * 2.0 / @height

		@pass1.camera.projectionMatrix.elements[8] = @pass0.camera.projectionMatrix.elements[8]
		@pass1.camera.projectionMatrix.elements[9] = @pass0.camera.projectionMatrix.elements[9]

	setSize: (width, height) ->
		@width = width
		@height = height

	resetFrames: () ->
		@validCount = 0
		@validFrames.set(0.0, 0.0, 0.0, 0.0)
		@temporalSSFadeStart = new Date()

	isFrameValid: () ->
		#maybe we will add some other checks, currently frames are reseting via GlobeCanvas::render
		return true

class TemporalSuperSampling
	constructor: (@prepass, @width, @height) -> 

		@enabled = true
		@needsSwap = true

		@fb = new Array(4)
		@fb[0] = new THREE.WebGLRenderTarget( @width, @height, { minFilter: THREE.LinearFilter, magFilter: THREE.NearestFilter, format: THREE.RGBFormat } );
		@fb[1] = new THREE.WebGLRenderTarget( @width, @height, { minFilter: THREE.LinearFilter, magFilter: THREE.NearestFilter, format: THREE.RGBFormat } );
		@fb[2] = new THREE.WebGLRenderTarget( @width, @height, { minFilter: THREE.LinearFilter, magFilter: THREE.NearestFilter, format: THREE.RGBFormat } );
		@fb[3] = new THREE.WebGLRenderTarget( @width, @height, { minFilter: THREE.LinearFilter, magFilter: THREE.NearestFilter, format: THREE.RGBFormat } );

		@copyPass = new EffectComposer.ShaderPass(
			{
				uniforms: {
					"tDiffuse": { type: "t", value: null }

				},
				vertexShader: copyVS,
				fragmentShader: copyFS, 
			}
			)
		@drawPass = new EffectComposer.ShaderPass(
			{
				uniforms: {
					fb0: { type: "t", value: @fb[0] },
					fb1: { type: "t", value: @fb[1] },
					fb2: { type: "t", value: @fb[2] },
					fb3: { type: "t", value: @fb[3] },
					fbMask: { type: 'v4', value: new THREE.Vector4() }
				},
				vertexShader: drawVS,
				fragmentShader: drawFS,
			}
			)

	render: ( renderer, writeBuffer, readBuffer, delta ) ->

		autoClear = renderer.autoClear
		renderer.autoClear = false

		@drawPass.uniforms.fbMask.value.copy(@prepass.validFrames)

		#Remove screen space shift for compositing pass
		@prepass.pass0.camera.projectionMatrix.elements[8] = 0.0
		@prepass.pass0.camera.projectionMatrix.elements[9] = 0.0

		@prepass.pass1.camera.projectionMatrix.elements[8] = @prepass.pass0.camera.projectionMatrix.elements[8]
		@prepass.pass1.camera.projectionMatrix.elements[8] = @prepass.pass0.camera.projectionMatrix.elements[8]

		targetFB = @fb[@prepass.frameCounter]

		@copyPass.render( renderer, targetFB, readBuffer, delta );
		@drawPass.render( renderer, writeBuffer, targetFB, delta );

		renderer.autoClear = autoClear

	setSize: (@width, @height) ->
		for i in [0...4]
			@fb[i].setSize(@width, @height)

		@prepass.resetFrames()

#We need to execute PrePass process before actual scene rendering for proper projection matrix setup
#After main scene render we can execute DrawPass for combining a final image
#Example:
#@composer.addPass( @temporalSSPrepass )
#@composer.addPass( @sceneBlendEffect ) # draw scene here
#@composer.addPass( @temporalSSDrawPass )
#
module.exports = {
	PrePass: TemporalSuperSamplingPrepass,
	DrawPass: TemporalSuperSampling
}