###
	This is a coffeescript file that compiles-to-javascript.

	Here are the initial models for the RhombusTree, ported from Mark Sherlock's solution
###


_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
glslify = require 'glslify'
EventEmitter = require 'event-emitter'
Color = require('color')
RhombusHierarchy = require('./hierarchy').RhombusHierarchy
Palette = require('../../core/palette')
shaderFactory = require('../theme/shader-factory')



window.requestsLog = undefined

#uncomment the next line to enable request logging
#window.requestsLog = [];


SCALE = 6371007.0
PI = 3.141592654

# I use a .html extension just to avoid browserify parse
placeholderDataURI = require('../lib/placeholder-base64.html')
placeholderImage = document.createElement( 'img' )
placeholderImage.src = placeholderDataURI
placeholder = new THREE.Texture(placeholderImage, THREE.UVMapping)
placeholder.needsUpdate = true
window.placeholder = placeholder

tempMatrix4 = new THREE.Matrix4()


THREE.ImageUtils.crossOrigin = ''
window.stopAnim = false




class RhombusTree
	constructor: (@root, @theme, @dataProviders, @backgroundTasks, renderLayers) ->
		self = this

		# provide events like 'loaded'
		EventEmitter(self)

		@hierarchy = new RhombusHierarchy()

		@frameCallbacks = []

		console.log "create globe tree"
		console.log this
		@nodes = {}
		@meshes = {}
		@updateList = []
		@updateListToRemove = []

		# to be populated with geosources
		@renderLayers = []
		@previousLayers = []

		@renderLayers = renderLayers if renderLayers?

		@screenSize = new THREE.Vector2(0,0);

		@theme.init()

		@shaderFactory = shaderFactory(self, @dataProviders, self.renderLayers, @theme)

		@visibleKeys = []
		@visibleKeysUpdate = true

		@currentCamera = undefined
		@currentCameraTag = 1
		
		@frozen = false


		@_elevationMultiplier = 1
		@boundSpheresTag = 1

		@meshSize = 10
		@useSkirt = true
		#TODO Link to the texture size
		@rhombusDivideThreshold = 300

		# cleanup unused nodes every 10 seconds
		@throttledTrim = _.throttle((-> self.performTrim()), 10000)

		# spawn nodes
		_.each @hierarchy.children(), (key) -> new RhombusNode(key, self)

	dispose: () ->
		#covert the @nodes to array so we can safly iterate over it
		for node in _.values(@nodes)
			node.dispose()

	loaded: () ->
		#Check that we have atleast 10 top rhombuses
		loaded = _.keys(@meshes).length >= 10
		return loaded and !@dataProviders.geoSource.isGeosourceListIsLoading(@shaderFactory.jit.geoSources)

	updateElevationMultiplier: (elevationMultiplier) ->
		return "TODO: Not working now"
		return if @_elevationMultiplier == elevationMultiplier

		@_elevationMultiplier = elevationMultiplier
		for key, mesh of @meshes
			mesh.material.uniforms.heightMultiplier.value = @_elevationMultiplier

		@updateBoundingSpheres()

	invalidateVisibilityList: ->
		@emit 'invalidateVisibilityList'
		@visibleKeysUpdate = true

	addToUpdateList: (node) ->
		node.updateListId = @updateList.length
		@updateList.push(node)

	removeFromUpdateList: (node) ->
		@updateListToRemove.push node 


	# need a way to update float uniforms
	updatePhongStrength: (phongStrength) ->
		@shaderFactory.uniforms['phongStrength_1'].value = phongStrength

		for key, mesh of @meshes
			mesh.material.uniforms['phongStrength_1'].value = phongStrength

	getElevationMultiplier: ->
		return @_elevationMultiplier


	updateBoundingSpheres: ->
		@boundSpheresTag++
	
	#freeze new requests (geometry and geosource data)
	freeze: () ->
		@frozen = true
		
		
	#unfreeze new requests (geometry and geosource data)
	unfreeze: () ->
		@frozen = false

	###*
	 * get a node with a give key.
	 * A new node will be created and added to the tree if not exists.
	 *
	 * @param  {string} key 	the request node key
	 * @return {RhombusNode}     a rhombus node
	###
	at: (key) ->
		return @nodes[key] if key of @nodes
		return new RhombusNode(key, this)


	###*
	 * returns all contacts of sphere with a camera
	 *
	 * @param {THREE.Camera} camera 	a camera object to test with
	 * @param {number} cameraRadius 	radius for a camera sphere intersection test
	 * @return {list{THREE.Vector3}}	a list with all camera contacts
	###
	findCameraContacts: (camera, cameraRadius) ->
		#TODO maybe better to use recursion, not sure in performance
		contacts = []
		processingQueue = @hierarchy.children().slice();
		checkedNodes = 0
		geometryChecks = 0
		cameraPosition = camera.position
		while processingQueue.length > 0
			rhombusKey = processingQueue.shift()
			node = @at(rhombusKey)
			if(node.isActive() && node.context != undefined)
				checkedNodes++
				radiusSum = node.context.boundingSphere.radius + cameraRadius
				distanceSq = cameraPosition.distanceToSquared(node.context.boundingSphere.center)
				#if we colide the node bounding sphere we need go deeper into the nodes or check a geometry if no childs to check
				if distanceSq <= radiusSum * radiusSum
					if node.isLeafNode() || !node.anyActiveChildren()
						contact = @getClosestContactMeshAndSphere(node, rhombusKey, camera, cameraRadius)
						if contact != undefined
							contacts.push(contact)
						geometryChecks++
					else
						processingQueue = processingQueue.concat(@hierarchy.children(rhombusKey))

		# console.log("Contacts processed:",checkedNodes, " Geometry checks:",geometryChecks, " Found contacts:", contacts.length)
		if(contacts.length > 0)
			contacts.sort((a,b)->
					distA = cameraPosition.distanceToSquared(a)
					distB = cameraPosition.distanceToSquared(b)
					if(distA > distB)
						return 1
					if (distA < distB)
						return -1
					return 0
				)
		return contacts


	###*
	 * returns closest contact node geometry and spehere
	 *
	 * @param {RhombusNode} node 		rhombus node to test with
	 * @param {string} meskKey 			key value for the mesh
	 * @param {THREE.Camera} camera 	a camera object to test with
	 * @param {number} cameraRadius 	radius for a camera sphere intersection test
	 * @return {THREE.Vector3}			closest point in found
	###
	getClosestContactMeshAndSphere: (node, meshKey, camera, cameraRadius) ->
		mesh = @meshes[meshKey]
		return unless mesh

		inverseMeshScale = node.invMatrix.getMaxScaleOnAxis()

		meshCameraPosition = camera.position.clone().applyMatrix4(node.invMatrix)
		meshCameraRadius = cameraRadius * inverseMeshScale
		cameraRadiusSq = meshCameraRadius * meshCameraRadius

		geometry = mesh.geometry
		geometryVertices = geometry.getAttribute('position').array
		geometryIndices = geometry.index.array

		cameraHeight = meshCameraPosition.lengthSq()
		vertexIndex0 = geometryIndices[0] * 3
		vertexIndex1 = geometryIndices[1] * 3
		edgeVertex0 = new THREE.Vector3(geometryVertices[vertexIndex0 + 0], geometryVertices[vertexIndex0 + 1], geometryVertices[vertexIndex0 + 2]);
		edgeVertex1 = new THREE.Vector3(geometryVertices[vertexIndex1 + 0], geometryVertices[vertexIndex1 + 1], geometryVertices[vertexIndex1 + 2]);
		edgeVertex0.multiplyScalar(cameraHeight)
		edgeVertex1.multiplyScalar(cameraHeight)
		edgeLengthSq = edgeVertex0.distanceToSquared(edgeVertex1)

		#Check if the length of the edge more than two radiuses of the camera sphere
		if edgeLengthSq > 1.41421356 * cameraRadiusSq * 0.5
			contact = @_getClosestContactMeshAndSphereRay(node, geometry, meshCameraPosition, cameraRadiusSq)
		else
			#IK: I think, we can suppose, mesh triangles is much smaller than camera radius, so we can check the vertices and not the triangles
			contact = @_getClosestContactMeshAndSphereVertices(node, geometry, meshCameraPosition, cameraRadiusSq)

		return unless contact
		return contact.applyMatrix4(mesh.matrix)


	_getClosestContactMeshAndSphereVertices: (node, geometry, meshCameraPosition, cameraRadiusSq) ->
		foundContact = false
		closestContactPoint = new THREE.Vector3();
		closestContactSq = 0.0

		vertex = new THREE.Vector3()

		geometryVertices = geometry.getAttribute('position').array

		for vertexIdx in [0...geometryVertices.length - node.skirtVerticesCount] by 1
			arrayIndex = vertexIdx * 3
			vertex.x = geometryVertices[arrayIndex + 0]
			vertex.y = geometryVertices[arrayIndex + 1]
			vertex.z = geometryVertices[arrayIndex + 2]

			if node.elevationList.length > 0
				elevation_value = node.vertexElevation(vertexIdx) / wgs84.earthRadius + 1.0
				vertex.multiplyScalar(elevation_value)

			distSq = meshCameraPosition.distanceToSquared(vertex)
			if(distSq < cameraRadiusSq)
				if(!foundContact || distSq < closestContactSq)
					closestContactSq = distSq
					closestContactPoint.copy(vertex)
				foundContact = true

		return unless foundContact
		return closestContactPoint

	_getClosestContactMeshAndSphereRay: (node, geometry, meshCameraPosition, cameraRadiusSq) ->
		ray = new THREE.Ray(meshCameraPosition)
		ray.direction.copy(meshCameraPosition).normalize().negate()

		contactPoint = new THREE.Vector3();

		vertexA = new THREE.Vector3()
		vertexB = new THREE.Vector3()
		vertexC = new THREE.Vector3()

		geometryVertices = geometry.getAttribute('position').array
		geometryIndices = geometry.index.array

		faceCount = geometryIndices.length / 3

		loadVertex = (vector, vertexIndex) ->
			arrayIndex = vertexIndex * 3
			vector.x = geometryVertices[arrayIndex + 0]
			vector.y = geometryVertices[arrayIndex + 1]
			vector.z = geometryVertices[arrayIndex + 2]

		for faceIdx in [0...faceCount] by 1
			arrayIndex = faceIdx * 3
			faceA = geometryIndices[arrayIndex + 0]
			faceB = geometryIndices[arrayIndex + 1]
			faceC = geometryIndices[arrayIndex + 2]

			loadVertex(vertexA, faceA)
			loadVertex(vertexB, faceB)
			loadVertex(vertexC, faceC)

			if node.elevationList.length > 0
				elevation_value = node.vertexElevation(faceA) / wgs84.earthRadius + 1.0
				vertexA.multiplyScalar(elevation_value)

				elevation_value = node.vertexElevation(faceB) / wgs84.earthRadius + 1.0
				vertexB.multiplyScalar(elevation_value)

				elevation_value = node.vertexElevation(faceC) / wgs84.earthRadius + 1.0
				vertexC.multiplyScalar(elevation_value)

			result = ray.intersectTriangle(vertexA, vertexB, vertexC, false, contactPoint)
			if result != null
				if !cameraRadiusSq or result.distanceToSquared(meshCameraPosition) <= cameraRadiusSq
					return contactPoint

		return undefined

	findNodeUnderPoint: (position) ->
		rayPosition = position.clone().normalize().multiplyScalar(wgs84.earthRadius*1.5)
		rayDir = rayPosition.clone().normalize().negate()
		ray = new THREE.Ray(rayPosition, rayDir)
		
		contactPoint = new THREE.Vector3();
		
		checkChildNodes = (node) ->
			mostAccurate = node
			for c in node.children
				if !c.isActive() 
					continue
				result = ray.intersectTriangle(c.worldBoundVertices[0], c.worldBoundVertices[1], c.worldBoundVertices[2], true, contactPoint)
				result2 = ray.intersectTriangle(c.worldBoundVertices[3], c.worldBoundVertices[0], c.worldBoundVertices[2], true, contactPoint)
				
				if result or result2
					return checkChildNodes(c)
			return node
		
		node =  checkChildNodes(@nodes[''])
		return node
	
	getNodeAndUvFromLocation: (position) ->
		#convert into latlon if needed
		if 'x' of position
			position = wgs84.xyzToLatLon(position)

		#convert into rhombus UV if needed
		if 'lat' of position
			radiansLatLon = new PYXIS.snyder.CoordLatLon();
			radiansLatLon.setInDegrees(position.lat,position.lon);
			rhombusUv = PYXIS.snyder.RhombusMath.wgs84LatLonToRhombusUV(radiansLatLon)
		else if 'rhombus' of position
			rhombusUv = position
		else
			throw "unsupported location type"

		nextRhombusUv = PYXIS.snyder.RhombusMath.increaseRhombusResolution(rhombusUv)

		while nextRhombusUv.rhombus of @nodes && @nodes[nextRhombusUv.rhombus].elevationList.length > 0
			rhombusUv = nextRhombusUv
			nextRhombusUv = PYXIS.snyder.RhombusMath.increaseRhombusResolution(rhombusUv)

		return {
			rhombusUv: rhombusUv
			node: @nodes[rhombusUv.rhombus]
		}
	
	getLocationElevation: (position) ->
		result = @getNodeAndUvFromLocation(position)
		return 0.0 if !result.node || !result.node.elevationList || result.node.elevationList.length == 0
		return result.node.getElevationFromUV(result.rhombusUv.u,result.rhombusUv.v)
		
	findVisibleKeys: (camera, screenSize) ->
		camera.frustum = new THREE.Frustum() if camera.frustum is undefined
		tempMatrix4.multiplyMatrices(
				camera.projectionMatrix,
				camera.matrixWorldInverse
				)

		camera.frustum.setFromMatrix(tempMatrix4)

		@maxDistanceFromCamera = Math.sqrt( camera.position.lengthSq() - SCALE * SCALE )

		@currentCamera = camera

		@currentCameraTag++

		@activeNodesCount = 0

		return @nodes[''].updateVisibility(camera, screenSize)


	updateWithCamera: (camera, screenSize) ->
		@screenSize.set(screenSize.width,screenSize.height)

		for i in [0...@updateList.length]
			@updateList[i].update()

		if @updateListToRemove.length > 0
			for i in [0...@updateListToRemove.length]
				node = @updateListToRemove[i]

				if node.updateListId == -1
					continue

				if @updateList.length > 1
					#replace removing node by the last node
					@updateList[node.updateListId] = @updateList[@updateList.length - 1]
					@updateList[node.updateListId].updateListId = node.updateListId

				node.updateListId = -1
				@updateList.length--


			@updateListToRemove.length = 0

		if @visibleKeysUpdate
			@visibleKeys = @findVisibleKeys(camera, screenSize)

			if @visibleKeys != undefined && @visibleKeys != []
				@root.children = _.map @visibleKeys, (key) => @meshes[key]

			@visibleKeysUpdate = false

		@throttledTrim()

		return true

	performTrim: ->
		@nodes[''].trimIfPossible()

	loadLayers: (layers, options) ->
		console.log "LOAD LAYERS ", layers, options

		# keep the old layers around to compare against if we need to rebuild shader
		@previousLayers = @renderLayers
		@renderLayers = layers

		@shaderFactory = shaderFactory(this, @dataProviders, @renderLayers, @theme)

		#devided in two phases because JitModule.load can trigger recursive update on childs nodes with invalid material
		#phase 1 update material and store previous material
		for id, node of @nodes
			if node.mesh
				mesh = node.mesh
				previousMaterial = mesh.material

				mesh.material = @shaderFactory.clone()
				mesh.material.uniforms.heightMultiplier.value = @_elevationMultiplier

				context = node.getJitContext();
				context.setPreviousMaterial(previousMaterial)
		#do actual load and dispose previous material
		for id, node of @nodes
			if node.mesh
				mesh = node.mesh
				context = node.getJitContext();
				previousMaterial = context.getPreviousMaterial()
				previousMaterial.jit.dispose(previousMaterial.meshAttributes,previousMaterial.uniforms, context )
				mesh.material.jit.init(mesh.geometry, mesh.material.meshAttributes, mesh.material.uniforms, context )
				mesh.material.jit.load(mesh.geometry, mesh.material.meshAttributes, mesh.material.uniforms, context )
				context.setPreviousMaterial(undefined)


	isGeoSourceLoading: (geoSourceId) ->
		return @dataProviders["geoSource"].isGeosourceIsLoading(geoSourceId)



