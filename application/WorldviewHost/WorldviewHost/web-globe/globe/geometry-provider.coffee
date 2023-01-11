###
	This is a coffeescript file that compiles-to-javascript.

	GeometryProvider provides logic for loading and processing geometry information
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'

DataLoader = require('../utilities/loader').DataLoader

class GeometryProvider
	constructor: (@backgroundTasks) ->
		@sharedBuffers = {}
		@sharedBuffersSkirt = {}

		@keys = {}

		@loaderOptions = {
			maxRequests: 3,
			manualSort: false
		}

		@dataLoader = new DataLoader(@loaderOptions)

	#
	# 	* returns current loading status in terms of pending requests and requests in progress
	getLoadingStatus: () ->
		result =
			requestsPending: @dataLoader.requestsCount()
			requestsInProgress: @dataLoader.requestsInProgress()

		return result

	#
	# * returns THREE.BufferGeometry for specified key and options
	# *
	# * options.size - field name to use
	# * options.skirt - max and minimum of the field [min,max]
	# * options.tessellate - will request only rhombues that intersects the bounding sphere
	# * options.networkRequest - query a mesh data from the server
	# *
	# * callback - called after geometry ready (maybe called inside this function if geometry already ready)
	getData: (key, options, callback) ->
		#options.tessellate = true
		self = this

		if key of @keys
			#has cached request
			cacheRecord = @keys[key]
			if (cacheRecord.netGeometry)
				#used cachded result
				@_processGeometry(cacheRecord.netGeometry, key, options, callback)
			else
				#join network request
				cacheRecord.pendingRequests.push {
					options: options,
					callback: callback
				}
		else
			#start new request
			@keys[key] = cacheRecord = {
				key : key,
				retries: 0,
				pendingRequests: [],
				generatedGeometryList : []
			}

			cacheRecord.pendingRequests.push {
				options: options,
				callback: callback
			}
			
			if options.networkRequest
				self._sendNetworkRequest(key,options)
			else
				self._generateGeometry(key,options)

	_sendNetworkRequest: (key,options) ->
		self = this
		cacheRecord = @keys[key];

		url = "#{window.gwcHost.get(key)}/api/v1/rhombus/geometry/?key=#{key}&size=#{options.size}"

		if (requestsLog != undefined)
			window.requestsLog.push url

		@dataLoader.load {
			url: url,
			success: (json) ->
				#we just need the vertices
				netGeometry = {
					vertices: json.vertices
				}
				cacheRecord.netGeometry = netGeometry

				for request in cacheRecord.pendingRequests
					self._processGeometry(netGeometry, key, request.options, request.callback)

				cacheRecord.pendingRequests = []

				#dispose entry if possible
				if cacheRecord.generatedGeometryList.length == 0 && cacheRecord.pendingRequests.length == 0
					delete self.keys[key]

			error: () ->
				#search if we have any pending request
				for request in cacheRecord.pendingRequests
					if request.options.enabled()

						cacheRecord.retries++
						if (cacheRecord.retries < 3)
							console.log "fail to load geometry, retrying ", key
							self._sendNetworkRequest(key,options)
						else
							console.log "fail to load geometry", key

						return

				#found no enabled request
				cacheRecord.pendingRequests = []

				#dispose entry if possible
				if cacheRecord.generatedGeometryList.length == 0 && cacheRecord.pendingRequests.length == 0
					delete self.keys[key]

			enabled: () ->
				for request in cacheRecord.pendingRequests
					if request.options.enabled()
						#console.log "requesting", key, request.options.priority()
						return true

				return false

			priority: () ->
				maxPriority = 0

				for request in cacheRecord.pendingRequests
					priority = request.options.priority()
					maxPriority = priority if priority > maxPriority

				return maxPriority
		}

	releaseData: (key, geometry) ->
		self = this
		cacheRecord = @keys[key]

		#remove all shared geometries before disposing the geometry
		#this will ensure that all shared buffer are not get deleted at the WebGL scope
		delete geometry.attributes["normal"]
		delete geometry.attributes["uv"]
		delete geometry.attributes["index"]

		if cacheRecord
			idx = cacheRecord.generatedGeometryList.indexOf(geometry)
			if idx != -1
				cacheRecord.generatedGeometryList.splice(idx, 1)
				if cacheRecord.generatedGeometryList.length == 0 && cacheRecord.pendingRequests.length == 0
					delete @keys[key]
				return
		throw "GeometryProvider::releaseGeometry: Can't find geometry";

	_processGeometry: (netGeometry, key, options, callback) ->
		geometry = @convertThreeJSGeometryToBufferGeometry(netGeometry, options)
		@keys[key].generatedGeometryList.push(geometry)
		callback(geometry)
		
	_generateGeometry: (key, options) ->
		size = 10
		size = options.size if options.size

		vertices = new Float32Array(size * size * 3)
		vertexIndex = 0
		
		uvTransform = new THREE.Vector4(1.0, 1.0, 0.0, 0.0)
		topRhombus = key[0]
		parsedKey = key.slice(1)
		uvTransform = PYXIS.snyder.RhombusMath.rhombusKeyToUvTransform(parsedKey);

		for vIndex in [0...size]
			for uIndex in [0...size]
				latLon = PYXIS.snyder.RhombusMath.rhombusUVToGeocentricLatLon(topRhombus, uvTransform.u(uIndex / (size-1)), uvTransform.v(vIndex / (size-1)))
				xyz = PYXIS.snyder.SphereMath.llxyz(latLon)

				vertices[vertexIndex * 3 + 0] = xyz.x
				vertices[vertexIndex * 3 + 1] = xyz.y
				vertices[vertexIndex * 3 + 2] = xyz.z

				vertexIndex++


		generatedNetGeometry = 
		{
			vertices: vertices
		}
		cacheRecord = @keys[key]
		cacheRecord.netGeometry = generatedNetGeometry
		
		#options.size = copySize
		#options.tessellate = true
		#options.generated = true
		
		for request in cacheRecord.pendingRequests
			@_processGeometry(cacheRecord.netGeometry, key, request.options, request.callback)

		cacheRecord.pendingRequests = []

		#dispose entry if possible
		if cacheRecord.generatedGeometryList.length == 0 && cacheRecord.pendingRequests.length == 0
			delete self.keys[key]
		
	update: () ->

	###*
	 * Creates a buffer geometry from a give ThreeJSGeometry.
	 *
	 * We create a normal buffer geometry which is very efficent in terms of memory.
	 * The geometry provide 5 BufferAtrributes:
	 *  1) position
	 *  2) normals, which is a copy of position
	 *  3) uv (shared with all same size rhombus)
	 *  4) index (shared with al same size rhombus)
	 *
	 * To make thing works with THREE webgl resource management. do the following:
	 *  a) normal buffer is a copy of position
	 *  b) all geometries of same size sahre uv,index buffers
	 *  c) before disposing the geomerty we "remove" the normal,uv and index attributes
	 *     from the geometry scope. This allow us to skip the part of removing the buffer
	 *     from WebGL in the geomerty dispose callback.
	 *  d) because position is not been removed, it will get disposed. However, the copy
	 *     normal buffer is not disposed as it was removed from geometry context already.
	 *
	 * @param  {Three.Geometry} geometry 	geomerty object
	 * @param  {object} options  			options
	 * @return {Three.BuferGeometry}        created buffer Geometry
	###
	convertThreeJSGeometryToBufferGeometry: (geometry, options) ->
		bufferGeometry = new THREE.BufferGeometry();

		originalSize = options.size
		if options.tessellate
			options.size = (options.size - 1) * 3 + 1

		verticesCount = options.size * options.size

		if options.skirt
			verticesCount += @getSkirtVerticesCount(options.size)

		positions = new THREE.BufferAttribute( new Float32Array( verticesCount * 3 ), 3 )

		#if not tesselate than just load vertices
		if !options.tessellate
			@copyVector3sArray(positions.array, geometry.vertices)
		else
			tempOriginalPositions = new Float32Array( originalSize * originalSize * 3 )
			@copyVector3sArray(tempOriginalPositions, geometry.vertices)
			@tessellateBufferGeometry(originalSize, tempOriginalPositions, 0, originalSize, positions.array, options.size)

		if options.skirt
			@fillSkirtData(positions.array, options.size)

		sharedBuffers = @getSharedBuffers(options.size, options.skirt)

		bufferGeometry.addAttribute('position', positions )
		bufferGeometry.addAttribute('normal', positions )
		bufferGeometry.addAttribute('uv', sharedBuffers.uv )
		bufferGeometry.setIndex(sharedBuffers.index)

		bufferGeometry.computeBoundingSphere()
		bufferGeometry.options = options

		return bufferGeometry

	getSharedBuffers: (meshSize, withSkirt) ->
		arrayName = "sharedBuffers"
		if withSkirt
			arrayName = "sharedBuffersSkirt"

		if meshSize of this[arrayName]
				return this[arrayName][meshSize]

		this[arrayName][meshSize] = @genSharedBuffers(meshSize, withSkirt)
		return this[arrayName][meshSize]

	genSharedBuffers: (meshSize, withSkirt) ->
		verticesCount = meshSize * meshSize
		faceCount = ( meshSize - 1 ) * ( meshSize - 1 ) * 2
		if withSkirt
			verticesCount = verticesCount + ( meshSize - 1 ) * 4 + 1
			faceCount = faceCount + ( meshSize - 1 ) * 4 * 2

		buffers = {}
		buffers.uv = new THREE.BufferAttribute( new Float32Array( verticesCount * 2 ), 2 )
		buffers.index = new THREE.BufferAttribute( new Uint32Array( faceCount * 3 ), 1 )

		#gen UV
		buffer = buffers.uv.array
		for i in [0...meshSize]
			for j in [0...meshSize]
				idx = j + i * meshSize
				buffer[idx * 2 + 0] = j / (meshSize-1)
				buffer[idx * 2 + 1] = 1.0 - i / (meshSize-1)
		#gen Indices
		buffer = buffers.index.array
		offset = 0
		for i in [0...(meshSize-1)]
			for j in [0...(meshSize-1)]
				buffer[offset+0] = j*meshSize + i
				buffer[offset+1] = j*meshSize + i + 1
				buffer[offset+2] = (j+1)*meshSize + i

				buffer[offset+3] = buffer[offset+1]
				buffer[offset+4] = (j+1)*meshSize + i + 1
				buffer[offset+5] = (j+1)*meshSize + i

				offset = offset + 6

		if withSkirt
			buffer = buffers.uv.array
			#gen additional UV coordinates
			vPos = meshSize * meshSize * 2
			for i in [0...meshSize] by 1
				idx = i
				buffer[vPos + 0] = buffer[ idx*2 + 0 ]
				buffer[vPos + 1] = buffer[ idx*2 + 1 ]
				vPos = vPos + 2
			#copy right line
			offset = meshSize - 1
			for i in [1...meshSize-1] by 1
				idx = offset + i * meshSize
				buffer[vPos + 0] = buffer[ idx*2 + 0 ]
				buffer[vPos + 1] = buffer[ idx*2 + 1 ]
				vPos = vPos + 2
			#copy bottom line
			offset = meshSize * (meshSize - 1)
			for i in [0...meshSize] by 1
				idx = offset + (meshSize - 1 - i)
				buffer[vPos + 0] = buffer[ idx*2 + 0 ]
				buffer[vPos + 1] = buffer[ idx*2 + 1 ]
				vPos = vPos + 2

			#copy left line
			offset = meshSize
			for i in [1...meshSize-1] by 1
				idx = offset + (meshSize - 1 - i) * meshSize
				buffer[vPos + 0] = buffer[ idx*2 + 0 ]
				buffer[vPos + 1] = buffer[ idx*2 + 1 ]
				vPos = vPos + 2
			#make additional vertex at end for "nice" math
			buffer[vPos + 0] = buffer[ 0 ]
			buffer[vPos + 1] = buffer[ 1 ]

			#gen additional faces
			buffer = buffers.index.array
			vPos = ( meshSize - 1 ) * ( meshSize - 1 ) * 2 * 3
			emitSkirtFaces = (meshSize) ->
				for i in [0...meshSize-1] by 1
					a = startSkirtVertex + i
					b = startSkirtVertex + i + 1
					c = vertexStart + i * vertexStep
					d = vertexStart + (i + 1) * vertexStep

					if not inverse_grid
						buffer[vPos + 0] = a
						buffer[vPos + 1] = b
						buffer[vPos + 2] = c
						buffer[vPos + 3] = c
						buffer[vPos + 4] = b
						buffer[vPos + 5] = d
					else
						buffer[vPos + 0] = c
						buffer[vPos + 1] = a
						buffer[vPos + 2] = d
						buffer[vPos + 3] = d
						buffer[vPos + 4] = a
						buffer[vPos + 5] = b
					vPos = vPos + 6

			#emit skirt faces

			#top skirt
			startSkirtVertex = meshSize * meshSize

			vertexStart = 0
			vertexStep = 1
			inverse_grid = true

			emitSkirtFaces(meshSize)

			#right skirt
			startSkirtVertex = meshSize * meshSize + meshSize - 1
			vertexStart = meshSize - 1
			vertexStep = meshSize
			inverse_grid = false
			emitSkirtFaces(meshSize)

			#bottom skirt
			startSkirtVertex = meshSize * meshSize + (meshSize - 1) * 2
			vertexStart = meshSize * meshSize - 1
			vertexStep = -1
			inverse_grid = true
			emitSkirtFaces(meshSize)

			#left skirt
			startSkirtVertex = meshSize * meshSize + (meshSize - 1) * 3
			vertexStart = meshSize * (meshSize - 1)
			vertexStep = meshSize * -1
			inverse_grid = false
			emitSkirtFaces(meshSize)

		return buffers

	getSkirtVerticesCount: (meshSize) ->
		return ( meshSize - 1 ) * 4 + 1

	fillSkirtData: (array, meshSize) ->
		idx = meshSize * meshSize * 3

		cloneVertex = (i) ->
			array[idx+0] = array[i * 3 + 0]
			array[idx+1] = array[i * 3 + 1]
			array[idx+2] = array[i * 3 + 2]
			idx+=3

		for i in [0...meshSize] by 1
			cloneVertex(i)

		#copy right line
		offset = meshSize - 1
		for i in [1...meshSize-1] by 1
			cloneVertex( offset + i * meshSize )

		#copy bottom line
		offset = meshSize * (meshSize - 1)
		for i in [0...meshSize] by 1
			cloneVertex( offset + (meshSize - 1 - i) )

		#copy left line
		offset = meshSize
		for i in [1...meshSize-1] by 1
			cloneVertex( offset + (meshSize - 1 - i) * meshSize )

		#make additional vertex at end for "nice" math
		cloneVertex(0)

	copyVector3sArray: ( array, vectors ) ->
		for i in [0...vectors.length] by 1
			array[i] = vectors[i]

		return array

	tessellateBufferGeometry: (sourceSize, sourceData, sourceOffset, sourcePitch, destData, destPitch) ->

		#4 original rhombus points
		sourceQuad = [new THREE.Vector3,new THREE.Vector3,new THREE.Vector3,new THREE.Vector3];
		twoMidPoints = [new THREE.Vector3,new THREE.Vector3]
		tmpVector = new THREE.Vector3

		loadVector3 = (array, destVector, x, y, meshSize) ->
			destVector.x = array[(y*meshSize+x)*3 + 0];
			destVector.y = array[(y*meshSize+x)*3 + 1];
			destVector.z = array[(y*meshSize+x)*3 + 2];

		storeVector3 = (array, srcVector, x, y, meshSize) ->
			array[(y * meshSize + x)*3 + 0] = srcVector.x
			array[(y * meshSize + x)*3 + 1] = srcVector.y
			array[(y * meshSize + x)*3 + 2] = srcVector.z

		calculateMidPoints = (arrayOfPoints, vector0, vector1) ->
			tmpVector.copy(vector1).sub(vector0)
			arrayOfPoints[0].copy(tmpVector).multiplyScalar(1.0 / 3.0).add(vector0)
			arrayOfPoints[1].copy(tmpVector).multiplyScalar(2.0 / 3.0).add(vector0)

		for y in [0...sourceSize-1]
			for x in [0...sourceSize-1]
				loadVector3(sourceData, sourceQuad[0], sourceOffset + x+0, y+0, sourcePitch)
				loadVector3(sourceData, sourceQuad[1], sourceOffset + x+1, y+0, sourcePitch)
				loadVector3(sourceData, sourceQuad[2], sourceOffset + x+1, y+1, sourcePitch)
				loadVector3(sourceData, sourceQuad[3], sourceOffset + x+0, y+1, sourcePitch)

				#move old quad vertices to the new mesh
				storeVector3(destData, sourceQuad[0], (x+0)*3, (y+0)*3, destPitch)
				storeVector3(destData, sourceQuad[1], (x+1)*3, (y+0)*3, destPitch)
				storeVector3(destData, sourceQuad[2], (x+1)*3, (y+1)*3, destPitch)
				storeVector3(destData, sourceQuad[3], (x+0)*3, (y+1)*3, destPitch)

				#calculate 4 midpoint pairs on quad edges
				calculateMidPoints(twoMidPoints, sourceQuad[0], sourceQuad[1])
				storeVector3(destData, twoMidPoints[0], x*3+1, y*3, destPitch)
				storeVector3(destData, twoMidPoints[1], x*3+2, y*3, destPitch)

				calculateMidPoints(twoMidPoints, sourceQuad[1], sourceQuad[2])
				storeVector3(destData, twoMidPoints[0], (x+1)*3, y*3+1, destPitch)
				storeVector3(destData, twoMidPoints[1], (x+1)*3, y*3+2, destPitch)

				calculateMidPoints(twoMidPoints, sourceQuad[2], sourceQuad[3])
				storeVector3(destData, twoMidPoints[0], x*3+2, (y+1)*3, destPitch)
				storeVector3(destData, twoMidPoints[1], x*3+1, (y+1)*3, destPitch)

				calculateMidPoints(twoMidPoints, sourceQuad[3], sourceQuad[0])
				storeVector3(destData, twoMidPoints[0], x*3, y*3+2, destPitch)
				storeVector3(destData, twoMidPoints[1], x*3, y*3+1, destPitch)

				#pair of diagonal midpoints
				calculateMidPoints(twoMidPoints, sourceQuad[1], sourceQuad[3])
				storeVector3(destData, twoMidPoints[0], x*3+2, y*3+1, destPitch)
				storeVector3(destData, twoMidPoints[1], x*3+1, y*3+2, destPitch)

				#calculate two triangle centers
				tmpVector.copy(sourceQuad[0]).add(sourceQuad[1]).add(sourceQuad[3]).multiplyScalar(1.0 / 3.0).normalize()
				storeVector3(destData, tmpVector, x*3+1, y*3+1, destPitch)
				tmpVector.copy(sourceQuad[1]).add(sourceQuad[2]).add(sourceQuad[3]).multiplyScalar(1.0 / 3.0).normalize()
				storeVector3(destData, tmpVector, x*3+2, y*3+2, destPitch)

				hello_coffee=0


module.exports = {
	GeometryProvider: GeometryProvider
}