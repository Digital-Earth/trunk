###
	This is a coffeescript file that compiles-to-javascript.

	CameraController provides logic for interaction on the Globe with a mouse
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
EffectComposer = require('three-effectcomposer')(THREE)
glslify = require 'glslify'
EventEmitter = require 'event-emitter'
Color = require 'color'

# local includes
wgs84 = require '../core/wgs84'
angles = require '../core/angles'
theme = require './theme'
Globe = require('./globe')  # THREEjs compatible globe
FocusCamera = require('./camera').FocusCamera

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

# used for mouseOnGlobe projection, but to avoid creating it every frame
referenceSphere = new THREE.Sphere(new THREE.Vector3(0,0,0), wgs84.earthRadius)
identityQuaternion = new THREE.Quaternion()

class MouseClickTrace
	constructor: (mouse, globeLocation) ->
		@mouseDownTime = new Date()
		@mouseLocations = []
		@dragLength = 0
		@addMouseLocation('mousedown', mouse, globeLocation, @mouseDownTime)

	addMouseLocation: (eventType, mouse, globeLocation, time) ->
		time = new Date() unless time
	
		mouseLocation =
			type: eventType,
			pos: { top: mouse.top, left: mouse.left}
			time: (time-@mouseDownTime) / 1000.0

		mouseLocation.location = globeLocation.clone() if globeLocation

		@mouseLocations.push mouseLocation

		if eventType == 'mousemove'
			size = @mouseLocations.length
			p1 = @mouseLocations[size-2].pos
			p2 = @mouseLocations[size-1].pos
			@dragLength += Math.sqrt( Math.pow(p1.left-p2.left,2) + Math.pow(p1.top-p2.top,2) )

		if eventType == 'mouseup'
			@mouseUpTime = time
			@mouseClickLength = (@mouseUpTime - @mouseDownTime) / 1000.0

	updateLastLocation: (mouse, globeLocation) ->
		return if !globeLocation

		size = @mouseLocations.length
		return if size == 0
		return if @mouseLocations[size-1].type != 'mousemove'

		lastPos = @mouseLocations[size-1].pos
		return if lastPos.top != mouse.top or lastPos.left != mouse.left

		@mouseLocations[size-1].location = globeLocation
		
	totalPosDelta: () ->
		if @mouseLocations.length < 2
			return { x: 0, y:0 }
		first = @mouseLocations[0].pos
		last = @mouseLocations[@mouseLocations.length-1].pos
		return { x: last.left-first.left, y: last.top - first.top }
		
	lastPosDelta: () ->
		if @mouseLocations.length < 2
			return { x: 0, y:0 }
		from = @mouseLocations[@mouseLocations.length-2].pos
		to = @mouseLocations[@mouseLocations.length-1].pos
		return { x: to.left-from.left, y: to.top - from.top }

clamp = (min,max,value) ->
	return min if (value < min)
	return max if (value > max)
	return value

