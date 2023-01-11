###
	Define the Theme model
###

_ = require 'underscore'
THREE = require 'three'
Color = require 'color'

TextureLoader = require('../../utilities/loader').TextureLoader
SharedUniforms = require('../shaders/jit/shared-uniforms')


###
	Asset Loading.
	Unfortunately we can't move all asset loading functionality to JIT since
	we will need these for background and post shaders as well.

	assets get loaded via theme.loadAsset - we store a reference to the texture and
	a simple timestamp for the last time the asset was accessed (that way we can run
	a process periodically to clean up assets)
###
dummyTexture = new THREE.WebGLRenderTarget( 1, 1, {
	minFilter: THREE.LinearFilter
	magFilter: THREE.NearestFilter
	format: THREE.RGBFormat
})

tempThreeColor0 = new THREE.Color();
tempThreeColor1 = new THREE.Color();

assets = {}
lastAssetValues = {}  # keep a state of asset urls to avoid loading when not necessary

# allow assetRoot path to be set externally
window.assetRoot or= '/'
window.assets = assets # debug
assetLoader = new TextureLoader()

globeBackground = new THREE.Vector3(0.0,0.0,0.0)

# name map needed for datGUI labels
nameMap =
	dataMin: "Data Min"
	dataMax: "Data Max"
	backgroundColor: "Background Color"
	backgroundColor2: "Background Color 2"
	environmentMultiplier: "Matcap Strength"
	heightMultiplier: "Height Multiplier"
	normalmapStrength: "Normalmap Strength"
	phongStrength: "Phong Strength"

	specular: 'Specular'
	shininess: 'Shininess'
	diffuseStrength: 'DiffuseStrength'
	lightingEnable: 'LightingEnable'



	# we can use the base alpha map to mask out the water and still bring on
	# full coverage Bing maps.  This acts as a boolean as 1 or 0
	landOnly: "Land Only"

	#post process options
	gammaValue: "Gamma Exponent"
	colorBoost: 'Color Boost'
	colorTint: 'Color Tint'
	tiltshiftValue: "Tilt Shift"
	tiltshiftRes: "Tilt Shift Location"

	# camera limits
	rangeMax: "Range Max"
	rangeMin: "Range Min"
	latMax: "Latitute Max" # no limit
	latMin: "Latitute Min"
	lonMax: "Longitude Max"
	lonMin: "Longitude Min"

	#fog
	fogColor: "Fog Color"
	fogIntensity: "Fog Intensity"
	fogHeightOffset: -200.0 #for start hegiht
	fogBlendCoef: 3.2 # fog blend dist conf( less value more blend )
	fogCameraDistCoef: 5.6 # camera distance blend coef( more value more fog on current dist )

	# lighting
	lightIntensity: "Lighting Intensity"

	# assets
	roughMatcap: 'Rough Matcap URL' 
	roughMatcapFrameCount: 'Rough Matcap Frame Count'
	roughMatcapFrame: 'Rough Matcap Frame Index'
	glossMatcap: 'Glossy Matcap URL'
	glossMatcapFrameCount: 'Glossy Matcap Frame Count'
	glossMatcapFrame: 'Glossy Matcap Frame Index'
	noiseMap: 'Noise Map URL'

	# base layer object
	globeBackground: 'Globe Background Color'
	noiseMultiplier: 'Noise Strength'
	basemapRoughness: 'Basemap Roughness'
	basemapLightingMultiplier: 'Basemap Matcap Strength'
	surfaceRoughness: 'Surface Roughness'
	surfaceLightingMultiplier: 'Surface Matcap Strength'

	# rhombus layers 
	# note instead of using an array or an object we use numbered variables for
	# ease of access
	layer0Alpha: 'Layer 0 Alpha'
	layer0Tint: 'Layer 0 Tint'

	layer1Alpha: 'Layer 1 Alpha'
	layer1Tint: 'Layer 1 Tint'

	layer2Alpha: 'Layer 2 Alpha'
	layer2Tint: 'Layer 2 Tint'

	layer3Alpha: 'Layer 3 Alpha'
	layer3Tint: 'Layer 3 Tint'

	layer4Alpha: 'Layer 4 Alpha'
	layer4Tint: 'Layer 4 Tint'

	layer5Alpha: 'Layer 5 Alpha'
	layer5Tint: 'Layer 5 Tint'

	layer6Alpha: 'Layer 6 Alpha'
	layer6Tint: 'Layer 6 Tint'

	layer7Alpha: 'Layer 7 Alpha'
	layer7Tint: 'Layer 7 Tint'

	layer8Alpha: 'Layer 8 Alpha'
	layer8Tint: 'Layer 8 Tint'

	layer9Alpha: 'Layer 9 Alpha'
	layer9Tint: 'Layer 9 Tint'


	# icon layers
	iconAlpha: 'Icon Alpha'
	iconBorderColor: 'Icon Border Color'


