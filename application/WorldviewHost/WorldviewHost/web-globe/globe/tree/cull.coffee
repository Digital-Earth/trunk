###
	Generic Culling Tree, for Icons and other items 
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
EventEmitter = require 'event-emitter'

PyxisHierarchy = require('./hierarchy').PyxisHierarchy

tempMatrix4 = new THREE.Matrix4()

HORIZON_FALLOFF = 0.8  # lower number the later things fall off the horizon
window.horizonFalloff = 0.8 # remove
SCALEx100 = 63710.07

occasionalLog = -> console.log.apply(console, arguments)
occasionalLog = _.throttle(occasionalLog, 1000) # only call every second


class CullingTree
	constructor: (@root, @NodeClass, @NodeHierarchyClass) ->
		self = this

		# Allow event subscribers
		EventEmitter(this)

		@NodeClass or= CullingNode
		@NodeHierarchyClass or= PyxisHierarchy

		@type = 'CullingTree'
		@nodes = {}
		@meshes = {}
		@visibleKeys = []
		@visibleKeysUpdate = true
		@lastVisibleKeys = []
		@frameCallbacks = []
		@children = []
		@currentCamera = undefined
		@hierarchy = new this.NodeHierarchyClass(self)

		# we don't need to perform cleanup on every frame
		# @throttledTrim = ->
		@throttledTrim = _.throttle((-> self.performTrim()), 30000)

		# spawn nodes
		_.each @hierarchy.children(), (key) -> new self.NodeClass(key, self)


	at: (key) ->
		return @nodes[key] if key of @nodes
		return new this.NodeClass(key, this)

	invalidateVisibilityList: ->
		@emit 'invalidateVisibilityList'
		@visibleKeysUpdate = true


	findVisibleKeys: (camera) ->
		camera.frustum = new THREE.Frustum() if camera.frustum is undefined
		tempMatrix4.multiplyMatrices(
				camera.projectionMatrix,
				camera.matrixWorldInverse
				)

		camera.frustum.setFromMatrix(tempMatrix4)

		@currentCamera = camera

		return @nodes[''].updateVisibility(camera) if @nodes['']

	onNextFrame: (callack) ->
		@frameCallbacks.push callack

	doSingleFrameCallback: ->
		if @frameCallbacks.length
			callback = @frameCallbacks.pop()
			callback()

	updateWithCamera: (camera) ->


		if @visibleKeysUpdate
			@lastVisibleKeys = @visibleKeys
			@visibleKeys = @findVisibleKeys(camera)
			if @visibleKeys != undefined && @visibleKeys != []
				@root.children = _.map @visibleKeys, (key) => @meshes[key]


			# TODO :: do we always want to make this comparison at the end?
			addedKeys = _.difference @visibleKeys, @lastVisibleKeys
			removedKeys = _.difference @lastVisibleKeys, @visibleKeys

			if addedKeys or removedKeys
				@emit("show", addedKeys) if addedKeys && addedKeys.length
				@emit("hide", removedKeys) if removedKeys && removedKeys.length

			@visibleKeysUpdate = false

		# we don't need to trim every frame, how about every 10 seconds ?
		@throttledTrim()

		@doSingleFrameCallback()
		@doSingleFrameCallback()


	performTrim: ->
		return unless @nodes['']
		startCount = _.size(@nodes)
		@nodes[''].trimIfPossible() 

		console.log "trim icon tree before / after", startCount, _.size(@nodes)


	# debug utility
	checkIfAllChildrenRenderable: ->
		renderable = []

		_.each @children, (child) ->
			renderable.push child if child.material and child.material.uniforms

		return renderable.length is @children.length