class RhombusNode
	constructor: (@key, @tree) ->
		self = this
		@boundSpheresTag = 0
		# console.log "node create ", @key, @tree

		if @tree.nodes[@key]?
			console.error("KEY SHOULD NOT ALREADY EXIST IN NODE MAP")

		@children = []
		@tree.nodes[@key] = this

		@tag = 0#@tree.currentCameraTag

		@meshSize = @tree.meshSize
		@skirtVerticesCount = 0

		@elevationList = []

		if @key isnt ''
			@parent = @tree.at(@tree.hierarchy.parent(@key))
			@parent.children.push this

			@tree.addToUpdateList(this)

			@tree.backgroundTasks.schedule () =>
				self = this
				@tree.dataProviders.geometry.getData @key,
					{
						size: @meshSize,
						skirt: @tree.useSkirt,
						priority: self.getGeometryPriorityClosure(),
						enabled: () ->
							return false if self.disposed
							#if the parent doesn't need children anymore...
							if self.canBeTrimmed() && self.parent && !self.parent.needChildren
								#try to trim parent to avoid this request
								self.parent.trimIfPossible()

							#return true if node is not yet been disposed
							return !self.disposed
					},
					(geometry) ->
						self.processGeometry(geometry)


	###
		Loading Priority
	###
	getGeometryPriority: ->
		if @key.length == 1 || @disposed
			return 100

		if !@parent.isActive()
			return 0

		priority = 100 - (@parent.distanceToCamera / @parent.context.boundingSphere.radius * 10.0)

		return priority

	# used just to bind this to function
	getGeometryPriorityClosure: ->
		self = this
		return -> self.getGeometryPriority()

	getGeoSourcePriority: ->
		#how do we calculate priority ?
		#1) based on distance to the camera?
		#2) based if the mesh is visbile or not?
		#3) based on disatance to the mouse on the sphere?
		#4) Top rhomuses have max priority

		if @key.length == 1 || @disposed
			return 100

		if !@tree.currentCamera || !@context
			return 0

		boundingSphere = @context.boundingSphere;

		distanceToCamera = boundingSphere.center.distanceTo(@tree.currentCamera.position)

		# calcualte the size of the sphere on screen. which the ratio betwee the sphere size and the distance to our position
		relativeSizeOnScreen = boundingSphere.radius / distanceToCamera

		priority = 100 - (distanceToCamera / boundingSphere.radius * 10.0)
		
		#if this node has more then 1 visible children, load it first 
		if @numOfVisibleChildren > 1
			priority += @numOfVisibleChildren * 20
		# check if this node is no longer visible
		else unless @key in @tree.visibleKeys
			priority -= 100


		# note sizeScale seems to always be zero, so I disabled this

		# if self.isActive()
		# 	sizeScale = Math.floor(self.pixels / self.tree.rhombusDivideThreshold)
		# 	switch sizeScale
		# 		when 1 then priority += 30  # just the right size
		# 		when 2 then priority += 20  # little to big, but close enough
		# 		else priority += 1         # all other active sizes


		# priority += (boundingSphere.radius-distanceToCamera) / self.tree.maxDistanceFromCamera

		return priority

	# used just to bind this to function
	getGeoSourcePriorityClosure: ->
		self = this
		return -> self.getGeoSourcePriority()

	###*
	 * isActive return if node is inside visiblie tree
	 *
	 * please note: active not neseery mean visible
	 * @return {Boolean} true if node is active
	###
	isActive: () -> @tree.currentCameraTag == @tag

	#TODO not working after JitShader implementation
	isLoaded: () ->
		return @mesh && @mesh.textureLoaded == @mesh.textureLoadedTarget

	###*
	 * return if this node has no children
	 * @return {Boolean} true if node has no children
	###
	isLeafNode: () -> @children.length == 0

	###*
	 * return node can be trimmed from the tree.
	 *
	 * usally, a node can be trimmed if it leaf node that is idle and not active
	 *
	 * @return {[type]} true if node can be trimmed from the tree, aka dispose
	###
	canBeTrimmed: () -> @key.length > 2 && !@isActive() && @isLeafNode()

	###*
	 * checking that this node was draw on previous frame
	 * @return {Boolean} true if node has all active childs
	###
	allActiveChildren: () ->
		for child in @children
			return false if !child.isActive()

		return true

	###*
	 * checking that this node was draw on previous frame
	 * @return {Boolean} true if node has all active childs
	###
	anyActiveChildren: () ->
		for child in @children
			return true if child.isActive()

		return false

	###*
	 * try to trim a node
	###
	trimIfPossible: () ->
		return if @isLeafNode()

		allChildrenCanBeDisposed = true

		for child in @children
			child.trimIfPossible()
			allChildrenCanBeDisposed &= child.canBeTrimmed()

		return unless allChildrenCanBeDisposed
		# if node is active...
		if @isActive
			# then avoid trimming if we need children
			return if @needChildren

		#time node
		for child in @children
			child.dispose()

		@children.length = 0

		return

	###*
	 * dispose node resources
	###
	dispose: () ->
		return if @key == '' || @disposed

		@tree.removeFromUpdateList(this)

		@disposed = true

		if @parent
			delete @parent

		delete @tree.meshes[@key]
		delete @tree.nodes[@key]

		return unless @mesh

		mesh = @mesh

		disposeGeometry = () =>
			@tree.dataProviders.geometry.releaseData(@key, mesh.geometry)

			mesh.dispatchEvent( { type: 'removed' } );
			mesh.geometry.dispose()

			mesh.material.jit.dispose(mesh.material.meshAttributes, mesh.material.uniforms, @context)
			mesh.material.dispose()

			@context.dispose()
			@context = undefined

		@tree.backgroundTasks.schedule disposeGeometry
		return

	###*
	 * returns vertex elevations on specified point in meters above the sea
	###
	vertexElevation: (vertexIdx) ->
		for i in [@elevationList.length-1..0]
			elevationRecord = @elevationList[i]
			sample = elevationRecord.values[vertexIdx]
			if sample >= 0.0
				sample = (elevationRecord.min + (elevationRecord.max - elevationRecord.min) * sample) * elevationRecord.scale
				return sample
		return 0.0

	getElevationFromUV: (u,v) ->
		meshSizeMinus1 = @meshSize - 1
		intU = Math.min(meshSizeMinus1-1, Math.floor(meshSizeMinus1 * u));
		intV = Math.min(meshSizeMinus1-1, Math.floor(meshSizeMinus1 * v));
		
		du = meshSizeMinus1 * u - intU;
		dv = meshSizeMinus1 * v - intV;
		
		if du+dv == 0
			return @vertexElevation(intV*@meshSize + intU)
		else if du+dv <= 1
			#             -> u
			# elevation0 o----o elevationU
			#            |   /|
			#       v |  |  / |
			#         V  | /  |
			#            |/   |
			# elevaitonV o----o elevation0
			#
			elevation0 = @vertexElevation(intV*@meshSize + intU)
			elevationU = @vertexElevation(intV*@meshSize + intU+1)
			elevationV = @vertexElevation((intV+1)*@meshSize + intU)
			
			return elevation0 * (1 - du - dv) + elevationU * du + elevationV * dv
		else
			#
			#            o----o elevationU
			#            |   /|
			#            |  / |  ^ 
			#            | /  |  | v
			#            |/   |  
			# elevaitonV o----o elevation0
			#
			#              <- u
			elevation0 = @vertexElevation((intV+1)*@meshSize + intU+1)
			elevationU = @vertexElevation(intV*@meshSize + intU+1)
			elevationV = @vertexElevation((intV+1)*@meshSize + intU)
			du = 1 - du
			dv = 1 - dv
			return elevation0 * (1 - du - dv) + elevationU * dv + elevationV * du

	calculateBoundingSphere: () ->
		return unless @context.boundingSphere

		geometry = @mesh.geometry
		geometryVertices = geometry.getAttribute('position').array
		geometryVerticesCount = geometryVertices.length / 3

		boundingSphere = @context.boundingSphere
		maxRadiusSq = 0.0

		vertex = new THREE.Vector3()

		centerIndex = @meshSize * @meshSize / 2 + @meshSize / 2
		center = new THREE.Vector3( geometryVertices[centerIndex * 3], geometryVertices[centerIndex * 3 + 1], geometryVertices[centerIndex * 3 + 2] )

		if @elevationList.length > 0
				elevation_value = @vertexElevation(centerIndex) / wgs84.earthRadius + 1.0
				center.multiplyScalar(elevation_value)

		boundingSphere.center = center

		#calculate sphere
		for vertexIdx in [0...geometryVerticesCount - @skirtVerticesCount] by 1
			vertex.x = geometryVertices[vertexIdx * 3 + 0]
			vertex.y = geometryVertices[vertexIdx * 3 + 1]
			vertex.z = geometryVertices[vertexIdx * 3 + 2]

			if @elevationList.length > 0
				elevation_value = @vertexElevation(vertexIdx) / wgs84.earthRadius + 1.0
				vertex.multiplyScalar(elevation_value)

			maxRadiusSq = Math.max( maxRadiusSq, center.distanceToSquared( vertex ) );

		radius = geometry.boundingSphere.radius
		radius = Math.sqrt(maxRadiusSq)

		geometry.boundingSphere.center.copy(center)
		geometry.boundingSphere.radius = radius

		@context.boundingSphere.center.applyMatrix4(@mesh.matrix)
		@context.boundingSphere.radius = radius * SCALE

		@boundSpheresTag = @tree.boundSpheresTag

		#debug code for displaying a bounding spheres as scene geometry objects
		if false
			if @sphereMesh != undefined
				@sphereMesh.geometry.dispose()
				@sphereMesh.material.dispose()
				@mesh.remove(@sphereMesh)
				@sphereMesh = undefined

			if @elevationList.length > 0
				color = '#0f0'
			else
				color = '#f00'
			sphereLines = new THREE.MeshBasicMaterial( { wireframe: true, color : color } );
			sphereGeometry = new THREE.SphereGeometry( radius, 32, 16 );
			@sphereMesh = new THREE.Mesh( sphereGeometry, sphereLines );
			@sphereMesh.position.copy(@context.boundingSphere.center)
			@sphereMesh.position.applyMatrix4(@invMatrix)
			@mesh.add(@sphereMesh)

	isValid: () ->
		self = this

		#if we don't have a bounding sphere...
		if @context is undefined || @context.boundingSphere is undefined
			return false

		jit = @mesh.material.jit

		# not all elevations updated them selves
		if self.elevationList.length != jit.elevations.length
			return false
		return true 

	update: () ->
		if @key != ''
			if !@isValid()
				return undefined

			jit = @mesh.material.jit

			# check if we loaded all resources to display something
			if !jit.wasReady and !jit.ready(@mesh.material.meshAttributes, @mesh.material.uniforms, @context )
				return undefined

			if @context.loadInitiated == false
				@context.loadInitiated = true
				jit.load( @mesh.geometry, @mesh.material.meshAttributes, @mesh.material.uniforms, @context )
				@tree.removeFromUpdateList(this)

			#once it ready it ready
			jit.wasReady = true	

	updateVisibility: (camera, screenSize) ->

		self = this

		if @key is ''
			result = []
			for c in @children
				r = c.updateVisibility(camera, screenSize)
				result = result.concat(r) if r?  #more than just a push because I need to merge the lists

			@numOfVisibleChildren = 0;
			self.tag = @tree.currentCameraTag
			return result
		else
			if !@isValid()
				return undefined

			#reset needChildren to be safe
			@needChildren = false

			@distanceToCamera = @context.boundingSphere.center.distanceTo(camera.position)

			#first check if @boudingSphere is on the right side of the sphere
			return [] if @distanceToCamera - @context.boundingSphere.radius > @tree.maxDistanceFromCamera

			#check if it intersect frustum
			return [] unless camera.frustum.intersectsSphere(@context.boundingSphere)
			
			# check if all edge verticies(normals from center) of mesh looks to the camera( not more than 90 degrees between eye vector, edge vertex normal )
			
			### TODO: this code produce some missing rhombuses, we need to investigate why
			meshCameraPos = camera.position.clone().applyMatrix4(self.invMatrix)
			meshCameraPos.normalize()
			visibleCheck = false

			tmpVector = new THREE.Vector3()
			for i in [0..3]
				view_vector = tmpVector.copy(self.bound_vertices[i])
				view_vector.sub(meshCameraPos)
				view_vector.normalize()
				visibleCheck |= self.bound_vertices[i].dot(view_vector) >= 0.0;
			return [] if !visibleCheck
			###
			
			self.tag = @tree.currentCameraTag
			@numOfVisibleChildren = 0
			@tree.activeNodesCount++

			if @tree.hierarchy.isLeaf(@key)
				@parent.numOfVisibleChildren++ if @parent
				return @key

			@pixels = @calculateScreenSize(camera, screenSize)
			if @pixels > @tree.rhombusDivideThreshold
				@needChildren = true
				result = []

				if @children.length == 0
					if @tree.frozen
						return @key
					#populate children
					for key in @tree.hierarchy.children(@key)
						@tree.at(key)

				for child in @children
					r = child.updateVisibility(camera, screenSize)
					if r is undefined
						@parent.numOfVisibleChildren++ if @parent
						return @key
					result = result.concat(r)

				return result

			else
				@parent.numOfVisibleChildren++ if @parent
				return @key



	###
		The function ported from desktop version of the World View Studio, with small changes
	###
	calculateScreenSize: (camera, screenSize) ->
		return undefined if @context.boundingSphere is undefined
		if @context.key.length == 1
			return 1000.0
		r = @context.boundingSphere.radius
		eye = camera.position.clone().sub(@context.boundingSphere.center)
		d = eye.length()
		eye.normalize()

		angle = Math.atan2(r, d) * 180 / Math.PI

		normal = @context.boundingSphere.center.clone()
		normal.normalize()

		normalAngleCos = Math.abs(normal.dot(eye))
		#The coefficients are differs from the WVStudio
		normalAngleCos = Math.sqrt(normalAngleCos)*0.5+0.5;
		size = screenSize.height / camera.fov * angle * normalAngleCos;

		return size;

	getJitContext: () ->
		self = this

		if !@context
			@context =
				key: @key
				meshSize: @meshSize
				skirtVerticesCount: @skirtVerticesCount
				uniforms: undefined
				parentContext: undefined
				boundingSphere: new THREE.Sphere()

				previousMaterial: undefined
				previousMaterialTextures: {}
				previousMaterialUsedTexture: []
				loadInitiated : false

				setPreviousMaterial: (material) ->
					if self.context.previousMaterial
						for name,uniform of self.context.previousMaterial.uniforms
							if uniform.type == 't' and self.context.previousMaterialUsedTexture.indexOf(uniform.value) != -1
								uniform.value = undefined

					self.context.previousMaterial = material

					self.context.previousMaterialTextures = {}
					self.context.previousMaterialUsedTexture = []

					if material
						for name,uniform of material.uniforms
							if uniform.type == 't' and uniform.value and uniform.value.image
								self.context.previousMaterialTextures[uniform.value.image.src] = uniform.value

				getPreviousMaterial: ->
					return self.context.previousMaterial

				getPreviousModule: (module) ->
					return undefined unless self.context.previousMaterial
					return self.context.previousMaterial.jit.tryGetCompiledModule module

				getPreviousTexture: (url) ->
					texture = self.context.previousMaterialTextures[url]

					if texture
						self.context.previousMaterialUsedTexture.push texture

					return texture

				getPreviousAttribute: (name) ->
					return self.context.previousMaterial.meshAttributes[name]

				loadTexture: (geoSource, callback, errorCallback, format, range) ->
					geoSource.getData(self.key,
						{
							format: format,
							range: range,
							success: (texture) ->
								callback(texture) if callback

							error: () ->
								errorCallback(self.key) if errorCallback

							enabled: () ->
								return true if self.key.length == 1
								return false if self.disposed
								return self.isActive() || self.parent && self.parent.needChildren && self.parent.isActive()

							priority: self.getGeoSourcePriorityClosure()
						}
					)
				disposeTexture: (geoSource, texture) ->
					geoSource.releaseData(self.key, texture)
				foreachChildren: (callback) ->
					for child in self.children
						if child.mesh
							callback(child.mesh.material.meshAttributes, child.mesh.material.uniforms,child.getJitContext())
				dispose: () ->
					@parentContext = undefined
					@uniforms = undefined

			EventEmitter(@context)

			@context.on 'elevationChanged', (args) ->
				self.elevationList[args.module.elevationIndex] = {
					values: args.vertexElevations,
					scale: args.module.scale,
					min: args.module.source.min,
					max: args.module.source.max,
				}
				self.calculateBoundingSphere()
				self.tree.emit 'elevationChanged', {
					key: self.key
				}

			@calculateBoundingSphere()

		#update parent context
		@context.parentContext = @parent.context if @parent
		@context.uniforms = @mesh.material.uniforms

		return @context

	processGeometry: (geometry) ->
		self = this

		@meshSize = geometry.options.size

		material = @tree.shaderFactory.clone()
		material.combine = THREE.MixOperation
		material.uniforms.heightMultiplier.value = @tree.getElevationMultiplier()

		vertexTopRight = @meshSize-1
		vertexBottomLeft = @meshSize*(@meshSize-1)
		vertexBottomRight = @meshSize*@meshSize-1
		geometryVertices = geometry.getAttribute('position').array
		@bound_vertices = [
			new THREE.Vector3(geometryVertices[0],geometryVertices[1],geometryVertices[2]),
			new THREE.Vector3(geometryVertices[vertexTopRight*3 + 0],geometryVertices[vertexTopRight*3 + 1],geometryVertices[vertexTopRight*3 + 2])
			new THREE.Vector3(geometryVertices[vertexBottomRight*3 + 0],geometryVertices[vertexBottomRight*3 + 1],geometryVertices[vertexBottomRight*3 + 2])
			new THREE.Vector3(geometryVertices[vertexBottomLeft*3 + 0],geometryVertices[vertexBottomLeft*3 + 1],geometryVertices[vertexBottomLeft*3 + 2])
		]

		if geometry.options.skirt
			@skirtVerticesCount = @tree.dataProviders.geometry.getSkirtVerticesCount(@meshSize)

		mesh = new THREE.Mesh(geometry, material)

		mesh.scale.set(SCALE, SCALE, SCALE)
		# mesh.castShadow = true
		mesh.rotation.set(-PI / 2, 0, 0)
		mesh.updateMatrix()
		mesh.key = @key

		mesh.textureLoaded = 0
		mesh.requests = {}

		@mesh = mesh
		#@borrowParentTextures()
		
		@worldBoundVertices = [
			@bound_vertices[0].clone().applyMatrix4(mesh.matrix),
			@bound_vertices[1].clone().applyMatrix4(mesh.matrix),
			@bound_vertices[2].clone().applyMatrix4(mesh.matrix),
			@bound_vertices[3].clone().applyMatrix4(mesh.matrix)
		]
		
		material.jit.init(geometry, material.meshAttributes, material.uniforms, @getJitContext() )
		
		#always load top level rhombuses
		if @getJitContext().key.length == 1
			@getJitContext().loadInitiated = true
			material.jit.load(geometry, material.meshAttributes, material.uniforms, @getJitContext() )

		@tree.meshes[@key] = mesh

		@invMatrix = new THREE.Matrix4()
		@invMatrix.getInverse(mesh.matrix)
		@calculateBoundingSphere()

		@tree.invalidateVisibilityList()

		# bsGeo = new THREE.IcosahedronGeometry(mesh.geometry.boundingSphere.radius * SCALE * 100, 2)
		# bsMat = new THREE.MeshBasicMaterial( { wireframe: true, wireframeLinewidth: 1.1, color: 0x000000 } )
		# bsMesh = new THREE.Mesh( bsGeo, bsMat )
		# bsMesh.position.set(@boundingSphere.center.x, @boundingSphere.center.y, @boundingSphere.center.z)
		# window.BSMESHES[@key] = bsMesh
		# @tree.object3d.parent.add( bsMesh )


window.BSMESHES = {}

window.debugCameraHit = ->
	tree = window.GC.root.tree
	camera = window.GC.getCamera()

	screenSize = {width:window.GC.width, height:window.GC.height}

	_.each tree.nodes, (node, key) ->
		return unless BSMESHES[key]
		BSMESHES[key].material.color.g = 0.0

		console.log "TEST KEY ", key, node.calculateScreenSize(camera, screenSize)
		if node and node.context.boundingSphere and camera.frustum.intersectsSphere(node.context.boundingSphere)
			console.log "HIT ", key
			if node.calculateScreenSize(camera, screenSize) > tree.rhombusDivideThreshold
				BSMESHES[key].material.color.g = 1.0

				result = []

				for i in [0...9] by 1
				   r = tree.at(tree.hierarchy.child(key, i)).updateWithCamera(camera)
				   console.log "R ", r, i
				   return tree.meshes[key] if r is undefined
				   result = result.concat(r)

				console.log "RESULT ?? ", result
				return result

module.exports =
	RhombusTree: RhombusTree
	RhombusNode: RhombusNode

