###
	DEPRECATED ---  Everything here has been moved to:  
		api/globe/canvas.coffee
		api/globe/globe.coffee


	This is a coffeescript file that compiles-to-javascript.

	The purpose here is to rapidly prototype behaviors and classes and then move them
	into the core engine js module once a piece of functionality is stable. -Dean


	For now, use jQuery for ajax data interaction
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
glslify = require 'glslify'
EventEmitter = require 'event-emitter'
createOrbitViewer = require('three-orbit-viewer')(THREE)
EffectComposer = require('three-effectcomposer')(THREE)
TrackballControls = require('../globe/lib/TrackballControls')(THREE)
Dispatcher = require('flux').Dispatcher
Color = require('color')


window.EventEmitter = EventEmitter










environmentMap = null
createEnvironmentMap = ->
	return environmentMap if environmentMap

	path = "/assets/images/textures/cube/SwedishRoyalCastle/"

	format = '.jpg'
	urls = [
			path + 'px' + format, path + 'nx' + format,
			path + 'py' + format, path + 'ny' + format,
			path + 'pz' + format, path + 'nz' + format
		]

	console.log "HAS reflect map ? ", THREE.CubeReflectionMapping
	#mapping = THREE.CubeReflectionMapping
	#TODO Use GlobeCanvas.threeCubeTextureLoader
	environmentMap = THREE.ImageUtils.loadTextureCube( urls)
	environmentMap.mapping= THREE.CubeReflectionMapping
	environmentMap.format=THREE.RGBFormat
	console.log environmentMap
	return environmentMap

createEnvironmentMap()
window.createEnvironmentMap = createEnvironmentMap



# NO longer using WAGNER post processing
# setup WAGNER post processing
# WAGNER.vertexShadersPath = 'engine/shaders/wagner/vertex-shaders'
# WAGNER.fragmentShadersPath = 'engine/shaders/wagner/fragment-shaders'
# WAGNER.assetsPath = 'assets/'
# depthTexture = null
# depthMaterial = null









# set to window for debugging
# window.copyCesiumCameraPosition = ->
#     window.GC.cameraParent.rotation.x = 0

#     c = window.CE.camera
#     c2 = window.GC.camera
#     d = window.GC.dummy
#     d.position.set( c.position.x, c.position.y, c.position.z )

#     v = c.viewMatrix
#     m = new THREE.Matrix4()
#     m.set(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15])
#     axisRotation = new THREE.Matrix4().makeRotationX(-1.0 * Math.PI/2)
#     # m.multiply(axisRotation)
#     # m.makeRotationX(-1.0 * Math.PI/2)
#     # dir = new THREE.Vector3(c.direction.x, c.direction.y, c.direction.z).normalize()
#     d.quaternion.setFromRotationMatrix(m)

#     # after everything rotate the camera parent so it is at the origin
#     window.GC.cameraParent.rotation.x = -1.0 * Math.PI / 2.0

#     # now copy dummy global position to camera
#     c2.position.setFromMatrixPosition( d.matrixWorld )
#     c2.quaternion.setFromRotationMatrix( d.matrixWorld )



#     # axisRotation = new THREE.Matrix4().makeRotationX(-1.0 * Math.PI/2)
#     # console.log("rotation -- ", axisRotation)
#     # c2.applyMatrix(axisRotation)

#     # v = c.viewMatrix
#     # m = new THREE.Matrix4()
#     # c2.matrixAutoUpdate = false
#     # c2.matrix.set(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15])
#     # c2.updateMatrixWorld( true )




###
	Take the current camera location and create a resonable size box around it.

	TODO :: save to map object so it can be used for geocard
###
window.boxCurrentLocation = ->
	console.log "box current location"

	config = window.globeConfig

	getCurrentCamera
	config.latMax = -1.0






###
	TODO :: merge interface with THREE Camera interface


###
class GlobeFrustum extends THREE.PerspectiveCamera
	_.extend GlobeFrustum.prototype, EventEmitter.prototype


	constructor: (fov, aspect, near, far)->
		self = this

		THREE.Camera.call( this )

		@type = 'PerspectiveCamera'
		@zoom = 1
		@offset = new GlobeOffsetFrustrum()
		@fov = undefined  # view angle in radians
		@fovVertical = undefined
		@aspectRatio = undefined
		@shadowCam = undefined

		@updateProjectionMatrix()


	# trigger update events and delegate to super class
	updateProjectionMatrix: ->
		@emit 'beforeUpdate'
		THREE.PerspectiveCamera.prototype.updateProjectionMatrix.call( this )
		@emit 'update'





