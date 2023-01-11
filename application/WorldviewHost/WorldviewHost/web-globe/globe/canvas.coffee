###
	This is a coffeescript file that compiles-to-javascript.

	Globe Canvas creates a THREEjs scene that contains a Globe widget
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
EffectComposer = require('three-effectcomposer')(THREE)
glslify = require 'glslify'
EventEmitter = require 'event-emitter'
Color = require 'color'

# local includes
Hosts = require '../core/hosts'
angles = require '../core/angles'
theme = require './theme'
Globe = require('./globe')  # THREEjs compatible globe
FocusCamera = require('./camera').FocusCamera
post = require('./shaders/post/postprocess')
SceneBlend = require('./shaders/post/sceneblend')
TemporalSuperSampling = require('./shaders/post/temporal-ss')
fontTexture = require('./shaders/text/font-texture')
TextureLoader = require('../utilities/loader').TextureLoader
LoadFeedbackService = require('../utilities/loader').LoadFeedbackService
Stats = require('./lib/stats.js')
BackgroundTasks = require('../utilities/background-tasks')

CameraController = require('./cameracontrol').CameraController
GeometryProvider = require('./geometry-provider').GeometryProvider
GeoSourceProvider = require('./geo-source-provider').GeoSourceProvider


# extendGlobeCanvas = require('./cesium2')
# buildCesium = require('./cesium')

# shader templates  -- shaders go through two passes: _.template
# globePhongVertexShader = glslify('../globe/shaders/globephong1-vs.glsl')


BACKDROP_VS = glslify('../globe/shaders/backdrop/backdrop-vs.glsl')
BACKDROP_FS = glslify('../globe/shaders/backdrop/backdrop-fs.glsl')


# use host defined in interface.coffee or this default
# window.gwcHost or= 'http://localhost:63036'
# window.gwcHost or= "http://192.168.120.159:8811"
# window.gwcHost or= "http://wvgwc.cloudapp.net:63036" # production

if _.isString(window.gwcHost)
	window.gwcHost = new Hosts(window.gwcHost)

else
	cdns = ( "https://#{cdn}.vo.msecnd.net" for cdn in [ "az804497", "az838716", "az839300" ] )
	window.gwcHost = new Hosts(cdns,"https://gwc.pyxis.worldview.gallery");

###
===================================
Global CONFIG

Use to attach variables to Dat.GUI and drive parameters

===================================
###

CONFIG =
	shaderColors:
		type: 'folder'
		name: 'Shader Colors'
		items: [
			{
				type: 'Color'
				name: 'Globe Background'
				value: Color().rgb(0.46*255, 0.46*255, 0.46*255).hexString()
			}
		]


# TODO: move into class


class GlobeDataModel
	constructor: (config) ->
		self = this
		@merge config

		@rgbMultiplex = 1.0

		@theme = theme.defaultConfig



	merge: (config) ->
		_.extend this, config


window.globeConfig = new GlobeDataModel(CONFIG)  # TODO: move away from window global




###
	==============================================

	Globe Implementation ported from Mark's setup

	==============================================
###

# cesium units are ~80,000 x as large
# radius of earh is 6378100 meters
# however, pyxis uses a sphere size of 6371007
SF = 63710
SCALE = 6371007

canvases = {}
PI = 3.141592654
FAR = 10 * SCALE
NEAR = 1000


THREE.ImageUtils.crossOrigin = ''
window.stopAnim = false



animate = ->
	return if window.stopAnim
	requestAnimationFrame( animate )

	render()

render = ->
	_.each canvases, (canvas, key) ->
		canvas.render()











###
	GlobeCanvas

	Events emitted, exposed to angular app:
	* resize
	* ready
	* start
	* dragstart
	* dragend
	* preloadLayers
	* preloadTitle
	* preloadLayersComplete
	* iconHoverStart
	* iconHoverEnd
	* cameraChange
	* cameraAnimationStart
	* cameraAnimationEnd

