###
	This is a coffeescript file that compiles-to-javascript. !!!

	This is an IconTree using most of the same principles behind the GlobeTree 
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
glslify = require 'glslify'
EventEmitter = require 'event-emitter'
Color = require('color')


IconHierarchy = require('./hierarchy').IconHierarchy
CullingTree = require('./cull').CullingTree
CullingNode = require('./cull').CullingNode
Palette = require('../../core/palette')
wgs84 = require('../../core/wgs84')

SharedUniforms = require('../shaders/jit/shared-uniforms')

GeoJsonLoader = require('../geojson-icons-provider')
GeoSourceLoader = require('../geo-source-icons-provider')

ICON_VS = require('../shaders/icons/icon-vs.glsl')
ICON_FS = require('../shaders/icons/icon-fs.glsl')


# In order to hide icons as they move past the horizon we use this multiplier  
# 1.0 has icons falling off well past horizon, a higher number has them hiding
# sooner
ICON_HORIZON_MULTIPLIER = 1.2

# higher number shows more icons at each zoom level
ICON_CUTOFF_MULTIPLIER = 7.0

# how much bigger is a parent node compared to child
PARENT_MULTIPLIER = 1.5
PARENT_HOVER_MULTIPLIER = 3.0

class IconTree extends CullingTree
	constructor: (@globe, @layer, @root, @backgroundTasks) ->
		self = this
		console.log "ICON TREE ", @layer

		# provide events like 'loaded'
		EventEmitter(self)

		# allow overridable node classes 
		@NodeClass or= IconNode
		@NodeHierarchyClass or= IconHierarchy

		#
		CullingTree.call(self, @root, @NodeClass, @NodeHierarchyClass)
		@type = 'IconTree'

		@displayField = @layer.style.Icon.HoverExpression
		if !@displayField && @layer.styleHint && @layer.styleHint.Icon
			@displayField = @layer.styleHint.Icon.HoverExpression

		# define loader
		if @layer.type == 'GeoJson'
			#use local client icons generator
			@iconProvider = new GeoJsonLoader(@layer)
		else
			#load from geoWebCore 
			@iconProvider = new GeoSourceLoader(@layer,{maxRequests:6})

		@geometry = new THREE.Geometry()

		@palette = new Palette(@layer.style.Icon.Palette)

		# TODO :: move sprite to IconNode layer
		if @layer.style.Icon.IconDataUrl
			image = document.createElement('img')
			image.src = @layer.style.Icon.IconDataUrl
			image.crossOrigin = 'anonymous'

			@sprite = new THREE.Texture(image)
			@sprite.needsUpdate = true
			@sprite.minFilter = THREE.LinearFilter  # for images that are not power of two
		else
			@sprite = @globe.canvas.threeTextureLoader.load(@layer.style.Icon.IconUrl or '/assets/images/angular.png')

		@sharedUniforms = {
			borderColorAndIconAlpha: SharedUniforms.convertColorAndAlphaToVector4(@globe.theme.iconBorderColor, @globe.theme.iconAlpha)
		}
		@mat = materialFactory(this, @sprite)

		# handle tree show/hide events
		@on 'show', (keys) =>
			_.each keys, (key) -> self.nodes[key].animateShow()

		@on 'hide', (keys) =>
			_.each keys, (key) -> self.nodes[key].animateHide()

		#start loading the tree
		@nodes[""] = new @NodeClass('',this)
		@nodes[""].loadItem()

	updateWithCamera: (camera) ->
		@maxDistanceFromCamera = Math.sqrt( camera.position.lengthSq() - wgs84.earthRadius * wgs84.earthRadius )

		return CullingTree.prototype.updateWithCamera.call(this, camera)

	updateIconsElevations: () ->
		self = this
		_.each @visibleKeys, (key) ->
			self.backgroundTasks.schedule () ->
				node = self.nodes[key]
				if node
					node.updateIconsElevations()

	###
		The hover raycast is wildly inaccurate so we have to be smart with how we select 
		icons to hover. My approach was to cast a wide net with the hover and then
		choose among the hit items by the following criteria:
		 - find the closest icon to the mouseOnGlobe marker
		 - if there is a palette defined, use the property (highest value) to drive selection.
		   this can be useful when you want the largest city to pop out amongst the crowd
	###
	addToHoverSet: (keys) ->

	removeFromHoverSet: (keys) ->

	_loadIconGroup: (key) ->
		updateData = (data) =>
			return if !@nodes[key]
			@nodes[key].setData(data)
			# only load children if we are at root, otherwise use the culling tree
			# to asynchronously load
			if key is ''
				@nodes[key].loadChildren()
				
		success = (data) =>
			return if !@nodes[key]
			@backgroundTasks.schedule () => updateData(data)

		error = (err) =>
			console.log "ERROR ", err

		priority = () =>
			if key and @nodes[key]
				boundingSphere = @nodes[key].boundingSphere
				return 10000 unless boundingSphere
				if @currentCamera
					distanceToCamera = boundingSphere.center.distanceTo(@currentCamera.position)
					return boundingSphere.radius - distanceToCamera
				return 10000
			return 0

		@iconProvider.load key, success, error, priority

