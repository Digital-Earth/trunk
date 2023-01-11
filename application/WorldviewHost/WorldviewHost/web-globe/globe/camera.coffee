THREE = require 'three'
angles = require '../core/angles'
wgs84 = require '../core/wgs84'

globeSphere = new THREE.Sphere(new THREE.Vector3(0,0,0),wgs84.earthRadius)
unitVectors =
	x: new THREE.Vector3(1,0,0)
	y: new THREE.Vector3(0,1,0)
	z: new THREE.Vector3(0,0,1)
	negZ: new THREE.Vector3(0,0,-1)

class GlobeCamera
	constructor: (camera)->
		@tilt = 0
		@heading = 0
		@latitude = 0
		@longitude = 0
		@altitude = 0
		@range = 10*1000*1000

		if camera
			@copy(camera)

	copy: (camera) ->
		if 'position' of camera
			#copy from THREE.Camera
			camDirection = unitVectors.negZ.clone()
			camDirection.applyQuaternion(camera.quaternion)

			if camera.altitude
				@altitude = camera.altitude
				globeSphere.radius = wgs84.earthRadius + @altitude
			else
				globeSphere.radius = wgs84.earthRadius

			#TODO This should work, but it don't, need to investigate this
			#globeSphere.radius = wgs84.earthRadius + @altitude

			ray = new THREE.Ray(camera.position,camDirection)
			target = ray.intersectSphere(globeSphere)

			@tilt = 180-angles.degrees(camDirection.angleTo(target))

			heading = new THREE.Vector3()
			heading.crossVectors(camera.up,target)
			trueNorth = new THREE.Vector3()
			trueNorth.crossVectors(unitVectors.y,target)

			@heading = angles.degrees(heading.angleTo(trueNorth))
			if (heading.y > 0)
				@heading = 360 - @heading

			latlon = wgs84.xyzToLatLon(target)

			@latitude = latlon.lat
			@longitude = latlon.lon

			@range = target.distanceTo(camera.position)



		if 'Heading' of camera
			#copy from Pyxis Camera model
			@tilt = camera.Tilt
			@heading = camera.Heading
			@latitude = camera.Latitude
			@longitude = camera.Longitude
			@altitude = camera.Altitude
			@range = camera.Range

		if 'heading' of camera
			#copy GlobeCamera object
			@tilt = camera.tilt
			@heading = camera.heading
			@latitude = camera.latitude
			@longitude = camera.longitude
			@altitude = camera.altitude
			@range = camera.range


	apply: (camera)->

		if 'position' of camera
			#add range
			position = new THREE.Vector3(@range,0,0)

			#tilt
			position.applyAxisAngle(unitVectors.z,-angles.radians(@tilt))

			#TODO Remove adding of the altitude into range
			#add altitude and earth radius
			position.add(new THREE.Vector3(wgs84.earthRadius + @altitude,0,0))

			latlon = {lat:@latitude,lon:@longitude}
			target = wgs84.latLonToXyz(latlon)
			target.multiplyScalar((@altitude+wgs84.earthRadius) / wgs84.earthRadius)
			radiansLatLon = wgs84.toRadiansGeocentricLatLon(latlon)

			rotation = new THREE.Euler(-angles.radians(@heading),radiansLatLon.lon,radiansLatLon.lat,'YZX')
			position.applyEuler(rotation)

			camera.position.copy(position)

			up = unitVectors.y.clone()
			up.applyEuler(rotation)
			camera.up.copy(up)
			camera.lookAt(target)

			camera.altitude = @altitude

		if 'Heading' of camera
			#apply to pyxis camera
			camera.Tilt = @tilt
			camera.Heading = @heading
			camera.Latitude = @latitude
			camera.Longitude = @longitude
			camera.Altitude = @altitude
			camera.Range = @range

		if 'heading' of camera
			#apply to globe camera
			camera.tilt = @tilt
			camera.heading = @heading
			camera.latitude = @latitude
			camera.longitude = @longitude
			camera.altitude = @altitude
			camera.range = @range

		return camera

	asPyxisCamera: () ->
		return {
			Tilt: @tilt
			Heading: @heading
			Latitude: @latitude
			Longitude: @longitude
			Altitude: @altitude
			Range: @range
		}

window.GlobeCamera = GlobeCamera






class FocusCamera extends THREE.PerspectiveCamera
	constructor: (@canvas, fov, aspect, near, far, @left, @bottom, @width, @height, @transform) ->  #transform is TEMPORARY
		@canvas.focusCameras.push(this)
		super(fov,aspect,near,far)
		console.log(this)

	render: ->
		@canvas.camera.updateMatrix()
		@matrix=@canvas.camera.matrix

		@applyMatrix(@transform)
		width = Math.floor(@canvas.width*@width)
		height = Math.floor(@canvas.height*@height)
		bottom = Math.floor(@canvas.height*@bottom)
		left = Math.floor(@canvas.width*@left)
		@canvas.renderer.setViewport(left,bottom,width,height)
		@canvas.renderer.setScissor(left,bottom,width,height)
		@canvas.renderer.enableScissorTest(true)
		@canvas.root.tree.updateWithCamera(this)
		@canvas.renderer.render(@canvas.scene, this)
		@canvas.renderer.setViewport(0,0,@canvas.width,@canvas.height)
		@canvas.renderer.setScissor(0,0,@canvas.width,@canvas.height)
		@canvas.renderer.enableScissorTest(false)





window.FocusCamera = FocusCamera



module.exports =
	GlobeCamera: GlobeCamera
	FocusCamera: FocusCamera