blendableParameters = [
	"specular", "shininess", "diffuseStrength", "lightingEnable", 

	"backgroundColor", "backgroundColor2", "environmentMultiplier",	"clipAnimationPower", "phongStrength"

	"gammaValue", "colorBoost", "colorTint", "tiltshiftValue", "tiltshiftRes",
	
	"rangeMax", "rangeMin", "latMax", "latMin", "lonMax", "lonMin", 

	"lightIntensity", 

	"globeBackground", "noiseMultiplier", "basemapRoughness", "basemapLightingMultiplier", "surfaceRoughness", 
	"surfaceLightingMultiplier", "roughMatcapFrame", "glossMatcapFrame",

	"layer0Alpha", "layer0Tint", "layer1Alpha", "layer1Tint", "layer2Alpha", "layer2Tint", "layer3Alpha", "layer3Tint",
	"layer4Alpha", "layer4Tint", "layer5Alpha", "layer5Tint", "layer6Alpha", "layer6Tint", "layer7Alpha", "layer7Tint",
	"layer8Alpha", "layer8Tint", "layer9Alpha", "layer9Tint", 

	"iconAlpha", "iconBorderColor"
]

uniformUpdateHandlers = 
	backgroundColor: (theme, scene) ->
		bc = Color(theme.namedObject['backgroundColor'])
		scene.backdropMaterial.uniforms.backgroundColor.value.set(bc.red() / 255.0, bc.green() / 255.0, bc.blue() / 255.0)

	backgroundColor2: (theme, scene) ->
		bc = Color(theme.namedObject['backgroundColor2'])
		scene.backdropMaterial.uniforms.backgroundColor2.value.set(bc.red() / 255.0, bc.green() / 255.0, bc.blue() / 255.0)

	globeBackground: (theme, scene) ->
		gb = Color(theme.namedObject['globeBackground'])
		globeBackground.set(gb.red() / 255.0, gb.green() / 255.0, gb.blue() / 255.0)

	clipAnimationPower: (theme, scene) ->
		if theme.clipAnimation
			scene.globe.tree.clipData.z = theme.clipAnimationPower

	lightIntensity: (theme, scene) ->
		scene.light0.intensity = theme.namedObject['lightIntensity']
		scene.light1.intensity = theme.namedObject['lightIntensity']

	roughMatcapFrameCount: (theme, scene) ->
		theme.initRoughMatCapFrameData(scene.globe.tree.shaderFactory.uniforms['roughMatcapFrameData_1'].value, theme.namedObject)

	roughMatcapFrame: (theme, scene) ->
		uniformUpdateHandlers.roughMatcapFrameCount(theme, scene)

	glossMatcapFrameCount: (theme, scene) ->
		theme.initGlossMatCapFrameData(scene.globe.tree.shaderFactory.uniforms['glossMatcapFrameData_1'].value, theme.namedObject)

	glossMatcapFrame: (theme, scene) ->
		uniformUpdateHandlers.glossMatcapFrameCount(theme, scene)

	heightMultiplier: (theme, scene) -> scene.globe.tree.updateElevationMultiplier(theme.namedObject['heightMultiplier'])

	environmentMultiplier: (theme, scene) ->
		scene.globe.tree.shaderFactory.uniforms['environment_and_noise_multiplier_1'].value.set(
			theme.namedObject['environmentMultiplier'], theme.namedObject['noiseMultiplier'])

	phongStrength: (theme, scene) -> scene.globe.tree.updatePhongStrength(theme.namedObject['phongStrength'])

	noiseMultiplier: (theme, scene) ->
		uniformUpdateHandlers.environmentMultiplier(theme, scene)

	basemapRoughness: (theme, scene) ->	
		scene.globe.tree.shaderFactory.uniforms['basemapRoughness_and_LightingMultiplier_1'].value.set( 
				theme.namedObject['basemapRoughness'], theme.namedObject['basemapLightingMultiplier'] )
	basemapLightingMultiplier: (theme, scene) ->
		uniformUpdateHandlers.basemapRoughness(theme, scene)

	surfaceRoughness: (theme, scene) ->
		scene.globe.tree.shaderFactory.uniforms['surfaceRoughness_and_LightingMultiplier_1'].value.set( 
			theme.namedObject['surfaceRoughness'], theme.namedObject['surfaceLightingMultiplier'] )
	surfaceLightingMultiplier: (theme, scene) ->
		uniformUpdateHandlers.surfaceRoughness(theme, scene)

	specular: (theme, scene) ->
		scene.globe.tree.shaderFactory.uniforms['lightingParams'].value.set(theme.namedObject['specular'], theme.namedObject['shininess'], theme.namedObject['diffuseStrength'], theme.namedObject['lightingEnable'])
	shininess: (theme, scene) ->
		uniformUpdateHandlers.specular(theme, scene)
	diffuseStrength: (theme, scene) ->
		uniformUpdateHandlers.specular(theme, scene)
	lightingEnable: (theme, scene) ->
		uniformUpdateHandlers.specular(theme, scene)


	#TODO Maybe preprocessor here?
	layer0Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer0Tint_1'].value, theme.namedObject['layer0Tint'], theme.namedObject['layer0Alpha'] )
	layer0Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer0Tint_1'].value, theme.namedObject['layer0Tint'], theme.namedObject['layer0Alpha'] )
	layer1Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer1Tint_1'].value, theme.namedObject['layer1Tint'], theme.namedObject['layer1Alpha'] )
	layer1Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer1Tint_1'].value, theme.namedObject['layer1Tint'], theme.namedObject['layer1Alpha'] )
	layer2Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer2Tint_1'].value, theme.namedObject['layer2Tint'], theme.namedObject['layer2Alpha'] )
	layer2Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer2Tint_1'].value, theme.namedObject['layer2Tint'], theme.namedObject['layer2Alpha'] )
	layer3Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer3Tint_1'].value, theme.namedObject['layer3Tint'], theme.namedObject['layer3Alpha'] )
	layer3Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer3Tint_1'].value, theme.namedObject['layer3Tint'], theme.namedObject['layer3Alpha'] )
	layer4Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer4Tint_1'].value, theme.namedObject['layer4Tint'], theme.namedObject['layer4Alpha'] )
	layer4Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer4Tint_1'].value, theme.namedObject['layer4Tint'], theme.namedObject['layer4Alpha'] )
	layer5Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer5Tint_1'].value, theme.namedObject['layer5Tint'], theme.namedObject['layer5Alpha'] )
	layer5Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer5Tint_1'].value, theme.namedObject['layer5Tint'], theme.namedObject['layer5Alpha'] )
	layer6Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer6Tint_1'].value, theme.namedObject['layer6Tint'], theme.namedObject['layer6Alpha'] )
	layer6Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer6Tint_1'].value, theme.namedObject['layer6Tint'], theme.namedObject['layer6Alpha'] )
	layer7Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer7Tint_1'].value, theme.namedObject['layer7Tint'], theme.namedObject['layer7Alpha'] )
	layer7Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer7Tint_1'].value, theme.namedObject['layer7Tint'], theme.namedObject['layer7Alpha'] )
	layer8Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer8Tint_1'].value, theme.namedObject['layer8Tint'], theme.namedObject['layer8Alpha'] )
	layer8Alpha: (theme, scene) ->SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer8Tint_1'].value, theme.namedObject['layer8Tint'], theme.namedObject['layer8Alpha'] )
	layer9Tint: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer9Tint_1'].value, theme.namedObject['layer9Tint'], theme.namedObject['layer9Alpha'] )
	layer9Alpha: (theme, scene) -> SharedUniforms.assignColorAndAlphaToVector4( scene.globe.tree.shaderFactory.uniforms['layer9Tint_1'].value, theme.namedObject['layer9Tint'], theme.namedObject['layer9Alpha'] )


	gammaValue: (theme, scene) ->
		if scene.postProcessPass
			gv = theme.namedObject['gammaValue']
			scene.postProcessPass.uniforms.gammaValue.value.set(gv, gv, gv)

	colorTint: (theme, scene) ->
		if scene.postProcessPass
			scene.postProcessPass.uniforms.colorTint.value.set( theme.namedObject['colorTint'] )

	colorBoost: (theme, scene) ->
		if scene.postProcessPass
			scene.postProcessPass.uniforms.colorBoost.value.set( theme.namedObject['colorBoost'] )

	tiltshiftValue: (theme, scene) ->
		if scene.postProcessPass
			scene.postProcessPass.uniforms.tiltshiftValue.value = theme.namedObject['tiltshiftValue']

	tiltshiftRes: (theme, scene) ->
		if scene.postProcessPass
			scene.postProcessPass.uniforms.tiltshiftRes.value = theme.namedObject['tiltshiftRes']

	iconBorderColor: (theme, scene) ->
		# set icon tree values
		iconBorderColor = theme.namedObject['iconBorderColor']
		iconAlpha = theme.namedObject['iconAlpha']

		if scene.globe.iconTrees
			_.each scene.globe.iconTrees, (iconTree) ->
				SharedUniforms.assignColorAndAlphaToVector4( iconTree.sharedUniforms.borderColorAndIconAlpha, iconBorderColor, iconAlpha )

	iconAlpha: (theme, scene) ->
		uniformUpdateHandlers.iconBorderColor(theme, scene)