materialFactory = (tree, sprite) ->
	matOpts =
		uniforms:
			color:     	{ type: "c", value: new THREE.Color( 0xffffff ) }
			texture:   	{ type: "t", value: sprite }
			#TODO Theme: change to shared uniform
			borderColorAndIconAlpha: {	type: 'v4', value: tree.sharedUniforms.borderColorAndIconAlpha }
		vertexShader: ICON_VS
		fragmentShader: ICON_FS
		transparent: true
		depthTest: false
		side: THREE.DoubleSide

	mat = new THREE.ShaderMaterial( matOpts )
	mat.size  = 1.0
	mat.sizeAttenuation = true

	return mat


###


Data Object:
Count: 15
Id: "F-0100"
Radius: 0.03072488715571305
Values: null
X: -0.25476376571695275
Y: -0.5922995768827827
Z: 0.7643799022097548

###
class IconNode extends CullingNode
	constructor: (@key, @tree, @icon) ->
		CullingNode.call(this, @key, @tree)

		@dataLoaded = false

		# these values are set as instance variables to allow customizeable
		# hover sizing depending on implementation
		@hoverSize = 2.0
		@hoverGroupSize = 1.5

		if @icon
			@visible = false

			style = @tree.layer.style.Icon
			@iconSize = 100.0 * style.Scale

			@tmpVector3 = new THREE.Vector3()
			@tmpColor = new THREE.Color()

			@geometry = new THREE.BufferGeometry()
			@positionsBuffer = new THREE.BufferAttribute( new Float32Array( 3 ), 3 )
			@opacityBuffer = new THREE.BufferAttribute( new Float32Array( 1 ), 1 )
			@sizeBuffer = new THREE.BufferAttribute( new Float32Array( 1 ), 1 )
			@caBuffer = new THREE.BufferAttribute( new Float32Array( 3 ), 3 )

			@geometry.addAttribute('position', @positionsBuffer)
			@geometry.addAttribute('opacity', @opacityBuffer)
			@geometry.addAttribute('size', @sizeBuffer)
			@geometry.addAttribute('ca', @caBuffer)
			
			@boundingSphere = new THREE.Sphere(
				@icon.Center,
				@icon.Radius
			)
			
			elevation = @tree.globe.tree.getLocationElevation(@icon.Center)
			position = @icon.Center.clone().normalize().multiplyScalar(wgs84.earthRadius + elevation);
			@positionsBuffer.array[0] = position.x
			@positionsBuffer.array[1] = position.y
			@positionsBuffer.array[2] = position.z

			@mesh = new THREE.Points( @geometry, materialFactory(@tree, @tree.sprite) )
			@mesh.key = @key
			@tree.root.children.push(@mesh)
			@tree.meshes[@key] = @mesh

			color = @icon.iconColor || new THREE.Color(style.Color)

			@opacityBuffer.array[0] = 0.0
			@sizeBuffer.array[0] = @icon.iconSize || @iconSize
			@caBuffer.array[0] = color.r
			@caBuffer.array[1] = color.g
			@caBuffer.array[2] = color.b

			@pointsCount = 1

	defaultAnimation: () ->
		return {ease: window.Back.easeOut.config(5), onUpdate: () => @markAttributesForUpdate() }

	updateVisibility: (camera) ->
		self = this

		if @boundingSphere
			return [] if @boundingSphere.center.distanceTo(camera.position) - @boundingSphere.radius > @tree.maxDistanceFromCamera

		# call child update
		keys = CullingNode.prototype.updateVisibility.call(this, camera)

		# console.log "Icon Node camera update ", @key
		return keys unless @geometry

		if @visible

			if @boundingSphere.center.distanceTo(camera.position) + @boundingSphere.radius > @tree.maxDistanceFromCamera
				@visibleVerticesCount = 0
				for i in [0...@pointsCount] by 1
					@tmpVector3.set(@positionsBuffer.array[i*3 + 0], @positionsBuffer.array[i*3 + 1], @positionsBuffer.array[i*3 + 2])

					if @tmpVector3.distanceTo(camera.position) < @tree.maxDistanceFromCamera
						@visibleVerticesCount++
						@opacityBuffer.array[i] = 1.0
					else
						@opacityBuffer.array[i] = 0.0

				@markAttributesForUpdate()

			else if @visibleVerticesCount != @pointsCount / 3
				@visibleVerticesCount = @pointsCount / 3
				for i in [0...@pointsCount] by 1
					@opacityBuffer.array[i] = 1.0

				@markAttributesForUpdate()

		return keys

	showBoundingSphere: (color) ->
		return unless @geometry
		return unless @mesh

		color or= 0x22ff00
		bounds = @boundingSphere
		mat = new THREE.MeshBasicMaterial( { color: color, wireframe: true } )
		geo = new THREE.SphereGeometry( bounds.radius, 16, 16 )
		sphere = new THREE.Mesh(geo, mat)
		sphere.position.copy(bounds.center)

		window.GC.add(sphere)

		setTimeout (-> window.GC.remove(sphere)), 500

	# helper function to check if icon index is a group
	isGroupIndex: (index) -> @data && @data.Groups && index < @data.Groups.length
	
	# trigger show animation
	animateShow: ->
		@showBoundingSphere(0x22ff00) if window.GC.debug

		@visible = true
		@visibleVerticesCount = 0

		# check and load children now in the update loop
		@childrenLoaded()

		return unless @geometry

		# console.log "SHOW ", @key

		# if vertices.length is 0 we should show all children
		if @positionsBuffer.array.length is 0
			for child in @children
				child.animateShow()
			return

		@updateIconsElevations()

		opacity_value = @opacityBuffer.array
		size_value = @sizeBuffer.array

		start = @defaultAnimation()
		end = @defaultAnimation()

		for i in [0...@pointsCount] by 1
			# set vertex attribute per vert
			opacity_value[i] = 1.0

			start[i] = 1.0
			end[i] = @iconSize

			# make it bigger if group node
			if @isGroupIndex(i)
				if @children && @children[i] && @children[i].hidden
					end[i] = 0
				else
					end[i] = @iconSize * PARENT_MULTIPLIER
				 
		end.onComplete = () =>
			#verify children are in thier correct state
			for i in [0...@children.length] by 1
				@hideChildIfNeeded(i)

		# animate all the properties at once
		TweenMax.fromTo(size_value, 0.6, start, end)


		@getParent().updateChildVisibility(this, 'show') if @getParent()

	updateIconsElevations: ->
		return unless @data
	
		position_value = @positionsBuffer.array
		idx = 0
		position = new THREE.Vector3()
		foundUpdate = false

		for group, i in @data.Groups
			group.vertIndex = idx

			if !group.CenterRuv || !group.CenterRuv.node
				group.CenterRuv = @tree.globe.tree.getNodeAndUvFromLocation(group.Center)
			else
				group.CenterRuv = @tree.globe.tree.getNodeAndUvFromLocation(group.CenterRuv.rhombusUv)
				
				if !group.CenterRuv || !group.CenterRuv.node
					group.CenterRuv = @tree.globe.tree.getNodeAndUvFromLocation(group.Center)

			if group.CenterRuv
				elevation = @tree.globe.tree.getLocationElevation(group.CenterRuv.rhombusUv)
			else
				elevation = 0

			if (group.Elevation != elevation)
				group.Elevation = elevation
				foundUpdate = true
				position.copy(group.Center)
				position.normalize().multiplyScalar(wgs84.earthRadius + elevation);
				position_value[idx*3 + 0] = position.x
				position_value[idx*3 + 1] = position.y
				position_value[idx*3 + 2] = position.z
			idx++

		for icon, i in @data.Icons
			icon.vertIndex = idx
			
			if !icon.CenterRuv || !icon.CenterRuv.node
				icon.CenterRuv = @tree.globe.tree.getNodeAndUvFromLocation(icon.Center)
			else
				icon.CenterRuv = @tree.globe.tree.getNodeAndUvFromLocation(icon.CenterRuv.rhombusUv)
				
				if !icon.CenterRuv || !icon.CenterRuv.node
					icon.CenterRuv = @tree.globe.tree.getNodeAndUvFromLocation(icon.Center)

			if icon.CenterRuv
				elevation = @tree.globe.tree.getLocationElevation(icon.CenterRuv.rhombusUv)
			else
				elevation = 0
			
			
			if (icon.Elevation != elevation)
				icon.Elevation = elevation
				foundUpdate = true
				position.copy(icon.Center)
				position.normalize().multiplyScalar(wgs84.earthRadius + elevation);
				position_value[idx*3 + 0] = position.x
				position_value[idx*3 + 1] = position.y
				position_value[idx*3 + 2] = position.z

			idx++
		
		@positionsBuffer.needsUpdate = true if foundUpdate
		return

	# trigger hide animation
	animateHide: ->
		@showBoundingSphere(0xff2200) if window.GC.debug

		# if the parent has no visible keys, keep visible
		# parent = @parent()
		# if parent and parent.geo
		# 	return unless parent.geo.vertices.length

		# console.log "HIDE ", @key


		# if the direct children are visible, keep visible
		# otherwise icons disappear too often
		childKeys = _.map @children, (c) -> c.key
		return if _.intersection(childKeys, @tree.visibleKeys).length


		@visible = false

		# if children are in visible keys, it's a zoom in
		# otherwise it's zoom out and we hide
		return unless @geometry


		size_value = @sizeBuffer.array

		# skip animation, just set size directly
		for i in [0...@pointsCount]
			size_value[i] = 0.0

		# start = @defaultAnimation()
		# end = @defaultAnimation()

		# for i in [0...@pointsCount]
		# 	end[i] = 0.0
		# 	start[i] = @iconSize
		# 	# make it bigger if group node
		# 	end[i] = @iconSize * PARENT_MULTIPLIER if @isGroupIndex(i)

		# # animate all the properties at once
		# TweenMax.fromTo(size_value, 0.6, start, end)

		
		@getParent().updateChildVisibility(this, 'hide') if @getParent()
		@markAttributesForUpdate()


	hideChild: (index) ->
		return unless @geometry

		size_value = @sizeBuffer.array
		size_value[index] = 0.0

		# no longer animate the hide operation
		# start = @defaultAnimation()
		# end = @defaultAnimation()

		# start[index] = @iconSize
		# start[index] = @iconSize * PARENT_MULTIPLIER if @isGroupIndex(index)
		# end[index] = 0.0

		# # animate all the properties at once
		# TweenMax.fromTo(size_value, 0.6, start, end)

		# if the child node is the hover node reset hover
		iconHover = @tree.globe.iconHover
		if iconHover and iconHover.key is @key and iconHover.vertexIndex is index
			@tree.globe.setHover() # sets hover state to null

		@markAttributesForUpdate()

	# make sure child respecit it hidden flag
	hideChildIfNeeded: (index) ->
		if @isGroupIndex(index) && @children && @children[index] && @children[index].hidden
			size_value = @sizeBuffer.array
			size_value[index] = 0
			@markAttributesForUpdate()
			return true
		return false

	showChild: (index) ->
		return unless @geometry

		size_value = @sizeBuffer.array

		start = @defaultAnimation()
		end = @defaultAnimation()

		start[index] = 0.0
		end[index] = @iconSize
		end[index] = @iconSize * PARENT_MULTIPLIER if @isGroupIndex(index)
		end.onComplete = () => @hideChildIfNeeded(index)

		# animate all the properties at once
		TweenMax.fromTo(size_value, 0.6, start, end)
		@markAttributesForUpdate()



	hoverStart: (index) ->
		# the object to animate
		size_value = @sizeBuffer.array

		return if size_value[index] is 0
		
		# this is a group 
		if @isGroupIndex(index)
			return if @hideChildIfNeeded(index)
			
			# no animation on aggregate hover
			size_value[index] = @iconSize * @hoverGroupSize * PARENT_MULTIPLIER
			@markAttributesForUpdate()
		else
			end = @defaultAnimation()
			end[index] = @iconSize * @hoverSize
			end.onComplete = () => @hideChildIfNeeded(index)

			TweenMax.to(size_value, 0.22, end)



	hoverEnd: (index) ->
		# the object to animate
		size_value = @sizeBuffer.array

		return if size_value[index] is 0
		return if @hideChildIfNeeded(index)

		end = @defaultAnimation()
		end[index] = @iconSize
		end[index] = @iconSize * PARENT_MULTIPLIER if @isGroupIndex(index)
		end.onComplete = () => @hideChildIfNeeded(index)

		TweenMax.to(size_value, 0.22, end)


	# naive check to see if size is zero
	isVisible: (index) ->
		size_value = @sizeBuffer.array
		return false if size_value[index] is 0
		return true


	# since this node contains the group that represent
	# the children, update based on state. this gets called
	# from child animateShow/animateHide
	updateChildVisibility: (child, childAction) ->
		return unless @data
		return unless @mesh

		opacity_values = @opacityBuffer.array

		for group, i in @data.Groups
			if @children[i].key is child.key
				if childAction is 'show'
					@children[i].hidden = true
					@hideChild(i)
				else
					@children[i].hidden = false
					@showChild(i)

	# disabled loadItem in favor of loading children when visible
	childrenLoaded: ->
		return false unless @data

		if @children.length is @data.Groups.length
			return true
		else
			@loadChildren()
			return false


	loadChildren: ->
		self = this

		unless @data
			@loadItem()
			return

		@children = []

		for group, i in @data.Groups
			if @tree.nodes[group.Id]
				node = @tree.nodes[group.Id]
				@children.push node
				node.parent = self
			else
				node = new @tree.NodeClass(group.Id, self.tree, group)
				@tree.nodes[group.Id] = node
				@children.push node
				node.parent = self

		for child in @children
			unless child.dataLoaded
				self.tree._loadIconGroup(child.key)

		return

	# only time we want to load
	loadItem: ->
		return if @data # and @data.Groups.length is @children.length
		@tree._loadIconGroup(@key)


	setData: (@data) ->
		@dataLoaded = true

		return unless @geometry
		return unless @data.Icons.length or @data.Groups.length

		@tree.root.remove(@mesh)
		@pointsCount = _.size(@data.Groups) + _.size(@data.Icons)

		# we need to create a new mesh entirely after the new vertex data comes in

		@geometry = new THREE.BufferGeometry()
		@positionsBuffer = new THREE.BufferAttribute( new Float32Array( @pointsCount * 3 ), 3 )
		@opacityBuffer = new THREE.BufferAttribute( new Float32Array( @pointsCount * 1 ), 1 )
		@sizeBuffer = new THREE.BufferAttribute( new Float32Array( @pointsCount * 1 ), 1 )
		@caBuffer = new THREE.BufferAttribute( new Float32Array( @pointsCount * 3 ), 3 )

		@geometry.addAttribute('position', @positionsBuffer)
		@geometry.addAttribute('opacity', @opacityBuffer)
		@geometry.addAttribute('size', @sizeBuffer)
		@geometry.addAttribute('ca', @caBuffer)

		@updateIconsElevations()
		
		@mesh = new THREE.Points( @geometry, materialFactory(@tree, @tree.sprite) )

		@mesh.key = @key
		@tree.root.children.push(@mesh)
		@tree.meshes[@key] = @mesh

		@geometry.verticesNeedUpdate = true


		# since the vertex count changed we have to recreate the material
		# otherwise the attributes never move past 1
		# @mesh.material = materialFactory(@tree.sprite)

		style = @tree.layer.style.Icon
		palette = @tree.palette
		attributes = @mesh.material.attributes
		groupValues = _.map @data.Groups, (group) -> group.Values
		iconValues = _.map @data.Icons, (icon) -> icon.Values
		allValues = _.union(groupValues, iconValues)

		# try setting attributes after vertices attached
		for i in [0...@pointsCount]
			# set vertex attribute per vert
			@opacityBuffer.array[i] = 0.0
			@sizeBuffer.array[i] = @iconSize

			color = style.Color || palette.evaluate(undefined, {hexString: true})

			# treat group nodes differently from individual
			if @isGroupIndex(i)
				@sizeBuffer.array[i] = @iconSize * PARENT_MULTIPLIER

				if style.Style is 'Palette' and style.PaletteExpression and allValues[i]
					valueObject = allValues[i][style.PaletteExpression]
					value = undefined
					if valueObject
						if valueObject.Values
							totalCount = 0
							for value,count of valueObject.Values
								if totalCount > 0
									weight = (totalCount+0.0) / (totalCount+count)
									color.mix(palette.evaluate(value),100 * weight)
								else
									color = palette.evaluate(value)
								totalCount += count
							color = color.hexString()
						else if valueObject.Min
							value = valueObject.Min # for now just use alphabetical lowest
							color = palette.evaluate(value, {hexString: true})
						else
							value = valueObject.Average
							color = palette.evaluate(value, {hexString: true})
						
				@tmpColor.set(color)

				@caBuffer.array[i*3 + 0] = @tmpColor.r
				@caBuffer.array[i*3 + 1] = @tmpColor.g
				@caBuffer.array[i*3 + 2] = @tmpColor.b

				#store group color for future referene
				@data.Groups[i].iconColor = @tmpColor.clone()
				@data.Groups[i].iconSize = @sizeBuffer.array[i]
			else

				if style.Style is 'Palette' and style.PaletteExpression and allValues[i]
					value = allValues[i][style.PaletteExpression]
					color = palette.evaluate(value, {hexString: true})

				@tmpColor.set(color)

				@caBuffer.array[i*3 + 0] = @tmpColor.r
				@caBuffer.array[i*3 + 1] = @tmpColor.g
				@caBuffer.array[i*3 + 2] = @tmpColor.b

		@markAttributesForUpdate()

		if @tree.visibleKeys and @key in @tree.visibleKeys
			@animateShow()

		@tree.invalidateVisibilityList()


	###
		Allow fetching a data value from both data.Groups and data.Icons by the vertex
		index.
	###
	getDataValueByIndex: (index) ->
		return null unless @data

		if @isGroupIndex(index)
			return @data.Groups[index]
		else
			return @data.Icons[index - @data.Groups.length]


	markAttributesForUpdate: (forceResize) ->
		#set material.needsUpdate = true will cause three.js to dispose and reinit the material.
		#we only need to noitfy the program about update the opacity/size/ca
		#@mesh.material.needsUpdate = true
		@opacityBuffer.needsUpdate = true
		@sizeBuffer.needsUpdate = true
		@caBuffer.needsUpdate = true


	###
		Expose data for HoverState, including screen position and xyz position
	###
	getHoverData: (index) ->
		return {} unless @data

		canvas = @tree.globe.canvas
		vertexData = @getDataValueByIndex(index)
		position = new THREE.Vector3(@positionsBuffer.array[index*3], @positionsBuffer.array[index*3 + 1], @positionsBuffer.array[index*3 + 2])
		clientPos = canvas.xyzToScreenCoordinates(position)
		color = Color().rgb(@caBuffer.array[index*3]*255, @caBuffer.array[index*3 + 1]*255, @caBuffer.array[index*3 + 2]*255)


		data = {
			key: @key
			index: index
			icon: true
			isGroupIcon: @isGroupIndex(index)
			geoSource: @data.GeoSource
			field: @tree.displayField
			value: vertexData.Values[@tree.displayField]
			data: vertexData
			position: position
			latlon: wgs84.xyzToLatLon(position)
			clientX: clientPos.x 
			clientY: clientPos.y
			color: color.hexString()
			borderColor: canvas.theme.iconBorderColor
		}

		return data



	###
		Is this node's radius at least a 1/3 distance to the center of the node
	###
	screenSize: (camera) ->
		return 0.0 if @boundingSphere is undefined

		fov = camera.fov/ 2 * Math.PI / 180.0

		# r = 40/(Math.pow(3,this.key.length-2))  # this needs to change
		r = @boundingSphere.radius
		d = camera.position.distanceTo(@boundingSphere.center)
		sz = r * ICON_CUTOFF_MULTIPLIER / d

		@sz = sz
		return sz


	isActive: -> @tree.visibleKeys.indexOf(@key) isnt -1
	isLoading: -> !@dataLoaded
	isLoaded: -> @dataLoaded