###
class GlobeCanvas
	constructor: (@container, @containerID, @config ) ->
		self = this
		window.GC = this # TODO :: get rid of global canvas references

		@backgroundTasks = new BackgroundTasks()

		# setup object as EventEmitter
		EventEmitter(self)

		@debug = false # turn on / off to enable debugging visualizations

		@syncGlobes = false
		@preloading = false
		@defaultSwapSceneAnimationTime = 2.0
		@defaultSwapScenePreloadTimeout = 5.0

		@dataProviders = {
			"geometry": new GeometryProvider(@backgroundTasks.category('geometry'))
			"geoSource" : new GeoSourceProvider(@backgroundTasks.category('geoSource'))
		}

		@globes = new Array()
		@isActive = true
		@focusCameras = new Array()
		@config or= window.globeConfig
		@theme = theme.default
		@threeTextureLoader = new THREE.TextureLoader()
		@threeTextureLoader.crossOrigin = 'Anonymous'

		if @config.defaultTheme
			console.log "CUSTOM DEFAULT THEME", @config.defaultTheme, theme
			@theme = theme[@config.defaultTheme]

		console.log "CANVAS CREATE XX -- ", @container, @config


		# @container = document.getElementById(@containerID)
		canvases[@containerID] = this


		@width = @container.clientWidth
		@height = @container.clientHeight

		@scene = @createScene()
		@swapScene = @createScene()



		# setup renderer
		@renderer = new THREE.WebGLRenderer( { antialias: true, alpha: true } )
		@renderer.setClearColor( 0x282828, 0 )
		@renderer.setSize( @width, @height )
		@renderer.domElement.style.position = "absolute"
		@renderer.domElement.style.top = "0px"
		@renderer.domElement.style.left = "0px"
		@renderer.shadowMap.enabled = false
		@renderer.shadowMap.type = THREE.PCFSoftShadowMap
		@renderer.gammaInput = true
		@renderer.gammaOutput = true

		#todo: move his into a module
		window.glSupport = {
			floatTextures: @renderer.extensions.get( 'OES_texture_float' )
		}

		# load system fonts into font canvas
		fontTexture.loadSystemFonts(this)

		# setting pixelratio was actually causing lots of issues for higher density screens
		# @renderer.setPixelRatio( window.devicePixelRatio )

		@container.appendChild( @renderer.domElement )

		@sceneBlendEffect = new SceneBlend.SceneBlendPass( @scene, @swapScene )
		if @theme.temporalSuperSamping
			@temporalSSPrepass = new TemporalSuperSampling.PrePass( @sceneBlendEffect.pass0, @sceneBlendEffect.pass1 )
			@temporalSSDrawPass = new TemporalSuperSampling.DrawPass( @temporalSSPrepass, @width, @height )

		@composer = new EffectComposer( @renderer )
		if @theme.temporalSuperSamping
			@composer.addPass( @temporalSSPrepass )
		@composer.addPass( @sceneBlendEffect )
		if @theme.temporalSuperSamping
			@composer.addPass( @temporalSSDrawPass )
		#@composer.addPass( new EffectComposer.RenderPass( @scene, @cameraController.camera ) )
		@postProcessPass = post.generateShaderPass(@theme)
		@composer.addPass( @postProcessPass )
		@postProcessPass.renderToScreen = true
		@scene.postProcessPass = @postProcessPass
		@swapScene.postProcessPass = @postProcessPass

		

		# allow other modules to hook into a single canvas resize event
		$(window).on 'resize', -> self.emit('resize')
		@on 'resize', -> self.fitToContainer()
		@fitToContainer()


		# now root is delegated to Globe Object
		# TODO:: as soon as multiple globes are supported, change from @root
		@scene.globe = new Globe(this, @theme, @dataProviders)
		@scene.threeScene.add(@scene.globe)

		@globe = @scene.globe

		console.log "Load loadFeedback", LoadFeedbackService
		@loadFeedback = new LoadFeedbackService(this)

		# @scene.cameraController.setDraggingGlobe(@globe)

		@setupCameraPolling()


		@startTime = new Date().getTime()
		animate()

		if window.themeControlsEnabled
			@theme.generateDatGui()


		setTimeout (-> self.emit 'ready'), 1000

		# double arrow to keep this ref
		@on 'ready', =>
			@isReady = true
			# @globe.loadLayers() # do we need this?

			# TweenMax.to @camera.position, 2.0, {x: -40.0*SF*0.2, y: 300.0*SF*0.2, z: -2000.0*SF*0.2}
			setTimeout (-> self.emit 'start'), 300

		# separate from 'ready' event, which includes animation staging
		@on 'start', =>
			@scene.cameraController.onStart()

		unless @config.disableMouseControls
			@bindMouseControls()

		unless @config.disableZoomControls
			@bindZoomControls()

		if false
			@stats = new Stats();
			@stats.domElement.style.position = 'absolute';
			@stats.domElement.style.bottom = '20px';
			@stats.domElement.style.left = '20px';
			@container.appendChild( @stats.domElement );


	# we need to support binding controls externally for 
	# the landing page to demo transition
	bindZoomControls: ->
		$(@renderer.domElement).on 'mousewheel', (event) =>
			@scene.cameraController.onMouseWheel(event.originalEvent.wheelDelta)
		$(@renderer.domElement).on 'dblclick', (event) =>
			@scene.cameraController.zoom(0.5)

	bindMouseControls: ->
		@windowEventsBounded = false
		mousemove = (event) => @scene.cameraController.onMouseMove(@renderer.domElement, event)
		mouseup = (event) =>
			@windowEventsBounded = false
			$(window).off 'mouseup', mouseup
			$(window).off 'mousemove', mousemove
			return @scene.cameraController.onMouseUp(@renderer.domElement, event)

		$(@renderer.domElement).on 'mousemove', (event) =>
			if !@windowEventsBounded
				return @scene.cameraController.onMouseMove(@renderer.domElement, event)

		$(@renderer.domElement).on 'mousedown', (event) =>
			@windowEventsBounded = true
			$(window).on 'mouseup', mouseup
			$(window).on 'mousemove', mousemove
			return @scene.cameraController.onMouseDown(@renderer.domElement, event)

	createScene: ->
		scene = {}

		lightIntensity = @theme.lightIntensity 

		scene.threeScene = new THREE.Scene()
		scene.cameraController = new CameraController(this, scene.threeScene, @width, @height)
		#sunLight = new THREE.SpotLight( 0x888888, 0.4, 0, Math.PI/2, 1 )
		sunLight = new THREE.DirectionalLight(0xE8B58D, lightIntensity)
		sunLight.position.set( -1000, -2000, -1000 )
		sunLight.castShadow = true
		sunLight.shadowDarkness = 0.3 * 1
		sunLight.shadowBias = -0.0002
		scene.light0 = sunLight
		scene.threeScene.add(sunLight)

		sunLight = new THREE.DirectionalLight(0xE8B58D, lightIntensity)
		sunLight.position.set( 1000, 2000, 1000 )
		sunLight.lookAt(0,0,0)
		sunLight.castShadow = true
		sunLight.shadowDarkness = 0.3 * 1
		sunLight.shadowBias = -0.0002
		scene.threeScene.add(sunLight)
		scene.light1 = sunLight

		@createBackDrop(scene)
		
		scene.cameraController.on 'dragstart', (event) =>
			@emit 'dragstart'
			
		scene.cameraController.on 'dragend', (event) =>
			@emit 'dragend'

		return scene

	preloadLayers: (layers, options, callback) ->
		@preloadLayersDelay = [layers, options, callback]

	preloadLayersDelayed: (layers, options, callback) ->
		if @swapSceneAnimation
			throw "can't start preloading while swap animation in progress"

		loadLayers = true

		if !@preloading
			if @swapScene.globe is undefined
				@swapScene.globe = new Globe(this, @theme, @dataProviders, layers)
				# @swapScene.cameraController.setDraggingGlobe(@swapScene.globe)
				@swapScene.threeScene.add @swapScene.globe
				loadLayers = false

			@preloading = true
			@syncGlobes = true

			@emit 'preloadLayers' # hook for loading events

			@preloadingStartTime = new Date().getTime()

			# on this operation use the map object to set the current theme
			if window.getCurrentMapModel && window.getCurrentMapModel()
				model = window.getCurrentMapModel()
				if model.Metadata.Theme
					this.theme = theme.themeFactory(theme.lightThemeConfig)
					this.theme.set(model.Metadata.Theme)

				if model.Metadata.Name
					@emit 'preloadTitle', model.Metadata.Name

		if loadLayers
			@swapScene.globe.loadLayers(layers, options)
		@preloadCallback = callback

		return null

	preloadingTimeout: () ->
	 	time = new Date().getTime() - @preloadingStartTime
	 	timeInSeconds = time / 1000.0
	 	return timeInSeconds > @defaultSwapScenePreloadTimeout

	isPreloading: ->
		return true if @preloading

	isSwapSceneActive: ->
		return @swapSceneAnimation

	doSwapScene: (time, callback) ->
		@swapSceneAnimation = true
		@swapSceneTimer = if time then time else @defaultSwapSceneAnimationTime
		@swapSceneAnimationTime = @swapSceneTimer
		@swapSceneFinishCallback = callback
		@theme.updateUniforms(@swapScene)

	onSwapSceneComplete: () ->
		oldScene = @scene
		@scene = @swapScene
		@globe = @scene.globe
		@swapScene = oldScene
		@swapSceneAnimation = false
		@syncGlobes = false
		@sceneBlendEffect.swapPasses()
		@setActiveCamera(@getCamera())

		@swapSceneFinishCallback() if @swapSceneFinishCallback
		@swapSceneFinishCallback = undefined

		# @swapScene.cameraController.setDraggingGlobe(undefined)
		@swapScene.threeScene.remove(@swapScene.globe)
		@swapScene.globe.dispose()
		@swapScene.globe = undefined


		@emit 'preloadLayersComplete' # hook for loading events

	setActiveCamera: (camera) ->
		@scene.cameraController.setActiveCamera(camera)

		@sceneBlendEffect.pass0.camera = camera

	syncScenes: (srcScene, dstScene) ->
		dstScene.light0.position.copy(srcScene.light0.position)
		dstScene.light1.position.copy(srcScene.light1.position)

		dstScene.cameraController.copy(srcScene.cameraController)

	getCamera: () ->
		return @scene.cameraController.camera


	createBackDrop: (scene) ->
		bc = Color(@theme.backgroundColor)
		bc2 = Color(@theme.backgroundColor2)
		
		options =
			uniforms:
				resolution: { type: 'v2', value: new THREE.Vector2( 0, 0 ) }
				noise: { type: 'f', value: .04 }
				backgroundColor: { type: 'v3', value: new THREE.Vector3(bc.red() / 255.0, bc.green() / 255.0, bc.blue() / 255.0)}
				backgroundColor2: { type: 'v3', value: new THREE.Vector3(bc2.red() / 255.0, bc2.green() / 255.0, bc2.blue() / 255.0)}
			vertexShader: BACKDROP_VS #glslify('./shaders/backdrop/backdrop-vs.glsl')
			fragmentShader: BACKDROP_FS #glslify('./shaders/backdrop/backdrop-fs.glsl')
			side: THREE.BackSide
		scene.backdropMaterial = new THREE.ShaderMaterial(options)

		geometry = new THREE.Geometry()

		geometry.vertices.push(
			new THREE.Vector3( -1, -1, 0 ),
			new THREE.Vector3(  1, -1, 0 ),
			new THREE.Vector3(  1,  1, 0 ),
			new THREE.Vector3( -1,  1, 0 )
		)

		geometry.faces.push( new THREE.Face3( 1, 0, 2 ) )
		geometry.faces.push( new THREE.Face3( 2, 0, 3 ) )

		scene.backdropSphere = new THREE.Mesh( geometry, scene.backdropMaterial )
		scene.backdropSphere.frustumCulled = false
		scene.threeScene.add scene.backdropSphere


	fitToContainer: ->
		console.log "FIT CONTAINER ", window.innerWidth, window.innerHeight

		# @controls.handleResize()
		@width = window.innerWidth #@container.clientWidth
		@height = window.innerHeight #@container.clientHeight
		@scene.cameraController.setSize(@width, @height)
		#TODO remove this, we just need to copy all data
		@swapScene.cameraController.setSize(@width, @height)

		@renderer.setSize( @width, @height )
		@composer.setSize( @width, @height )
		@sceneBlendEffect.setSize( @width, @height )
		
		if @temporalSSPrepass and @temporalSSDrawPass
			@temporalSSPrepass.setSize( @width, @height )
			@temporalSSDrawPass.setSize( @width, @height )

		#$('#cesiumContainer').width(@width).height(@height)
		$(@container).width(@width).height(@height)

		# $('#ui').height( @height )
		# @composer.setSize( @renderer.domElement.width, @renderer.domElement.height )
		# depthTexture = WAGNER.Pass.prototype.getOfflineTexture( @composer.width, @composer.height, false )

		# set our resolution uniforms
		@scene.backdropMaterial.uniforms.resolution.value.set(@width, @height);
		@swapScene.backdropMaterial.uniforms.resolution.value.set(@width, @height);
		@t = 0.0

	add: (item) -> @scene.threeScene.add(item)
	remove: (item) -> @scene.threeScene.remove(item)

	update: (dt) ->
		if @preloadLayersDelay and not @swapSceneAnimation
			@preloadLayersDelayed(@preloadLayersDelay[0], @preloadLayersDelay[1], @preloadLayersDelay[2])
			@preloadLayersDelay = undefined

		for k, v of @dataProviders
			v.update()

		if(@prevTime != undefined)

			if @theme.lightsAnimiation
				matrix = new THREE.Matrix4().makeRotationZ((dt-@prevTime)*0.25);
				@scene.light0.position.applyMatrix4(matrix)
				@scene.light1.position.applyMatrix4(matrix)
				@scene.light1.position.applyMatrix4(matrix)

			if @swapSceneAnimation
				@swapSceneTimer = @swapSceneTimer - (dt-@prevTime)

				@sceneBlendEffect.blendValue = 1.0 - @swapSceneTimer / @swapSceneAnimationTime

				if @swapSceneTimer < 0.0
					@swapSceneTimer = 0.0
					@swapSceneAnimation = false
					@onSwapSceneComplete()

		#set global time and frame time
		@globe.tree.shaderFactory.uniforms['time'].value.set(dt, dt-@prevTime)

		@prevTime = dt

		for g in @globes
			g.update(dt)

		@scene.cameraController.update()

		if @syncGlobes
			@syncScenes(@scene, @swapScene)
			@swapScene.cameraController.update()

		# throttled call to 200ms
		@cameraPoll()
		@loadFeedback.update()

		# please don't remove this return true.
		# Without the return, the above for statement will create an array with all meshes that consume time and memory
		return true


	raycast: (x,y) ->
		return @scene.cameraController.raycast(@renderer.domElement, x, y)

	getMouseIntersection: () ->
		@scene.cameraController.getMouseIntersections()

	getMouseGeometryOnGlobe: ->
		return @scene.cameraController.getMouseGeometryOnGlobe()
		
	getMouseClickTrace: () ->
		return @scene.cameraController.mouseClickTrace 
	
	getMouseClickMetadata: ->
		return @scene.cameraController.getMouseClickMetadata() 

	render: ->
		self = this
		renderStart = new Date()
		timeDelta = new Date() - @startTime
		timeDeltaSeconds = timeDelta / 1000.0
		window.globeConfig.time = timeDeltaSeconds

		# TODO :: move this out of the render function
		@update(timeDeltaSeconds)
		# @controls.update(timeDeltaSeconds)

		# copy camera coordinates from Cesium viewer
		#if @copyCameraPosition
		#    window.copyCesiumCameraPosition()

		# console.log("TD ", timeDelta)

		# # standard render
		# @renderer.render(@scene, @camera)
		if @scene.cameraController.isCameraMatrixChanged()
			@scene.globe.invalidateVisibilityList()
			@swapScene.globe.invalidateVisibilityList() if @syncGlobes

			if @temporalSSPrepass
				@temporalSSPrepass.resetFrames()

		@globe.updateWithCamera(@scene.cameraController.camera, { width:@width, height:@height })

		@scene.cameraController.storeCameraMatrix()

		if @syncGlobes
			@swapScene.globe.updateWithCamera(@swapScene.cameraController.camera, { width:@width, height:@height })
			@swapScene.cameraController.storeCameraMatrix()

		if @swapScene.globe != undefined and @preloading
			if @swapScene.globe.tree.loaded() || @preloadingTimeout()
				@preloading = false
				@preloadCallback() if @preloadCallback
				@preloadCallback = undefined

		# _.each @iconTrees, (iconTree) ->
		#     iconTree.emit 'update', self.camera
		@composer.render()
		for fc in @focusCameras
			fc.render()

		@scene.cameraController.updateMousePositionOnGlobe(@renderer.domElement);
		if @stats
			@stats.update();

		@scene.globe.theme.setAltitudeParameters(@scene, @scene.cameraController.camera.position.length() - wgs84.earthRadius)

		timeSpentSoFar = new Date() - renderStart
		@backgroundTasks.run(Math.max(5, 15 - timeSpentSoFar))

		return null


	# we have to apply this to both scenes otherwise spin will start again when
	# the swapscene is loaded
	stopGlobeSpin: -> 
		@scene.cameraController.stopGlobeSpin()
		@swapScene.cameraController.stopGlobeSpin() if @swapScene  

	startGlobeSpin: -> 
		@scene.cameraController.startGlobeSpin()
		@swapScene.cameraController.startGlobeSpin() if @swapScene 
	
	###
		return an interface for loading the application
	###
	loadingAnimation: ->
		console.log 'setup loading animation'
		self = this

		# create wireframe sphere
		sphere = new THREE.IcosahedronGeometry(SF * 100.0, 3)
		# sphere = new THREE.BoxGeometry( SF * 100.0, SF * 100.0, SF * 100.0 )
		sphereMaterial = new THREE.MeshBasicMaterial( { wireframe: true, wireframeLinewidth: 1.1, color: 0xaaaaaa } )
		sphereMesh = new THREE.Mesh( sphere, sphereMaterial )
		@scene.threeScene.add( sphereMesh )

		loadingCamera = new THREE.PerspectiveCamera( 45, @width / @height, 0.2*SF, FAR )
		loadingCamera.position.set( 40*SF,  300.87928477802794*SF,  1200.0*SF)
		loadingCamera.lookAt( new THREE.Vector3(0,0,0) )
		@scene.threeScene.add loadingCamera
		@setActiveCamera loadingCamera

		window.sphereMesh = sphereMesh # DEBUG
		window.loadingCamera = loadingCamera

		scaleAnim = new TweenMax.from(sphereMesh.scale, 0.6, {x: 1.2, y: 1.2, z: 1.2, ease: Power4.easeOut})
		spinAnim = new TweenMax.to(sphereMesh.rotation, 20.0, {y: 3.5})
		# blurAnim = new TweenMax.fromTo(@tiltShift.uniforms.v, 6.0, {value: 0.02, ease: Power4.easeOut}, {value: 0.0022})

		@globe.visible = false

		# create our animation
		@on 'ready', =>
			console.log 'animation exit'

			$('#loading-message').remove()
			spinAnim.kill()

			sphereMaterial.color = new THREE.Color( 0x666666 )
			sphereMaterial.wireframeLinewidth = 4.0
			sphereMaterial.opacity = 0.5
			spot = sphereMesh.rotation.y + 0.05
			spinAnim = new TweenMax.to(sphereMesh.rotation, 0.5, {y: spot, ease: Power4.easeOut})
			# blurAnim = new TweenMax.to(self.tiltShift.uniforms.v, 1.0, {value: 0.0022, ease: Power4.easeOut})

			# window.copyCesiumCameraPosition()
			#console.log "CAMERA POS ", self.dummy.position, self.camera.position
			animatedCameraSwitch loadingCamera, self.camera, ->
				console.log "animation switch camera"
				self.setActiveCamera self.camera
				sphereMesh.visible = false
				self.globe.visible = true

				setTimeout (-> $('#ui').show()), 1000
				#setTimeout (-> self.theme.generateDatGui()), 1000
			# cameraAnim = new TweenMax.to(self.camera)


	# DELEGATE TO Globe Object
	loadLayers: (layers, options) ->
		self = this

		# on this operation use the map object to set the current theme
		if window.getCurrentMapModel && window.getCurrentMapModel() 
			model = window.getCurrentMapModel()
			if model.Metadata.Theme
				self.theme.set(model.Metadata.Theme)

		@globe.loadLayers(layers, options)


	isGeoSourceLoading: (geoSourceId) ->
		return @dataProviders["geoSource"].isGeosourceIsLoading(geoSourceId)


	disableLoading: ->
		@globe.freeze()
		if @swapScene.globe
			@swapScene.globe.freeze()
		#_.each @dataProviders.geoSource.geoSources, (gs) ->
		#	gs.textureLoader.sleep()

	enableLoading: ->
		@globe.unfreeze()
		if @swapScene.globe
			@swapScene.globe.unfreeze()
		#_.each @dataProviders.geoSource.geoSources, (gs) ->
		#	gs.textureLoader.wake()


	###
		Simply watches the position of the camera to see if it is updating,
		while the camera is animating with disableLoading and when the camera
		stops moving we re-enable and emit a "cameraAnimationEnd" event

		Note it is here rather than cameraController to allow global loaders
		to listen to the event.
	###
	setupCameraPolling: ->
		self = this
		isAnimating = false
		cameraPosition = self.scene.cameraController.camera.position
		lastCameraPosition = self.scene.cameraController.camera.position

		updateCam = ->
			cam = self.scene.cameraController.camera.position
			if isAnimating
				if cam.equals(cameraPosition) and cam.equals(lastCameraPosition)
					self.emit('cameraAnimationEnd')
					isAnimating = false
			else
				unless cam.equals(cameraPosition)
					self.emit('cameraAnimationStart')
					isAnimating = true

			lastCameraPosition = cameraPosition.clone()
			cameraPosition = cam.clone()


		@cameraPoll = _.throttle(updateCam, 200)


		if @config.stopLoadingWhileCameraMove
			@on 'cameraAnimationStart', ->
				self.disableLoading()

			@on 'cameraAnimationEnd', ->
				self.enableLoading()



	# delegate to cameracontroller
	animateCamera: (camera, options, callback) ->
		return @scene.cameraController.animateCamera(camera, options, callback)

	###
		Given an xyz position as Vector3, project back through the camera to get
		the screenspace xy coordinate.
	###
	xyzToScreenCoordinates: (position) ->
		v = position.clone().project(@scene.cameraController.camera)

		v.x = (v.x + 1) / 2 * @scene.cameraController.width
		v.y = -(v.y - 1) / 2 * @scene.cameraController.height

		return v


	# provide as {lat: N, lon: N}
	latlonToScreenCoordinates: (latlon) ->
		return @xyzToScreenCoordinates(wgs84.latLonToXyz(latlon))

	###
		Provide a count of the assets that are currently loading
		by searching through all of the trees in both scenes
	###
	getLoadingStatus: ->
		data =
			requests: 0
			requestsRunning: 0
			textureRequests: 0
			textureRequestsRunning: 0
			iconRequests: 0
			iconRequestsRunning: 0
			visibleKeys: @globe.tree.visibleKeys
			total: 0

		# let's ignore geometry and icon providers for now
		geometryRequests = @dataProviders.geometry.getLoadingStatus()
		# data.requests += geometryRequests.requestsPending
		# data.requestsRunning += geometryRequests.requestsInProgress

		# only return requests on visible keys
		textureRequests = @dataProviders.geoSource.getLoadingStatus(data)
		# data.textureRequests = textureRequests.requestsPending
		# data.textureRequestsRunning = textureRequests.requestsInProgress

		# if @globe
		# 	_.each @globe.iconTrees, (t) ->
		# 		data.iconRequests += t.loader.requests.length
		# 		data.iconRequestsRunning += t.loader.requestsRunning

		# if @swapScene and @swapScene.globe
		# 	_.each @swapScene.globe.iconTrees, (t) ->
		# 		data.iconRequests += t.loader.requests.length
		# 		data.iconRequestsRunning += t.loader.requestsRunning

		# data.requests = data.textureRequests + data.iconRequests
		# data.requestsRunning = data.textureRequestsRunning + data.iconRequestsRunning
		return data



	# ###
	#     Delegate load to tree
	# ###
	# loadMap: (map, options) ->
	#     self = this

	#     # this is pretty ugly for now...
	#     console.log "Map Load x", map, window.globeConfig.sourceID
	#     window.globeCurrentMap = map
	#     @map = map
	#     @root.tree.loadMap(map, options)

	# create icon trees from the map
	# TODO:: call destroy function if we find memory leaks


	# updateStyle: (id, style) ->
	#     @map.Groups[0].Items[0].Style = style
	#     @root.tree.loadMap(@map)