###
	no constructor on Theme, instead the class is applied
	as the prototype of a config object via themeFactory
###
class Theme
	###
		Set theme from another object, this will automatically update the values in the dat-gui control
		as well as change the uniforms
	###
	set: (object) ->
		_.extend this, object

		@updateUniforms(window.GC.scene)
		@updateUniforms(window.GC.swapScene) if window.GC.syncGlobes


	init: ->
		return if @initialized

		@initialized = true
		# named object is a backwards-facing reference for the nameMap
		@namedObject = {}

		_.each this, (val, key) =>
			if key != "altitudeBasedParameters"
				@namedObject[key] = val

		if @altitudeBasedParameters
			@blendParameters = []
			@lastAltitude = -1
			#add first block with default parameters
			block = {}
			block.startAltitude = 9999999999.0
			block.endAltitude = block.startAltitude
			block.parameters = {}
			_.extend block.parameters, @namedObject
			@blendParameters.push block

			_.each @altitudeBasedParameters, (val, key) =>
				block = {}
				_.extend block, val
				block.parameters = {}
				_.extend block.parameters, @blendParameters[@blendParameters.length - 1].parameters
				_.extend block.parameters, val.parameters 
				block.endAltitude = block.startAltitude - block.blendAmount

				@blendParameters.push block


	###
		Return a set of uniforms given a config
		TODO: reduce boilerplate by reading the values of the config
	###
	generateRhombusUniforms: ->
		gb = Color(@globeBackground)

		globeBackground.x = gb.red() / 255.0
		globeBackground.y = gb.green() / 255.0
		globeBackground.z = gb.blue() / 255.0

		uniforms = {

			# cutoff: { type: "f", value: 3.9 },

			# elevation on vertex shader will only calculate if useElevation is 1.0
			# useElevation: { type: "f", value: 0.0 },
			# elevationMultichannel: { type: "f", value: 0.0 },
			# elevationRange: { type: "v2", value: new THREE.Vector2(0,1) },
			# elevationMap: { type: "t", value: undefined },
			# elevationMapUvTrans: { type: "v4", value: new THREE.Vector4(0,0,0,0) },

			heightMultiplier: { type: "f", value: 1.0 },

			#envMap: { type: 't', value: undefined },
			reflectivity : { type: "f", value: 99.99 },
			#refractionRatio: { type:'f',value: 0.5 },


			globeBackground: { type: "v3", value: globeBackground, shared: true },



			# these have been moved into the environment
			# tRoughMatCap: { type: 't', value: window.GC.assets.roughMatcap },
			# tGlossMatCap: { type: 't', value: window.GC.assets.glossMatcap },
			# tNoiseMap: { type: 't', value: window.GC.assets.noiseTexture },
			tTest: { type: 't', value: dummyTexture }
		}

		return uniforms



	generatePostProcessUniforms: ->
		console.log "generate post process uniforms ", this

		uniforms = {
			tDiffuse: { type: "t", value: null },
			tiltshiftValue: { type: "f", value: @tiltshiftValue }
			tiltshiftRes: { type: "f", value: @tiltshiftRes }
			gammaValue:   { type: "v3", value: new THREE.Vector3( @gammaValue, @gammaValue, @gammaValue ) }
			colorTint:   { type: "c", value: new THREE.Color(@colorTint) }
			colorBoost:   { type: "c", value: new THREE.Color(@colorBoost) }
		}


	saveThemeToMap: ->
		# on this operation use the map object to set the current theme
		if window.getCurrentMapModel()
			window.getCurrentMapModel().Metadata.Theme = this

	updateUniforms: (scene) ->
		self = this

		@init() unless @initialized

		# apply uniforms to allow dynamic colors

		# we need a handle to the real uniform names
		# this is just a map of uniform names
		# env = mesh.material.jit.require('environment').assetUniforms

		# HMM... there's gotta be a way to get the real uniform name

		_.each uniformUpdateHandlers, (val, key) -> 
			val(self, scene)


	initRoughMatCapFrameData: (vector4, theme) ->
		#x - frameSize
		#y - frameIndex0
		#z - frameIndex1
		#w - frameBlendValue
		if theme is undefined
			theme = this

		vector4.x = 1.0 / theme.roughMatcapFrameCount
		vector4.y = theme.roughMatcapFrame
		vector4.z = theme.roughMatcapFrame
		vector4.w = 0.0

	updateRoughMatCapFrameData: (vector4, frame0Index, frame1Index, blendValue) ->
		#x - frameSize
		#y - frameIndex0
		#z - frameIndex1
		#w - frameBlendValue

		vector4.x = 1.0 / @roughMatcapFrameCount
		vector4.y = frame0Index
		vector4.z = frame1Index
		vector4.w = blendValue


	initGlossMatCapFrameData: (vector4, theme) ->
		#x - frameSize
		#y - frameIndex0
		#z - frameIndex1
		#w - frameBlendValue
		if theme is undefined
			theme = this

		vector4.x = 1.0 / theme.glossMatcapFrameCount
		vector4.y = theme.glossMatcapFrame
		vector4.z = theme.glossMatcapFrame
		vector4.w = 0.0

	updateGlossMatCapFrameData: (vector4, frame0Index, frame1Index, blendValue) ->
		#x - frameSize
		#y - frameIndex0
		#z - frameIndex1
		#w - frameBlendValue

		vector4.x = 1.0 / @glossMatcapFrameCount
		vector4.y = frame0Index
		vector4.z = frame1Index
		vector4.w = blendValue


	setAltitudeParameters: (scene, altitude) ->
		return unless @blendParameters

		if @lastAltitude == altitude
			return

		@lastAltitude = altitude

		srcBlock = @blendParameters[0]
		destBlock = undefined
		blendValue = 0.0
		for i in [0...@blendParameters.length]
			destBlock = @blendParameters[i]
			if altitude < destBlock.startAltitude and altitude > destBlock.endAltitude
				blendValue = 1.0 - (altitude - destBlock.endAltitude) / (destBlock.startAltitude - destBlock.endAltitude)
				break
			if altitude > destBlock.startAltitude
				break
			srcBlock = destBlock

		#blend parameters
		if srcBlock and destBlock
			for parameterName in blendableParameters
				parameterValue = @namedObject[parameterName]
				if srcBlock.parameters[parameterName] == destBlock.parameters[parameterName]
					continue
				if parameterName == "roughMatcapFrame"
					@updateRoughMatCapFrameData(scene.globe.tree.shaderFactory.uniforms['roughMatcapFrameData_1'].value,
						srcBlock.parameters[parameterName], destBlock.parameters[parameterName], blendValue)
					continue
				if parameterName == "glossMatcapFrame"
					@updateGlossMatCapFrameData(scene.globe.tree.shaderFactory.uniforms['glossMatcapFrameData_1'].value,
						srcBlock.parameters[parameterName], destBlock.parameters[parameterName], blendValue)
					continue
				if typeof parameterValue == 'number'
					@namedObject[parameterName] = srcBlock.parameters[parameterName] + (destBlock.parameters[parameterName] - srcBlock.parameters[parameterName]) * blendValue
					uniformUpdateHandlers[parameterName](this, scene)
				else if typeof parameterValue == 'string' #color
					tempThreeColor0.set(srcBlock.parameters[parameterName])
					tempThreeColor1.set(destBlock.parameters[parameterName])
					@namedObject[parameterName] = "#" + tempThreeColor0.lerp(tempThreeColor1, blendValue).getHexString()
					uniformUpdateHandlers[parameterName](this, scene)

	###
		Same as updateUniforms but specifically related to global assets
	###
	updateAssets: (scene, meshes) ->
		meshes or= _.toArray(scene.globe.tree.meshes)

		# reset values
		@roughMatcap = @namedObject['roughMatcap']
		@glossMatcap = @namedObject['glossMatcap']
		@noiseMap = @namedObject['noiseMap']

		@loadAssetByName 'roughMatcap', (texture) ->
			for mesh in meshes
				mesh.material.uniforms['tRoughMatCap_1'].value = texture

		@loadAssetByName 'glossMatcap', (texture) ->
			for mesh in meshes
				mesh.material.uniforms['tGlossMatCap_1'].value = texture

		@loadAssetByName 'noiseMap', (texture) ->
			for mesh in meshes
				mesh.material.uniforms['tNoiseMap_1'].value = texture



	###
		Generate DatGUI from the config
	###
	generateDatGui: (gui) ->
		self = this

		console.log "MY DATA CREATED x", @message
		gui or= new dat.GUI({closed: true })
		window.datgui = gui # debug handle

		# gui.remember(this)

		@init() unless @initialized

		themeUpdateHandler = ->
			window.GC.theme.updateUniforms(window.GC.scene)
			window.GC.theme.updateUniforms(window.GC.swapScene) if window.GC.syncGlobes
			window.GC.theme.saveThemeToMap()

		assetUpdateHandler = ->
			window.GC.theme.updateAssets(window.GC.scene)
			window.GC.theme.updateAssets(window.GC.swapScene) if window.GC.syncGlobes
			window.GC.theme.saveThemeToMap()

		gui.add(this, 'name').listen()


		scene = gui.addFolder 'Scene'
		scene.addColor(@namedObject, 'backgroundColor').name(nameMap['backgroundColor']).listen().onChange themeUpdateHandler
		scene.addColor(@namedObject, 'backgroundColor2').name(nameMap['backgroundColor2']).listen().onChange themeUpdateHandler
		scene.add(@namedObject, 'lightIntensity', 0.0, 1.0).name(nameMap['lightIntensity']).listen().onChange themeUpdateHandler
		

		base = gui.addFolder 'Base Map'
		base.addColor(@namedObject, 'globeBackground').name(nameMap['globeBackground']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'environmentMultiplier', 0, 4.0).name(nameMap['environmentMultiplier']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'phongStrength', 0, 1.0).name(nameMap['phongStrength']).listen().onChange themeUpdateHandler
		
		base.add(@namedObject, 'roughMatcap').name(nameMap['roughMatcap']).listen().onChange assetUpdateHandler
		base.add(@namedObject, 'glossMatcap').name(nameMap['glossMatcap']).listen().onChange assetUpdateHandler
		base.add(@namedObject, 'roughMatcapFrameCount', 0, 16.0).step(1.0).name(nameMap['roughMatcapFrameCount']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'roughMatcapFrame', 0, 16.0).step(1.0).name(nameMap['roughMatcapFrame']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'glossMatcapFrameCount', 0, 16.0).step(1.0).name(nameMap['glossMatcapFrameCount']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'glossMatcapFrame', 0, 16.0).step(1.0).name(nameMap['glossMatcapFrame']).listen().onChange themeUpdateHandler

		
		base.add(@namedObject, 'basemapRoughness', 0, 1.0).name(nameMap['basemapRoughness']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'basemapLightingMultiplier', 0, 2.0).name(nameMap['basemapLightingMultiplier']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'surfaceRoughness', 0, 1.0).name(nameMap['surfaceRoughness']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'surfaceLightingMultiplier', 0, 2.0).name(nameMap['surfaceLightingMultiplier']).listen().onChange themeUpdateHandler


		noise = gui.addFolder 'Noise Map'
		noise.add(@namedObject, 'noiseMap').name(nameMap['noiseMap']).listen().onChange assetUpdateHandler
		noise.add(@namedObject, 'noiseMultiplier', 0, 1.0).name(nameMap['noiseMultiplier']).listen().onChange themeUpdateHandler

		icon = gui.addFolder 'Icon Defaults'
		icon.addColor(@namedObject, 'iconBorderColor').name(nameMap['iconBorderColor']).listen().onChange themeUpdateHandler
		icon.add(@namedObject, 'iconAlpha', 0, 1.0).name(nameMap['iconAlpha']).listen().onChange themeUpdateHandler


		layers = gui.addFolder 'Layers'
		

		base = layers.addFolder 'Layer 0'
		base.addColor(@namedObject, 'layer0Tint').name(nameMap['layer0Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer0Alpha', 0, 1.0).name(nameMap['layer0Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 1'
		base.addColor(@namedObject, 'layer1Tint').name(nameMap['layer1Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer1Alpha', 0, 1.0).name(nameMap['layer1Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 2'
		base.addColor(@namedObject, 'layer2Tint').name(nameMap['layer2Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer2Alpha', 0, 1.0).name(nameMap['layer2Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 3'
		base.addColor(@namedObject, 'layer3Tint').name(nameMap['layer3Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer3Alpha', 0, 1.0).name(nameMap['layer3Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 4'
		base.addColor(@namedObject, 'layer4Tint').name(nameMap['layer4Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer4Alpha', 0, 1.0).name(nameMap['layer4Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 5'
		base.addColor(@namedObject, 'layer5Tint').name(nameMap['layer5Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer5Alpha', 0, 1.0).name(nameMap['layer5Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 6'
		base.addColor(@namedObject, 'layer6Tint').name(nameMap['layer6Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer6Alpha', 0, 1.0).name(nameMap['layer6Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 7'
		base.addColor(@namedObject, 'layer7Tint').name(nameMap['layer7Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer7Alpha', 0, 1.0).name(nameMap['layer7Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 8'
		base.addColor(@namedObject, 'layer8Tint').name(nameMap['layer8Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer8Alpha', 0, 1.0).name(nameMap['layer8Alpha']).listen().onChange themeUpdateHandler

		base = layers.addFolder 'Layer 9'
		base.addColor(@namedObject, 'layer9Tint').name(nameMap['layer9Tint']).listen().onChange themeUpdateHandler
		base.add(@namedObject, 'layer9Alpha', 0, 1.0).name(nameMap['layer9Alpha']).listen().onChange themeUpdateHandler

		post = gui.addFolder 'Post Processing'
		post.add(@namedObject, 'gammaValue', 0.0, 4.0).name(nameMap['gammaValue']).listen().onChange themeUpdateHandler
		post.addColor(@namedObject, 'colorTint').name(nameMap['colorTint']).listen().onChange themeUpdateHandler
		post.addColor(@namedObject, 'colorBoost').name(nameMap['colorBoost']).listen().onChange themeUpdateHandler
		post.add(@namedObject, 'tiltshiftValue', 0.0, .01).name(nameMap['tiltshiftValue']).listen().onChange themeUpdateHandler
		post.add(@namedObject, 'tiltshiftRes', 0.0, 1.0).name(nameMap['tiltshiftRes']).listen().onChange themeUpdateHandler




		# f1 = gui.addFolder 'Render Options'
		# f1.add dataObj, 'speed', -5, 5
		# f1.add dataObj, 'displayOutline'

		# f1.addColor(dataObj, 'backgroundColor').onChange colorHandler
		# f1.addColor(dataObj, 'globeBackground').onChange colorHandler
		# f1.add(dataObj, 'environmentMultiplier').onChange colorHandler
		# f1.add(dataObj, 'heightMultiplier').onChange colorHandler

		# f1.addColor(dataObj, 'fogColor').onChange colorHandler
		# f1.add(dataObj, 'fogIntensity', 0.0, 1.0).onChange colorHandler
		# f1.add(dataObj, 'fogHeightOffset', -5000.0, 5000.0).onChange colorHandler
		# f1.add(dataObj, 'fogBlendCoef', 1.0, 5.0).onChange colorHandler
		# f1.add(dataObj, 'fogCameraDistCoef', 4.0, 7.0).onChange colorHandler

		# f1.add(dataObj, 'specular', 0.0, 2.0).onChange colorHandler
		# f1.add(dataObj, 'shininess', 1.0, 64.0).onChange colorHandler
		# f1.add(dataObj, 'diffuseStrength', 0.0, 5.0).onChange colorHandler
		# f1.add(dataObj, 'lightingEnable', 0.0, 1.0).onChange colorHandler


	###
		since this gets called many times before a callback response we
		store an array of callbacks to be executed on load
	###
	loadAsset: (url, callback) ->
		# console.log "LOAD ", url, callback
		return unless url and url.indexOf
		unless url.indexOf('http') is 0
			url = window.assetRoot + url

		if assets[url]
			if _.isArray(assets[url])
				return assets[url].push callback
			else
				assets[url].lastAccessed = new Date().getTime()
				return callback(assets[url].texture)
		else
			# start array of callbacks
			assets[url] = [callback]

			assetLoader.load({
				url: url,
				success: (texture) ->
					texture.generateMipmaps = false
					texture.minFilter = THREE.LinearFilter
					texture.needsUpdate = true
					callbacks = assets[url]
					assets[url] = {texture: texture, lastAccessed: new Date().getTime()}

					_.each callbacks, (callback) -> callback(texture)
				})


	loadAssetByName: (name, callback) ->
		if this[name]
			@loadAsset(this[name], callback)





themeFactory = (config) ->
	config.__proto__ = Theme.prototype
	return config


module.exports =
	Theme: Theme
	themeFactory: themeFactory

