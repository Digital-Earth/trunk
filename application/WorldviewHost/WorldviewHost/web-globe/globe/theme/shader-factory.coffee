###
	Define the shaderFactory to construct the JitShader
###

_ = require 'underscore'
THREE = require 'three'
Color = require 'color'

SharedUniforms = require('../shaders/jit/shared-uniforms')

Jit = require('../jit-shader')
Jit.GeoSource =  require('../shaders/jit/geo-source').GeoSource
Jit.Phong = require('../shaders/rhombus/phong')
Jit.Elevation = require('../shaders/rhombus/elevation')
Jit.Color = require('../shaders/rhombus/color')
Jit.ColorBlend = require('../shaders/rhombus/color-blend')
Jit.Environment = require('../shaders/rhombus/environment')
Jit.Phong = require('../shaders/rhombus/phong')
Jit.Selection = require('../shaders/rhombus/selection')
Jit.Wgs84BoundingBox = require('../shaders/jit/wgs84-bounding-box')
Jit.Line = require('../shaders/rhombus/line')
Jit.ClipAnimation = require('../shaders/jit/clip-animation')
Jit.MipBlend = require('../shaders/rhombus/mip-blend')
Jit.DebugLoadingPriority = require('../shaders/jit/debug-loading')

cloneMeshAttributes = (attributes) ->
	newAttributes = {}
	for key, value of attributes
		if value.array
			arr = value.array.slice()
			attribute = new THREE.BufferAttribute( arr, value.itemSize )
		else
			attribute = new THREE.BufferAttribute( undefined, value.itemSize )
		newAttributes[key] = attribute
	return newAttributes

###
	note: currently this is the only shaderFactory implemented 