###
	The goal of GlobeControls is to eventually separate out
	the default THREE TrackballControls into a framework
	that fits within the Flux paradigm
###
class GlobeControls extends THREE.TrackballControls
	constructor: (camera, domElement, options) ->
		self = this

		# @controls = new THREE.TrackballControls(camera, domElement)  # call TrackballControls prototype
		super(camera, domElement)

		@minDistance = 100.2 * SF
		@maxDistance = 2200.0 * SF

		@target.set( 0, 0, 0 )
		@rotateSpeed = 10.0
		@zoomSpeed = 1.8
		@panSpeed = 10.0
		@noRoll = true  # disable roll
		@noZoom = false
		@noPan = true  # until camera tilt is available, keep locked to the center
		@staticMoving = false
		@dynamicDampingFactor = 0.06
		@keys = [ 65, 83, 68 ]

		# move rotation variables to object scope
		@rotationAxis = new THREE.Vector3()
		@rotationQuaternion = new THREE.Quaternion()


	###
		Custom camera zoom.  Dampen motion as the camera is moved towards the surface, but
		use discrete chunks when near '2D/Flat'
	###
	zoomCamera: ->
		self = this
		# console.log "ZOOM CAMERA"

		if @state is @states.TOUCH_ZOOM_PAN
			factor = @touchZoomDistanceStart / @touchZoomDistanceEnd
			@touchZoomDistanceStart = @touchZoomDistanceEnd
			@eye.multiplyScalar( factor );
		else
			dampen = (@eye.length() - (@minDistance + 0.1)) / (@maxDistance - @minDistance)
			factor = 1.0 + ( @zoomEnd.y - @zoomStart.y ) * @zoomSpeed * dampen

			if factor isnt 1.0 and factor > 0.0
				@eye.multiplyScalar( factor )

				@zoomStart.y += ( @zoomEnd.y - @zoomStart.y ) * @dynamicDampingFactor


	###
		Custom rotation.  Needs to resemble more of a panning action when closer to the surface.

		Rotation might
	###
	rotateCamera: ->
		angle = Math.acos( @rotateStart.dot( @rotateEnd ) / @rotateStart.length() / @rotateEnd.length() )

		if angle
			@rotationAxis.crossVectors( @rotateStart, @rotateEnd ).normalize()

			dampen = Math.min((@eye.length() - (@minDistance - 0.01)) / (@maxDistance - @minDistance), 1.0)
			angle *= @rotateSpeed * dampen

			@rotationQuaternion.setFromAxisAngle( @rotationAxis, -1.0 * angle )

			@eye.applyQuaternion( @rotationQuaternion )
			@object.up.applyQuaternion( @rotationQuaternion )

			@rotateEnd.applyQuaternion( @rotationQuaternion )

			@rotateStart.copy( @rotateEnd )





###
	The goal of GlobeControls is to eventually separate out
	the default THREE TrackballControls into a framework
	that fits within the Flux paradigm


	@controls is reference to GlobeControls instance
###
class GlobeDispatcher extends Dispatcher

	# copy methods from right to left, adds EventEmitter to prototype chain
	 _.extend GlobeDispatcher.prototype, EventEmitter.prototype, GlobeDispatcher.prototype

	# apply singleton pattern for dispatcher (there should only be one even with multiple apps)
	instance: (options) ->
		return global.pyxisDispatcher if global.pyxisDispatcher

		global.pyxisDispatcher = new GlobeDispatcher(options)
		return global.pyxisDispatcher

	constructor: (options) ->
		options or= {}
		@config = _.extend @defaults, options

		@on 'resize', =>
			'asdf'







window.testBillboards = (icons) ->
	console.log 'test billboards'

	geo = new THREE.Geometry()
	sprite = THREE.ImageUtils.loadTexture('/assets/images/angular.png')

	# _.each _.range(200), (i) ->
	#     s = (Math.PI/2.0) - Math.random() * Math.PI
	#     t = (Math.PI) - Math.random() * Math.PI*2.0

	#     vert = new THREE.Vector3()
	#     vert.x = SCALE * 1.005 * Math.cos(s) * Math.sin(t)
	#     vert.y = SCALE * 1.005 * Math.sin(s) * Math.sin(t)
	#     vert.z = SCALE * 1.005 * Math.cos(t)

	#     geo.vertices.push vert

	# window.testIconLoader()
	# icons = window.ic.getIcons()

	_.each icons, (icon) ->
		vert = new THREE.Vector3(SCALE * 1.015 * icon.X, SCALE * 1.015 * icon.Z, SCALE * 1.015 * icon.Y * -1.0)
		geo.vertices.push vert

	matOpts =
		size: 26.0
		sizeAttenuation: false
		map: sprite
		transparent: true
	mat = new THREE.PointCloudMaterial( matOpts )

	particles = new THREE.Points( geo, mat )
	window.GC.add(particles)

	# hola b
	return particles