###
	Goal:  we have a target camera that we want to match position with, so we
	animate a dummy camera to it and then swap it out when they are aligned
###
animatedCameraSwitch = (camera, targetCamera, callback) ->

	origin = new THREE.Vector3(0, 0, 0)

	options =
		x: targetCamera.position.x
		y: targetCamera.position.y
		z: targetCamera.position.z
		ease: Power4.easeInOut
		onComplete: callback
		onUpdate: ->
			camera.lookAt(origin)
			camera.updateProjectionMatrix()

	console.log "ANIMATE TO :: ", options, targetCamera.position
	posAnimation = new TweenMax.to(camera.position, 0.8, options)
	# camera.updateProjectionMatrix()
	# camera.lookAt(new THREE.Vector3(0,0,0))

	rotOptions =
		x: targetCamera.quaternion.x
		y: targetCamera.quaternion.y
		z: targetCamera.quaternion.z
		w: targetCamera.quaternion.w
		ease: Power4.easeInOut
	# rotAnimation = new TweenMax.to(camera.quaternion, 3.0, rotOptions)




window.themeControls = ->
	window.GC.theme.generateDatGui()

#For debuging temporal super sampling
window.toggleTSS = ->
	return unless window.GC.temporalSSPrepass and window.GC.temporalSSDrawPass
	if window.GC.temporalSSPrepass.enabled
		window.GC.temporalSSPrepass.enabled = false
		window.GC.temporalSSDrawPass.enabled = false
	else
		window.GC.temporalSSPrepass.enabled = true
		window.GC.temporalSSDrawPass.enabled = true


window.GlobeCanvas = GlobeCanvas


# export only one Class
module.exports = GlobeCanvas