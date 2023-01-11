THREE = require 'three'
EffectComposer = require('three-effectcomposer')(THREE)

fragmentShader = require('./scene-blend-fs.glsl')
vertexShader = require('./scene-blend-vs.glsl')

class SceneBlendPass

	constructor: (@scene0, @scene1, @width, @height, overrideMaterial, clearColor, clearAlpha) ->  #transform is TEMPORARY 
		@pass0 = new EffectComposer.RenderPass(@scene0.threeScene, @scene0.cameraController.camera, overrideMaterial, clearColor, clearAlpha)
		@pass1 = new EffectComposer.RenderPass(@scene1.threeScene, @scene1.cameraController.camera, overrideMaterial, clearColor, clearAlpha)

		@enabled = true;
		
		@blendValue = 0.0

		#Create quad specific stuff
		@rtTexture = new THREE.WebGLRenderTarget( @width, @height, { minFilter: THREE.LinearFilter, magFilter: THREE.NearestFilter, format: THREE.RGBFormat } );
		
		quadMaterial =
			uniforms:
				tDiffuse: { type: "t", value: @rtTexture },
				resolution: { type: 'v2', value: new THREE.Vector2( 0, 0 ) }
				blendValue: { type: 'f', value: .04 }
			vertexShader: vertexShader
			fragmentShader: fragmentShader
			side: THREE.BackSide
			transparent : true
			blending : THREE.NormalBlending
		@quadMaterial = new THREE.ShaderMaterial(quadMaterial)

		@quadgeometry = new THREE.Geometry()

		@quadgeometry.vertices.push(
			new THREE.Vector3( -1, -1, 0 ),
			new THREE.Vector3(  1, -1, 0 ),
			new THREE.Vector3(  1,  1, 0 ),
			new THREE.Vector3( -1,  1, 0 )
		)

		@quadgeometry.faces.push( new THREE.Face3( 1, 0, 2 ) )
		@quadgeometry.faces.push( new THREE.Face3( 2, 0, 3 ) )

		@quadMesh = new THREE.Mesh( @quadgeometry, @quadMaterial )
		@quadMesh.frustumCulled = false
		@quadScene = new THREE.Scene()
		@quadScene.add @quadMesh

	render: ( renderer, writeBuffer, readBuffer, delta ) ->
		if @blendValue == 0
			if @scene0.globe.tree.clipAnimation
				@scene0.globe.tree.clipData.x = 0.0
			@pass0.render(renderer, writeBuffer, readBuffer, delta)
			return
		else if @blendValue == 1
			if @scene1.globe.tree.clipAnimation
				@scene1.globe.tree.clipData.x = 0.0
			@pass1.render(renderer, writeBuffer, readBuffer, delta)
			return

		if @scene0.globe.tree.clipAnimation
			autoClear = renderer.autoClear
			@pass1.clear = false
			renderer.autoClear = false

			updateBlendValue = (clipData, blendValue) ->
				powValue = clipData.z

				###
				#alternative curve
				powValue = 1.0 / powValue
				if blendValue < 0.5
					blendValue = 0.5 - Math.pow((0.5 - blendValue) * 2, powValue) * 0.5
				else
					blendValue = 0.5 + Math.pow((blendValue - 0.5) * 2, powValue) * 0.5
				###
				if blendValue < 0.5
					blendValue = Math.pow((blendValue) * 2, powValue) * 0.5
				else
					blendValue = 1.0 - Math.pow((1.0 - blendValue) * 2, powValue) * 0.5

				clipData.y = blendValue * -2 + 1

			@scene0.globe.tree.clipData.x = 1.0
			@scene1.globe.tree.clipData.x = -1.0
			updateBlendValue(@scene0.globe.tree.clipData, @blendValue)
			updateBlendValue(@scene1.globe.tree.clipData, @blendValue)

			@scene1.backdropSphere.visible = false
			@pass0.render(renderer, writeBuffer, readBuffer, delta)
			@pass1.render(renderer, writeBuffer, readBuffer, delta)
			@scene1.backdropSphere.visible = true

			renderer.autoClear = autoClear
			@pass1.clear = true
		else
			@quadMaterial.uniforms.blendValue.value = @blendValue
			@pass1.render(renderer, writeBuffer, @rtTexture, delta)
			@pass0.render(renderer, writeBuffer, readBuffer, delta)
			
			renderer.render( @quadScene, @pass0.camera, readBuffer, false );
			renderer.autoClear = autoClear

	swapPasses: () ->
		oldPass0 = @pass0
		@pass0 = @pass1
		@pass1 = oldPass0
		oldScene = @scene0
		@scene0 = @scene1
		@scene1 = oldScene
		@blendValue = 0.0

	setSize: (@width, @height) ->
		@rtTexture.setSize(@width, @height)


module.exports = {
	SceneBlendPass: SceneBlendPass
}