###
buildThemedShader = (rhombusTree, dataProviders, renderLayers, themeObject) ->

	# a way to provide a globe without a map select
	if renderLayers is undefined or renderLayers.length is 0
		renderLayers = themeObject.defaultRenderLayers

	jitShader = new Jit.JitShader(themeObject)

	elevationModules = []
	colorModules = []
	geoSourceDataProvider = dataProviders["geoSource"]
	geoSources = []

	_.each renderLayers, (layer, index) ->
		# options to pass into each jitModule
		options = {
			layerIndex: index
			landOnly: themeObject.landOnly
		}

		if layer.characterization
			boundingSphere = layer.characterization.boundingSphere
			nativeResolution = layer.characterization.NativeResultion

		if layer.geoSource && layer.geoSource.Specification && layer.geoSource.Specification.OutputType == "Coverage"
			if layer.style.Fill
				field = layer.style.Fill.PaletteExpression
				palette = layer.style.Fill.Palette
				range = [palette.Steps[0].Value,palette.Steps[palette.Steps.length-1].Value]

				geosource = geoSourceDataProvider.getGeoSource(layer.geoSource.Id, {range:range})
				geoSources.push geosource

				source = new Jit.GeoSource(geosource,{
					field:field
					range:range
					hexSampling:true,
					borrowParentTexture:true,
					boundingSphere:boundingSphere
					})
					
				source2 = source.asLowResolution(1)
				mipBlendSource = new Jit.MipBlend([source, source2], options)
					
				if layer.style.ShowAsElevation
					elevationModules.push({layer: layer, module:new Jit.Elevation(source)})

				# the theme will drive whether alpha of this layer should create
				# a land border
				if index is 0 and themeObject.landOnly
					options.applyToLandAlpha = true

				colorModules.push(new Jit.ColorBlend(mipBlendSource.gradient(palette), options))
			else
				geosource = geoSourceDataProvider.getGeoSource(layer.geoSource.Id, {})
				geoSources.push geosource
				source = new Jit.GeoSource(geosource, {
					hexSampling:true,
					borrowParentTexture:true,
					boundingSphere:boundingSphere
					})
				
				source2 = source.asLowResolution(1)
				mipBlendSource = new Jit.MipBlend([source, source2], options)
				
					
				colorModules.push(new Jit.ColorBlend(mipBlendSource, options))
		else
			sineEffect = undefined
			if layer.style.Fill and _.some(layer.style.Fill.Effects, (effect) -> effect.Type == 'SinePattern' && sineEffect = effect )
				geosource = geoSourceDataProvider.getGeoSource(layer.geoSource.Id, {style: layer.style})
				geoSources.push geosource
				source = new Jit.GeoSource(geosource,{
						style: layer.style
						hexSampling:true,
						borrowParentTexture:true,
						boundingSphere:boundingSphere
						})
						
				source2 = source.asLowResolution(1)
				mipBlendSource = new Jit.MipBlend([source, source2], options)
				
				colorModules.push(new Jit.Selection(mipBlendSource, sineEffect.Properties))
			else
				geosource = geoSourceDataProvider.getGeoSource(layer.geoSource.Id, {style: layer.style})
				geoSources.push geosource
				console.log "unknown layer type ?? ", layer
				source = new Jit.GeoSource(geosource,{
						style: layer.style
						hexSampling:true,
						borrowParentTexture:true,
						boundingSphere:boundingSphere
						})
				source2 = source.asLowResolution(1)
				mipBlendSource = new Jit.MipBlend([source, source2], options)
				options.globalAlpha = 0.6;
				colorModules.push(new Jit.ColorBlend(mipBlendSource, options))

	elevationSorting = (record) ->
		characterization = record.layer.characterization

		if characterization and characterization.NativeResolution
			return characterization.NativeResolution
		return 0

	elevationModules = _.sortBy(elevationModules,elevationSorting)

	for i in [0...elevationModules.length]
		elevationModules[i].module.elevationIndex = i
		jitShader.require(elevationModules[i].module)

	for i in [0...colorModules.length]
		jitShader.require(colorModules[i])

	# load the environtment first
	jitShader.require(new Jit.Environment())
	jitShader.require(new Jit.Phong())

	if themeObject.clipAnimation
		rhombusTree.clipAnimation = true
		rhombusTree.clipData = new THREE.Vector3(0.0, 0.0, 1.0)
		jitShader.require(new Jit.ClipAnimation())


	# if themeObject.debugLoading
	# always enable and use a uniform to turn on / off 
	jitShader.require(new Jit.DebugLoadingPriority())

	jitShader = jitShader.compile()

	vertexShader = jitShader.vertexCode
	fragmentShader = jitShader.fragmentCode

	#create attributes
	attributes = jitShader.attributes

	additionalUniforms = {}
	if themeObject.clipAnimation
		additionalUniforms["clipData"] = {type: "v3", value: rhombusTree.clipData, shared:true}
	
	additionalUniforms["screenSize"] = {type: "v2", value: rhombusTree.screenSize, shared:true}
	additionalUniforms["lightingParams"] = {type: "v4", value: new THREE.Vector4(1.0, 16.0, 1.0, 0.0), shared:true}
	additionalUniforms["time"] = {type: "v2", value: new THREE.Vector2(0.0, 0.0), shared:true}

	#create uniforms
	uniforms = SharedUniforms.merge(
		[
			THREE.UniformsLib[ "lights" ],
			# TODO ::
			themeObject.generateRhombusUniforms(),
			jitShader.uniforms,
			additionalUniforms
		]
	)

	# window.globalUniforms = uniforms

	shader = new THREE.RawShaderMaterial({
		wireframe: false
		side: THREE.FrontSide  # DoubleSide
		uniforms: uniforms
		vertexShader: vertexShader
		fragmentShader: fragmentShader
		lights:true
	})

	jitShader.geoSources = geoSources
	jitShader.elevations = elevationModules

	shader.combine = THREE.MixOperation

	shader.jit = jitShader
	shader.meshAttributes = attributes


	# RawShaderMaterial clone is broken, so we have to build our own
	shader.clone = ->
		material = new THREE.RawShaderMaterial()
		THREE.Material.prototype.clone.call( this, material )

		material.fragmentShader = this.fragmentShader
		material.vertexShader = this.vertexShader
		material.uniforms = SharedUniforms.clone( this.uniforms )
		material.attributes = SharedUniforms.clone( this.attributes )
		material.defines = this.defines
		material.shading = this.shading
		material.meshAttributes = cloneMeshAttributes( this.meshAttributes )

		material.wireframe = this.wireframe;
		material.wireframeLinewidth = this.wireframeLinewidth;
		material.fog = this.fog;
		material.lights = this.lights;
		material.vertexColors = this.vertexColors;
		material.skinning = this.skinning;

		material.morphTargets = this.morphTargets;
		material.morphNormals = this.morphNormals;

		material.jit = this.jit;

		return material


	return shader


###
	The default shader, with default lighting and vanilla settings will
	come later.  But this just provides an interface.
###
buildDefaultShader = (renderLayers, themeObject) ->





###
	ShaderFactory will read the themeObject and check if the specified theme
	provides a shader factory, otherwise using a default.

	TODO: implement default shader
###
shaderFactory = (rhombusTree, dataProviders, renderLayers, themeObject) ->
	return buildThemedShader(rhombusTree, dataProviders, renderLayers, themeObject)


module.exports = shaderFactory