class CameraController
	constructor: (@globeCanvas, @scene, @width, @height) ->

		EventEmitter(this)

		@camera = new THREE.PerspectiveCamera( 45, @width / @height, NEAR, FAR )
		@camera.position.set( -40*SF,  200*SF,  -200.0*SF)
		@camera.lookAt( new THREE.Vector3(0,0,0) )
		@scene.add @camera


		# keep a reference to the active camera so it can be toggled while loading etc.
		@activeCamera = @camera
		@globeCamera = new GlobeCamera(@camera)

		@cameraCheckRadius = @globeCanvas.theme.rangeMin # in meters
		@rangeUpdate() # make sure rangeRatio is set

		@mouseSphere = new THREE.Mesh(new THREE.SphereGeometry(0.5*SF,32,32), new THREE.MeshBasicMaterial( {color: 0xffff00} ))
		# @scene.add(@mouseSphere)

		@mouseButtonDown = false
		@dragging = false
		@animating = false
		@moved = false
		
		@lastMouseDelta = { x:0, y:0 }

		@mouse = new THREE.Vector2(0.0, 0.0)

		@raycaster = new THREE.Raycaster()
		@storedCameraMatrix = new THREE.Matrix4()

		@zoomToPointer = true
		
		config = {}
		config = @globeCanvas.config.camera if @globeCanvas.config.camera 
		
		@dragSpeed = 0.9
		@dragStopSpeed = 0.1
		@zoomAnimationSpeed = 0.25

		# spin speed when globe spin is enabled
		@globeSpinEnabled = @globeCanvas.config.enableGlobeSpin
		@globeSpinAmount = @globeCanvas.config.globeSpinAmount or 0.15
		
		@dragSpeed = config.dragSpeed if config.dragSpeed != undefined
		@dragStopSpeed = config.dragStopSpeed if config.dragStopSpeed != undefined
		@zoomAnimationSpeed = config.zoomAnimationSpeed if config.zoomAnimationSpeed != undefined

		# we keep rangeMin / rangeMax to define a 0-1 rangeRatio (which is used to scale things)
		# but allow for limits to be defined separately from that
		@rangeLimitMin = @globeCanvas.config.rangeLimitMin or @globeCanvas.theme.rangeMin
		@rangeLimitMax = @globeCanvas.config.rangeLimitMax or @globeCanvas.theme.rangeMax

		@targetRange = undefined

		# this was a real pain to figure out, apparently it needs world units for the threshold
		# to work on point clouds. Put the number too small and you won't hit anything, too large
		# and everything will hit
		# SF is earth radius meters / 100
		@raycaster.params.Points.threshold = SF * 10.0

		@mouseOnGlobe = undefined
		@mouseDownOnGlobe = undefined

		@mouseCursor = 'default'
		@lastMouseCursor = 'default'

		# we only care about the start of the movement, so this emitter is throttled
		@emitCameraMoveEvent = => @globeCanvas.emit('cameraChange')
		@emitCameraMoveEvent = _.throttle(@emitCameraMoveEvent, 200)


		# trackball controls
		#THREE.TrackballControls = window.THREE.TrackballControls  # until dependencies are resolved
		# @controls = new GlobeControls( @camera, @renderer.domElement )
		# @controls = new THREE.TrackballControls( @camera, @renderer.domElement )

	copy: (src) ->
		@mouseCursor = src.mouseCursor
		# @lastMouseCursor = src.lastMouseCursor
		@dragging = src.dragging
		@moved = src.moved
		@mouse.copy(src.mouse)
		if @mouseOnGlobe is undefined || @mouseOnGlobe is null
			@mouseOnGlobe = new THREE.Vector3(0.0, 0.0, 0.0)
		if src.mouseOnGlobe != undefined && src.mouseOnGlobe != null
			@mouseOnGlobe.copy(src.mouseOnGlobe)

		if @mouseDownOnGlobe is undefined || @mouseOnGlobe is null
			@mouseDownOnGlobe = new THREE.Vector3(0.0, 0.0, 0.0)
		if src.mouseDownOnGlobe != undefined && src.mouseDownOnGlobe != null
			@mouseDownOnGlobe.copy(src.mouseDownOnGlobe)
		if src.lastRotation
			@lastRotation = src.lastRotation.clone()
		else
			@lastRotation = null

		@width = src.width
		@height = src.height
		@rangeRatio = src.rangeRatio
		@raycaster.params.Points.threshold = src.raycaster.params.Points.threshold
		@globeCamera.copy(src.globeCamera)
		@camera.copy(src.camera)
		@globeCamera.copy(@camera)
		
		@targetRange = src.targetRange

		@mouseSphere.position.copy(src.mouseSphere.position)


	setMouseCursor: (domElement) ->
		return if @mouseCursor is @lastMouseCursor

		# console.log "update mouse", @mouseCursor, @lastMouseCursor

		if @mouseCursor is 'default'
			domElement.style.cursor = 'default'
		if @mouseCursor is 'move'
			domElement.style.cursor = 'move'
		if @mouseCursor is 'grab'
			domElement.style.cursor = '-webkit-grab'
		if @mouseCursor is 'grabbing'
			domElement.style.cursor = '-webkit-grabbing'
		if @mouseCursor is 'pointer'
			domElement.style.cursor = 'pointer'

		@lastMouseCursor = @mouseCursor

	setSize: (width, height) ->
		@width = width
		@height = height
		@camera.aspect = @width / @height
		@camera.updateProjectionMatrix()

	# we don't need this anymore and it was causing issues
	# when undefined is passed in
	# use @globeCanvas.scene.globe
	setDraggingGlobe: (globe) ->
		@globe = globe

	setActiveCamera: (camera) ->
		@activeCamera = camera


	update: ->
		@updateClipPlanes()

		if @globeSpinEnabled
			@updateGlobeSpin()

	updateClipPlanes: () ->
		
		rangeMax = @globeCanvas.theme.rangeMax  # max range in meters
		rangeMin = @globeCanvas.theme.rangeMin  # min range in meters

		#todo: better change to real camera elevation sampling.
		minDistance = @globeCamera.range * Math.cos(angles.radians(@globeCamera.tilt))
		
		scaleNearFar = clamp(0,1,minDistance / NEAR)
		scaleNearFar = Math.floor(scaleNearFar*10) / 10;
		
		nearAndFar = [NEAR * scaleNearFar, FAR * scaleNearFar]

		# if @globeCamera.range < NEAR * 20
		# 	nearAndFar = [NEAR / 20.0,FAR / 500.0]
		# else if @globeCamera.range < NEAR * 100
		# 	nearAndFar = [NEAR / 10.0,FAR / 100.0]
		# else if @globeCamera.range < NEAR * 500
		# 	nearAndFar = [NEAR / 3.0,FAR / 10.0]
		# else 
		# 	nearAndFar = [NEAR,FAR]

		if @camera.near != nearAndFar[0]
			@camera.near = nearAndFar[0]
			@camera.far  = nearAndFar[1]
			@camera.updateProjectionMatrix()


	###
		Tweak settings that are based off of how close the camera is to the
		surface of the globe. Here I adjust raycast threshold.
	###
	rangeUpdate: ->
		rangeMax = @globeCanvas.theme.rangeMax  # max range in meters
		rangeMin = @globeCanvas.theme.rangeMin  # min range in meters

		# 0 is at surface, 1 is furthest away
		@rangeRatio = clamp(0.0,1.0,(@globeCamera.range - rangeMin) / (rangeMax - rangeMin))

	updateAltitude: ->
		target = wgs84.latLonToXyz({lat:@globeCamera.latitude,lon:@globeCamera.longitude})
	
		altitude = @globeCanvas.scene.globe.tree.getLocationElevation(target)
		if altitude
			@globeCamera.copy(@camera)
			@globeCamera.altitude = altitude
			referenceSphere.radius = wgs84.earthRadius + altitude 
			@globeCamera.apply(@camera)
			
	_animateRangeChange: ->
		if @targetRange
			if Math.abs(@targetRange - @globeCamera.range) > @targetRange * 0.05
				if @zoomToPointer
					@camera.updateMatrixWorld();
					@raycaster.setFromCamera( @mouse, @camera )
					sphereMousePos = @raycaster.ray.intersectSphere(referenceSphere)
					
				@globeCamera.copy(@camera)
				@globeCamera.range += (@targetRange - @globeCamera.range) * @zoomAnimationSpeed
				@globeCamera.apply(@camera)
				
				if @zoomToPointer
					@camera.updateMatrixWorld();
					@raycaster.setFromCamera( @mouse, @camera )
					zoomedSphereMousePos = @raycaster.ray.intersectSphere(referenceSphere)
					#calcualte rotation
					q = new THREE.Quaternion()
					from = sphereMousePos
					to = zoomedSphereMousePos
					if from and to
						from.normalize()
						to.normalize()
						q.setFromUnitVectors(to,from)
		
						@rotate(q)
						
				@rangeUpdate()
				@updateAltitude()

			else
				@targetRange = undefined

	updateMousePositionOnGlobe: (domElement) ->
		# update the picking ray with the camera and mouse position
		@raycaster.setFromCamera( @mouse, @camera )

		# note: currently we do sphere intersection to improve performance.
		#
		# calculate objects intersecting the picking ray
		#intersects = @raycaster.intersectObjects( @globe.rhombusRoot.children )
		#if (intersects.length)
		# 	@mouseOnGlobe = intersects[0].point
		#
		# 	#position = wgs84.latLonToXyz(wgs84.xyzToLatLon(@mouseOnGlobe))
		# 	#@mouseSphere.position.set(position.x,position.y,position.z)
		# else
		# 	@mouseOnGlobe = undefined
		#
		@mouseOnGlobe = @raycaster.ray.intersectSphere(referenceSphere)

		# update mouse click trace if needed
		if @mouseButtonDown and @mouseClickTrace
			@mouseClickTrace.updateLastLocation(@mouseOnElement,@mouseOnGlobe)			

		# call icon tree intersection detection for icon hover states
		@raycastIconTrees() unless @dragging 

		# only set it on change to avoid costly DOM operation
		@setMouseCursor(domElement)

		if (@mouseOnGlobe)
			angle2pixelFactor = Math.tan(angles.radians(@camera.fov / 2 )) / (domElement.height / 2)
			pixelSize = @camera.position.distanceTo(@mouseOnGlobe) * angle2pixelFactor
			@mouseOnGlobeRadius = pixelSize / 2
		else
			@mouseOnGlobeRadius = undefined

		if @navigationType == 'cameraTargetMove'
			@_changeCameraTargetBasedOnMouse(domElement)
		if @navigationType == 'cameraTargetLock'
			@_changeCameraDirectionBasedOnMouse()

		@_animateRangeChange()
		@globeCamera.copy(@camera)

	_changeCameraTargetBasedOnMouse: (domElement) ->
		if @dragging && @mouseOnGlobe 
			if !@mouseDownOnGlobe
				@mouseDownOnGlobe = @mouseOnGlobe.clone()
			else if @moved && !@mouseDownOnGlobe.equals(@mouseOnGlobe)

				#store original distance between the two points
				originalDistance = @mouseDownOnGlobe.distanceTo(@mouseOnGlobe)

				#and a back camera in case we need to restore - same rotation at the pools
				backupPosition = @camera.position.clone()
				backupRotation = @camera.rotation.clone()
				oldCameraQ = @camera.quaternion.clone().inverse();

				screenSize = new THREE.Vector3(domElement.width * 0.5, domElement.height * 0.5, 0.0)

				#calculate drag points in the screen space
				fromScreen = @mouseDownOnGlobe.clone().project(@camera).multiply(screenSize)
				toScreen = @mouseOnGlobe.clone().project(@camera).multiply(screenSize)
				toScreen.sub(fromScreen).multiplyScalar(@dragSpeed).add(fromScreen)
				@raycaster.setFromCamera( toScreen.clone().divide(screenSize), @camera )

				#calcualte rotation
				q = new THREE.Quaternion()
				from = @mouseDownOnGlobe.clone()
				to = @raycaster.ray.intersectSphere(referenceSphere)

				from.normalize()
				to.normalize()
				q.setFromUnitVectors(to,from)

				@lastRotation = q
				@rotate(q)

				#check if the new camera rotation didn't change the screen to much

				if (toScreen.distanceTo(fromScreen) < 2.0)
					#revert the camera to its orignal location
					@camera.position.copy(backupPosition)
					@camera.rotation.copy(backupRotation)
					@moved = false
					@lastRotation = null

				@mouseSphere.position.copy(@mouseDownOnGlobe)
				@emitCameraMoveEvent()

		else if @lastRotation
			@mouseDownOnGlobe = undefined
			@lastRotation.slerp(identityQuaternion, @dragStopSpeed)
			@rotate(@lastRotation)

			#project two points to the screen, and check how much is changed
			screenSize = new THREE.Vector3(domElement.width * 0.5, domElement.height * 0.5, 0.0)
			orig = @camera.position.clone().project(@camera).multiply(screenSize)
			rotated = @camera.position.clone().applyQuaternion(@lastRotation).project(@camera).multiply(screenSize)
			if identityQuaternion.equals(@lastRotation) or Math.abs(1.0 - Math.abs(identityQuaternion.dot(@lastRotation))) < Number.EPSILON
				@lastRotation = null

	_changeCameraDirectionBasedOnMouse: () ->
		if @lastMouseDelta.x || @lastMouseDelta.y
			@lastMouseDelta.x *= @dragSpeed
			@lastMouseDelta.y *= @dragSpeed
			@lastMouseDelta.x = 0 if Math.abs(@lastMouseDelta.x) < 1
			@lastMouseDelta.y = 0 if Math.abs(@lastMouseDelta.y) < 1

			modifiedCamera = new GlobeCamera(@camera);

			modifiedCamera.heading += @lastMouseDelta.x / 100
			modifiedCamera.tilt -= @lastMouseDelta.y / 100

			modifiedCamera.tilt = clamp(0,80,modifiedCamera.tilt)
			modifiedCamera.apply(@camera)

			@emitCameraMoveEvent()

	rotate: (relativeQuaternion) ->
		#rotate camera
		camDirection = new THREE.Vector3(0,0,-1).applyQuaternion(@camera.quaternion);
		camDirection.applyQuaternion(relativeQuaternion);
		@camera.position.applyQuaternion(relativeQuaternion)

		#this cause the up direction to change (aka - heading)
		#not change the up would look the "heading"
		@camera.up.applyQuaternion(relativeQuaternion)

		@camera.lookAt(@camera.position.clone().add(camDirection))
		#@updateAltitude()


	raycastIconTrees: ->
		return if @animating # no need to do hover check while camera is animating

		globe = @globeCanvas.scene.globe
		intersectArray = globe.throttledIconHoverCheck(this)

		if globe.iconHover
			@mouseCursor = 'pointer' if @mouseCursor == 'default'
		else
			@mouseCursor = 'default' if @mouseCursor == 'pointer'
				
	storeCameraMatrix: ->
		@storedCameraMatrix.copy(@camera.matrix)

	isCameraMatrixChanged: ->
		return !@storedCameraMatrix.equals(@camera.matrix)

	raycast: (domElement, x, y) ->

		normalizedPosition =
			x :   ( (x - domElement.offsetLeft) / domElement.width ) * 2 - 1
			y : - ( (y - domElement.offsetTop) / domElement.height ) * 2 + 1

		@raycaster.setFromCamera( normalizedPosition, @camera )

		intersects = @raycaster.intersectObjects( @globeCanvas.scene.globe.rhombusRoot.children )

		return intersects[0].point if intersects.length
		return null

	getMouseIntersections: () ->
		@raycaster.setFromCamera( @mouse, @camera )
		return @raycaster.intersectObjects( @globeCanvas.scene.globe.rhombusRoot.children )


	getMouseGeometryOnGlobe: ->
		return null unless @mouseOnGlobe

		latlon = wgs84.xyzToLatLon(@mouseOnGlobe)

		# if we are hovering over an icon we want to use the icon position instead of the
		# mouseOnGlobe position to ensure we hit the right target with our circle
		if @globeCanvas.globe.iconHover
			node = @globeCanvas.globe.getIconHoverNode()
			latlon = node.getHoverData(@globeCanvas.globe.iconHover.vertexIndex).latlon

		circleGeometry =
			type: "Circle"
			coordinates: [latlon.lon,latlon.lat]
			radius: @mouseOnGlobeRadius * 10.0  # magic number to get icons -- 5.0 failed in many cases

		return circleGeometry

	# return metadata about the click -- if it is an icon, we return the hoverData
	# otherwise we build a new dataset
	getMouseClickMetadata: ->
		data = {}

		if @globeCanvas.globe.iconHover
			node = @globeCanvas.globe.getIconHoverNode()
			data = node.getHoverData(@globeCanvas.globe.iconHover.vertexIndex)

		return data

	#Events
	onStart:() ->
		@startTime = new Date().getTime()
		@copyCameraPosition = true

	onMouseMove: (domElement, event) ->
		offset = $(domElement).offset()
		@mouseOnElement = { left: (event.pageX - offset.left), top: (event.pageY - offset.top) } 
	
		@mouse.x =   ( @mouseOnElement.left / domElement.width ) * 2 - 1
		@mouse.y = - ( @mouseOnElement.top / domElement.height ) * 2 + 1
		@moved = true
		if @mouseButtonDown
			if !event.which
				#it seems like we missed the mouse up event.
				@onMouseUp(domElement,event)
				return
				
			if !@dragging
				@emit 'dragstart'
				@dragging = true
				if @mouseCursor == 'grab'
					@mouseCursor = 'grabbing'

			if @mouseClickTrace
				@mouseClickTrace.addMouseLocation('mousemove', @mouseOnElement)

				delta = @mouseClickTrace.lastPosDelta()
				@lastMouseDelta.x += delta.x
				@lastMouseDelta.y += delta.y

		return false

	onMouseDown: (domElement, event) ->
		# console.log("MOUSE DOWN ", event.which, @mouse.x, @mouse.y)
		@mouseDownOnGlobe = @mouseOnGlobe.clone() if @mouseOnGlobe
		@mouseButtonDown = true
		
		switch event.which
			when 2 then @navigationType = 'scroll'
			when 3 then @navigationType = 'cameraTargetLock'
			else
				#copy google earth navigation UX
				if event.shiftKey
					this.navigationType = 'cameraTargetLock'
				else
					@navigationType = 'cameraTargetMove'
					if @mouseCursor == 'default'
						@mouseCursor = 'grab'
						@setMouseCursor(domElement)

		@mouseClickTrace = new MouseClickTrace(@mouseOnElement, @mouseOnGlobe)
		@lastMouseDelta = { x:0, y:0 }

		return false

	onMouseUp: (domElement, event) ->
		@emit 'dragend' if @dragging
		@dragging = false
		@mouseButtonDown = false
		@mouseCursor = 'default'
		@setMouseCursor(domElement)

		@mouseClickTrace.addMouseLocation('mouseup', @mouseOnElement,@mouseOnGlobe) if @mouseClickTrace

		@updateAltitude()

	onMouseWheel: (wheelDelta) ->
		factor = 0.85
		
		if wheelDelta < 0
			factor = -factor
			
		@zoom(factor)
	
	zoom: (factor) ->
		@globeCamera.copy(@camera)
		@emitCameraMoveEvent()

		targetRange = @targetRange
		if !targetRange
			targetRange = @globeCamera.range

		if factor > 0
			targetRange *= factor
		else
			targetRange /= -factor

		# keep target range within defined limits
		targetRange = clamp(@rangeLimitMin, @rangeLimitMax, targetRange);

		@targetRange = targetRange


	# provide the same interface as setCamera in interface
	# we may want to remove this or refactor but right now it's needed
	# on the landing page where there is no interface loaded
	animateCamera: (camera, options, callback) ->
		self = this
		globeCamera = @camera
		currentCameraAnimation = null
		currentCamera = new GlobeCamera(globeCamera)
		targetCamera = new GlobeCamera(camera)

		#verify the target camera is valid according to range limit
		targetCamera.range = clamp(@rangeLimitMin, @rangeLimitMax, targetCamera.range);

		options or= {}

		defaults = 
			duration: 1000,
			easing: Cubic.easeOut
		
		_.extend defaults, options

		if targetCamera.heading - currentCamera.heading > 180
			targetCamera.heading -= 360

		if targetCamera.heading - currentCamera.heading < -180
			targetCamera.heading += 360

		if targetCamera.longitude - currentCamera.longitude > 180
			targetCamera.longitude -= 360

		if targetCamera.longitude - currentCamera.longitude < -180
			targetCamera.longitude += 360

		target = wgs84.latLonToXyz({lat:targetCamera.latitude,lon:targetCamera.longitude})

		distanceInMeters = target.angleTo(globeCamera.position) * wgs84.earthRadius

		@animating = true
		@emitCameraMoveEvent()

		if ( distanceInMeters / Math.max(targetCamera.range,currentCamera.range) > 2 )
			currentCamera.rangeAnim = 0

			startRange = currentCamera.range
			middleRange = distanceInMeters / 2
			endRange = targetCamera.range

			currentCameraAnimation = window.TweenMax.to(currentCamera, options.duration / 1000, {
				rangeAnim: 1
				latitude: targetCamera.latitude,
				longitude: targetCamera.longitude,
				altitude: targetCamera.altitude,
				heading: targetCamera.heading,
				range: targetCamera.range,
				tilt: targetCamera.tilt,
				easing: options.easing,
				onComplete: ->
					self.animating = false
					self.updateAltitude()
					self.globeCamera.copy(currentCamera)
					self.rangeUpdate()
					callback() if callback
				onUpdate: () ->
					if currentCamera.rangeAnim < 0.5
						d = Math.sin(Math.PI * currentCamera.rangeAnim)
						currentCamera.range = startRange * (1-d) + middleRange * d
					else
						d = Math.sin(Math.PI * currentCamera.rangeAnim)
						currentCamera.range = endRange * (1-d) + middleRange * d

					currentCamera.apply(globeCamera)
				})
		else
			currentCameraAnimation = window.TweenMax.to(currentCamera, options.duration / 1000, {
				latitude: targetCamera.latitude,
				longitude: targetCamera.longitude,
				altitude: targetCamera.altitude,
				heading: targetCamera.heading,
				range: targetCamera.range,
				tilt: targetCamera.tilt,
				easing: options.easing,
				onComplete: ->
					self.animating = false
					self.updateAltitude()
					self.globeCamera.copy(currentCamera)
					self.rangeUpdate()
					callback() if callback
				onUpdate: ()-> currentCamera.apply(globeCamera)
				})


		return {
			currentCamera: currentCamera
			currentCameraAnimation: currentCameraAnimation
		}


	###
		Allow for the earth rotation spin which can be interrupted with
		mouse gestures. 
	###
	updateGlobeSpin: ->
		unless @dragging or @animating
			@globeCamera.copy(@camera)
			@globeCamera.longitude -= (@globeSpinAmount * @rangeRatio)
			@globeCamera.apply(@camera)

	###
		Instead of a hard stop, animate down to zero and then disable
	###
	stopGlobeSpin: ->
		self = this

		@globeCanvas.config.enableGlobeSpin = false # make sure this isn't set

		originalSpeed = @globeSpinAmount

		TweenMax.to(this, 2.0, {
			globeSpinAmount: 0, 
			ease: Quad.easeOut,
			onComplete: -> 
				self.globeSpinAmount = originalSpeed
				self.globeSpinEnabled = false
			})

	startGlobeSpin: ->
		@globeSpinEnabled = true

		originalSpeed = @globeSpinAmount

		TweenMax.fromTo(this, 2.0, {globeSpinAmount: 0}, {globeSpinAmount: originalSpeed, ease: Quad.easeOut})

module.exports = {
	CameraController: CameraController
}