###
	This is a coffeescript file that compiles-to-javascript.

	Define the Globe Widget - a Three.js compatible Globe 
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
glslify = require 'glslify'
EventEmitter = require 'event-emitter'
wgs84 = require '../core/wgs84'

RhombusTree = require('./tree/rhombus').RhombusTree
IconTree = require('./tree/icon').IconTree
TextHoverTree = require('./tree/icon-text').TextHoverTree

referenceSphere = new THREE.Sphere(new THREE.Vector3(0,0,0), wgs84.earthRadius)
tmpVector3 = new THREE.Vector3()

class Globe extends THREE.Object3D
	constructor: (@canvas, @theme, @dataProviders, renderLayers) ->
		self = this

		THREE.Object3D.call(this)

		@settings =
			refineSurfaceFromCamera: true

		# we don't need to check hover on every frame, so use a throttled handler
		hoverCheck = -> self.intersectIconTrees.apply(self, arguments)
		@throttledIconHoverCheck = _.throttle(hoverCheck, 100)
		@iconHoverIntersects = []

		@loadLayers(renderLayers)

	dispose: () ->
		@tree.dispose()

	###
		Most of the Globe loading functionality will come through these layers.
	###
	loadLayers: (layers, options) ->
		self = this

		@layers = layers

		renderLayers = _.reject layers, (layer) -> layer.style.Icon

		#create rhombus tree if needed
		if !@tree
			# add the base objects
			@rhombusRoot = new THREE.Object3D()
			@tree = new RhombusTree(@rhombusRoot, @theme, @dataProviders, @canvas.backgroundTasks.category('rhombus'), [])
			@add(@rhombusRoot)
			
			@tree.on 'elevationChanged', _.throttle((-> self.updateIconsElevations()), 5000)
				

		@tree.loadLayers(renderLayers, options)

		_.each @iconTrees, (it) ->
			self.remove(it.root)

		@iconTrees = []

		_.each layers, (layer) ->
			if layer.style and layer.style.Icon
				iconRoot = new THREE.Object3D()

				if self.canvas.config.iconHover is 'custom'
					iconTree = new IconTree(self, layer, iconRoot, self.canvas.backgroundTasks.category('icons'))
				else
					iconTree = new TextHoverTree(self, layer, iconRoot, self.canvas.backgroundTasks.category('icons'))
				self.iconTrees.push(iconTree)


				self.add(iconRoot)


				# push to scene once the 3DObject is working
				# self.scene.add(tree)

	updateIconsElevations: () ->
		_.each @iconTrees, (ic) ->
			ic.updateIconsElevations()

	isGeoSourceLoading: (geoSourceId) ->
		return @tree.isGeoSourceLoading(geoSourceId)

	###
		Invalidate current draw list
	###
	invalidateVisibilityList: ->
		@tree.invalidateVisibilityList()
		_.each @iconTrees, (ic) ->
			ic.invalidateVisibilityList()

	###
		Build a draw list and dispose non visible nodes
	###
	updateWithCamera: (camera, screenSize) ->

		return unless @settings.refineSurfaceFromCamera

		@tree.updateWithCamera(camera, screenSize)

		_.each @iconTrees, (ic) ->
			ic.updateWithCamera(camera)


	###
		Changing the hover interaction to avoid using the PointCloud Raycaster
		and instead raycast 
	###
	intersectIconTrees: (cameraController) ->
		self = this

		hoverRay = cameraController.raycaster.ray
		mousePosition = cameraController.mouseOnGlobe

		return unless mousePosition


		# console.log "intersectIconTrees"

		# @lastIconHoverIntersects = @iconHoverIntersects

		# build an array of key objects that intersect
		intersectArray = []

		for treeIndex, ic of @iconTrees

			# as we map the boundingSpheres we also add the key and treeIndex so we can
			# map from a boundingSphere to a node
			boundingSpheres = []
			for key in ic.visibleKeys 
				bounds = ic.nodes[key].boundingSphere
				bounds.key = key
				bounds.treeIndex = treeIndex
				boundingSpheres.push(bounds)

			for boundingSphere in boundingSpheres
				if hoverRay.isIntersectionSphere(boundingSphere)
					intersectArray.push({
						treeIndex: treeIndex
						key: boundingSphere.key
						vertexIndex: null #iconIntersections[i].index
					})

		if intersectArray.length

			# we now need to transform the intersection array of boundingSpheres into
			# the actual point cloud points so we can determine which is closest to the mouse
			# get normalized distance to earth radius -- we only care about the 
			# 2D distance for now.  We might need to revisit for tilted items
			points = []
			for intersection in intersectArray
				node = self.iconTrees[intersection.treeIndex].nodes[intersection.key]
				positionsArray = node.mesh.geometry.attributes.position.array
				vertexCount = positionsArray.length / 3
				for vertexIndex in [0...vertexCount]
					tmpVector3.set(positionsArray[vertexIndex*3 + 0], positionsArray[vertexIndex*3 + 1], positionsArray[vertexIndex*3 + 2])

					point = tmpVector3.normalize().multiplyScalar(wgs84.earthRadius)
					mp = mousePosition.normalize().multiplyScalar(wgs84.earthRadius)
					points.push({
						treeIndex: intersection.treeIndex
						key: intersection.key
						vertexIndex: vertexIndex
						distance: mp.distanceTo(point)
					})


			# all we need is the closest
			closestNode = _.min points, (keyObject) ->
				return keyObject.distance


			maxDistance = cameraController.rangeRatio * wgs84.earthRadius * 0.025

			# because we set a wide radius for the raycast we need to then trim out
			# any nodes that are not within a certain radius of the icon
			if closestNode.distance < maxDistance
				@setHover(closestNode)
				intersectArray = [closestNode]
			else
				@setHover()
				intersectArray = []
		else
			@setHover()
				

		return intersectArray



	setHover: (newNode) ->
		if newNode and @isValidHoverNode(newNode) is false
			newNode = undefined

		if @iconHover and newNode is undefined
			@iconHoverEnd()
			@iconHover = undefined

		if @iconHover is undefined and newNode
			@iconHover = newNode
			@iconHoverStart()

		if @iconHover and newNode
			unless @iconHover.key is newNode.key and @iconHover.vertexIndex is newNode.vertexIndex 
				@iconHoverEnd()
				@iconHover = newNode
				@iconHoverStart()

	iconHoverStart: ->
		return unless @iconHover
		node = @getIconHoverNode()
		node.hoverStart(@iconHover.vertexIndex)
		@canvas.emit 'iconHoverStart', node.getHoverData(@iconHover.vertexIndex)
		
	iconHoverEnd: ->
		return unless @iconHover
		node = @getIconHoverNode()
		node.hoverEnd(@iconHover.vertexIndex)
		@canvas.emit 'iconHoverEnd', node.getHoverData(@iconHover.vertexIndex)

	getIconHoverNode: (iconHover) ->
		iconHover or= @iconHover
		return null unless iconHover
		return @iconTrees[iconHover.treeIndex].nodes[iconHover.key]

	# if we are hovering over a hidden parent return false
	isValidHoverNode: (iconHover) ->
		node = @getIconHoverNode(iconHover)
		return node.isVisible(iconHover.vertexIndex)

	# once hovered for a certain amount of time
	# we can display text popup or whatever the tree defines
	iconHoverDuration: ->
		return unless @iconHover

		if @iconTrees[@iconHover.treeIndex].textHoverEnabled
			node = @iconTrees[@iconHover.treeIndex].nodes[@iconHover.key]
			node.textHover(@iconHover.vertexIndex)

	#freeze new requests (geometry and geosource data)
	freeze: () ->
		@tree.freeze()
		
		
	#unfreeze new requests (geometry and geosource data)
	unfreeze: () ->
		@tree.unfreeze()



# export only one Class
module.exports = Globe