class CullingNode
	constructor: (@key, @tree) ->
		self = this

		if @tree.nodes[@key]?
			console.error("KEY SHOULD NOT ALREADY EXIST IN NODE MAP", @key)

		@children = []
		@tree.nodes[@key] = this

	isValid: () ->
		if @boundingSphere is undefined
			return false
		return true

	update: () ->
		for c in @children
			c.update()


	updateVisibility: (camera) ->
		self = this

		if @key is ''
			result = []
			_.each @children, (c) ->
			   r = c.updateVisibility(camera)
			   result = result.concat(r) if r?  #more than just a push because I need to merge the lists

			return result
		else
			if !@isValid()
				return undefined

			mesh = @tree.meshes[@key]

			return [] unless camera.frustum.intersectsSphere(@boundingSphere)
			#return [] if camera.position.dot(@boundingSphere.center) < 0.0

			screenSize = @screenSize(camera)
			if screenSize > 1 #yeah, magic number.
				# keep key in the visible set when we zoom in so that the parents
				# are always marked as active
				result = [@key] 

				@childrenLoaded() # check if children are loaded when visible

				for key in self.tree.hierarchy.children(self.key)
				   r = self.tree.at(key).updateVisibility(camera)
				   return self.key if r is undefined
				   result = result.concat(r)

				return result

			else
				return @key


	loadItem: -> console.log "ERR: culling node loadItem stub"

	getParent: -> 
		return @parent if @parent
		return @tree.at(@tree.hierarchy.parent(@key))

	###*
	 * simple utility functions to count the number of children recursively
	###
	countChildren: ->
		total = 1

		for child in @children
			total += child.countChildren()

		return total

	countActiveChildren: ->
		total = 0
		total += 1 if @isActive()

		for child in @children
			total += child.countActiveChildren()

		return total

	countTrimmableChildren: ->
		total = 0
		total += 1 if @canBeTrimmed()

		for child in @children
			total += child.countTrimmableChildren()

		return total

	###*
	 * Check if children are loaded, called on visible nodes.
	 *
	 * implementation left to subclass
	 * @return {Boolean} true if children are loaded
	###
	childrenLoaded: -> true
	loadChildren: -> true

	###*
	 * isActive return if node is inside visiblie tree
	 *
	 * please note: active not neseery mean visible
	 * @return {Boolean} true if node is active
	###
	isActive: () -> true

	###*
	 * isLoading return if the current node is loading resources
	 * @return {Boolean} ture if node is loading
	###
	isLoading: () -> false

	###*
	 * isLoaded return if the current node is loaded
	 * @return {Boolean} ture if node is loading
	###
	isLoaded: () -> true

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
	canBeTrimmed: () -> @key.length > 2 && !@isActive() && @isLeafNode() #&& !@isLoading()

	###*
	 * checking that this node was draw on previous frame
	 * @return {Boolean} true if node has all active childs 
	###
	allActiveChildren: () ->
		for child in @children
			return false if !child.isActive()

		return true

	###*
	 * try to trim a node
	###
	trimIfPossible: () ->
		childrenToDispose = []

		for child in @children
			child.trimIfPossible()

			if child.canBeTrimmed()
				childrenToDispose.push child

		# console.log "TRIM CHILDREN ? ", @key, @children.length, childrenToDispose.length
		# only dispose children if we can dispose of all of them
		if @children.length == childrenToDispose.length
			for child in childrenToDispose
				child.dispose()

			# reset here instead of splicing
			@children = []

		return

	###*
	 * dispose node resources
	###
	dispose: () ->

		@disposed = true

		if @key isnt ''
			return unless @mesh
			# console.log "disposing icon node", @key

			# console.log ""

			mesh = @mesh

			# @tree.onNextFrame () ->

			# invoke a fake removed
			mesh.dispatchEvent( { type: 'removed' } )
			mesh.geometry.dispose()
			mesh.material.dispose()

			delete @tree.meshes[@key]
			delete @tree.nodes[@key]

			# leave up to the parent to reset children
			# parent = @getParent()
			# if parent
			# 	parent.children.splice(parent.children.indexOf(this),1)
			# 	delete @parent if @parent # this just deletes the @parent reference if it exists

		return


	###
		This function is breaking for IconTrees
	###
	screenSize: (camera) ->
		return undefined if @boundingSphere is undefined

		fov = camera.fov/ 2 * Math.PI / 180.0

		r = 40/(Math.pow(3,this.key.length-2))  # this needs to change

		dsq = camera.position.distanceToSquared(this.boundingSphere.center)
		sz = 5 * SCALEx100 * (Math.tan(fov) * r / Math.sqrt(dsq - (r * r)))
		sz = 10000000000 if sz? and sz is NaN # set to max if NaN
		@sz = sz
		return sz



window.testDraw = ->
	mat = new THREE.MeshBasicMaterial( { color: 0xffaa00, wireframe: true } )
	box = new THREE.Mesh( new THREE.BoxGeometry( 200, 200, 200 ), mat)
	window.GC.add(box)

window.testDrawBoundingSphere = (cullingNode) ->
	bounds = cullingNode.boundingSphere
	mat = new THREE.MeshBasicMaterial( { color: 0xffaa00, wireframe: true } )
	geo = new THREE.SphereGeometry( bounds.radius, 32, 32 )
	sphere = new THREE.Mesh(geo, mat)
	sphere.position.copy(bounds.center)

	window.GC.add(sphere)



module.exports = 
	CullingTree: CullingTree
	CullingNode: CullingNode