window.testIconAngle = (key) ->
	camera = window.GC.getCamera()
	mesh = window.GC.globe.iconTrees[0].meshes[key]

	# use boundingsphere position
	iconVector = mesh.geometry.boundingSphere.center
	cameraVector = (new THREE.Vector3( 0, 0, -1 )).applyQuaternion(camera.quaternion )

	console.log "ANGLE ", iconVector.angleTo( cameraVector ), iconVector, cameraVector

	if iconVector.angleTo( cameraVector ) > Math.PI / 2
		mesh.visible = true
	else
		mesh.visible = false


occasionalLog = -> console.log.apply(console, arguments)
occasionalLog = _.throttle(occasionalLog, 1000) # only call every second

window.logNodeHierarchy = (node, spacer) ->
	spacer or= ''
	console.log spacer, node.key
	_.each node.children, (child) ->
		window.logNodeHierarchy(child, spacer + ' ')



window.testScreenSizeCalc = (node) ->
	camera = window.GC.getCamera()
	console.log "SCREENSIZE ", node.screenSize(camera)






###
Example Icon Style object:
{
"Fill":
	{"Style":"SolidColor",
	"Color":"rgba(127,127,255,0.60)"},
"Icon":{"IconDataUrl":"data:image/png;base64,..",
	"Scale":0.2,
	"PaletteExpression":"RANK_MAX",
	"Style":"Palette",
	"Palette":{
		"Steps":[{"Value":0,"Color":"#FEF0D9"}, .. ]},
	"Color":"rgba(0,0,0,0.00)"}
}
###




module.exports =
	IconTree: IconTree
	IconNode: IconNode