window.testBillboardGroup = (group, level) ->
	icons = []

	_.each group.Icons, (icon) -> icons.push(icon)

	_.each group.Groups, (group) ->
		console.log "group ", group.Id
		icons.push(group)

	window.testBillboards(icons)




'''

proxy.Bind("verifyMapSchema", (JsonString obj) =>
{
	var mapResource = JsonConvert.DeserializeObject<MapWithEmbeddedResources>(obj.ToString());

	if (mapResource.Id == Guid.Empty)
	{
		mapResource.Id = Guid.NewGuid();
	}
	return mapResource;
});

proxy.BindAsync("publishLocally", (GeoSource geoSource) =>
{
	return engine.PublishLocally(geoSource);
});

proxy.BindAsync("unpublishLocally", (GeoSource geoSource) =>
{
	return engine.UnpublishLocally(geoSource);
});

var settingProvider = new PublishSettingProvider();
settingProvider.Register(
	typeof(UploadImagePublishSetting),
	args =>
		{
		return Task<IPublishSetting>.Factory.StartNew(() => new UploadImagePublishSetting()
			{
			ImagePath = ImageStorage.FromResourceName((args as UploadImagePublishSettingArgs).ImageStorageUrl)
		});
	});

proxy.BindAsync("publishGeoSource", (GeoSource geoSource, Style style, Gallery gallery) =>
{
	//apply provider
	geoSource.Metadata.Providers = new List<Provider>
	{
			new Provider
			{
				Id = gallery.Id,
				Name = gallery.Metadata.Name,
				Type = ResourceType.Gallery,
			}
		};

	return engine.BeginPublish(geoSource, style, settingProvider).TransactionTask.Result.PublishedGeoSource;
});

var publishingTasksProgress = new List<PublishGeoSourceProgress>();

proxy.BindAsync("startPublishGeoSource", (GeoSource geoSource, Style style, Gallery gallery) =>
{
	//apply provider
	geoSource.Metadata.Providers = new List<Provider>
	{
		new Provider
		{
			Id = gallery.Id,
			Name = gallery.Metadata.Name,
			Type = ResourceType.Gallery,
		}
	};

	var publishProgress = engine.BeginPublish(geoSource, style, settingProvider);

	publishingTasksProgress.Add(publishProgress);

	return publishProgress.Status;
});

proxy.Bind("getPublishGeoSourceStatus", (GeoSource geoSource) =>
{
	var task = publishingTasksProgress.FirstOrDefault(p => p.ProvidedGeoSource.Id == geoSource.Id);
	if (task != null)
	{
		return task.Status;
	}
	return null;
});

proxy.BindAsync("finishPublishGeoSource", (GeoSource geoSource) =>
{
	var task = publishingTasksProgress.First(p => p.ProvidedGeoSource.Id == geoSource.Id);
	var result = task.TransactionTask.Result.PublishedGeoSource;

	publishingTasksProgress.Remove(task);
	return result;
});

proxy.BindAsync("publishMap", (MapWithEmbeddedResources map, Gallery gallery) =>
{
	//apply provider
	map.Metadata.Providers = new List<Provider>()
		{
			new Provider() {
				Id = gallery.Id,
				Name = gallery.Metadata.Name,
				Type = ResourceType.Gallery,
			}
		};

	//create backward support before we publish the map.
	map.Downgrade(engine);

	return engine.BeginPublish(map, settingProvider).TransactionTask.Result.PublishedMap;
});
'''









###
	http://geowebcore.pyxis.worldview.gallery:9678/api/rhombus/multi?
	http://uc.sxz.ca/layeredimage.php?key=03&scheme=7
	https://api.pyxis.worldview.gallery/api/v1/Map/1bab46fb-2b62-4e9f-aa4b-c939a383601f
###

'''
{
Key: "key",
Size: 244,
Channels: [
	{Channel: "R",GeoSource: "--ID--", Min: '0', Max: '100',Field: 'Field0'},
	{Channel: "G",GeoSource: "--ID--", Min: '0', Max: '100',Field: 'Field1'},
	{Channel: "B",GeoSource: "--ID--", Min: '0', Max: '100',Field: 'Field2'},
	{Channel: "A",GeoSource: "--ID--", Min: '0', Max: '100',Field: 'Field3'}
]
}
'''

module.exports = {}

