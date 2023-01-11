###
	Specify text variations of the IconTree

	TextHoverTree is a simple variation that provides text popups on Icon hover

	TextTree replaces Icons with Text
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
glslify = require 'glslify'
EventEmitter = require 'event-emitter'
Color = require('color')

IconTree = require('./icon').IconTree
IconNode = require('./icon').IconNode
CullingNode = require('./cull').CullingNode

fontTexture = require('../shaders/text/font-texture')

###
	Hook into the IconNode and override hover methods to  
	display text on the globe 

	TODO :: we need UI to select the text display field  
###
class TextHoverTree extends IconTree
	constructor: (@globe, @layer, @root, @backgroundTasks) ->
		@NodeClass = TextHoverNode
		super(@globe, @layer, @root, @backgroundTasks)

class TextHoverNode extends IconNode
	constructor: (@key, @tree, @icon) ->
		super(@key, @tree, @icon)

		# use different hover size for these nodes
		@hoverGroupSize = 2.0

		@_hoverStart = @hoverStart
		@_hoverEnd = @hoverEnd

		@hoverStart = (index) ->
			if @tree.displayField
				attributes = @mesh.material.attributes
				groupValues = _.map @data.Groups, (group) -> group.Values
				iconValues = _.map @data.Icons, (icon) -> icon.Values
				allValues = _.union(groupValues, iconValues)

				text = allValues[index][@tree.displayField]
				if _.isObject(text)
					groupData = @getDataValueByIndex(index)
					text = "(#{groupData.Count})"
					
				# crop text if longer than 75 chars
				if text and text.length > 75
					text = text[0..75] + '...'

				positionsArray = @geometry.attributes.position.array
				position = new THREE.Vector3(positionsArray[index*3 + 0], positionsArray[index*3 + 1], positionsArray[index*3 + 2])

				scaleRatio = GC.scene.cameraController.rangeRatio
				scaleRatio *= 1200 / GC.container.offsetHeight
				@textGroup = fontTexture.createBillboard(
						window.GC.scene, 
						text, 
						position, 
						{scale: 1.2 * scaleRatio})
				# @textGroup = window.drawText(text, @geometry.vertices[index], {scale: 1.1 * scaleRatio})

				# TODO :: read camera position to determine animation angle
				fromPos = @textGroup.position.y - 300000.0 * scaleRatio
				TweenMax.from(@textGroup.position, 0.25, {y: fromPos, easing: Quad.easeOut})

			this._hoverStart(index)

		@hoverEnd = (index) ->
			if @textGroup
				window.GC.scene.threeScene.remove(@textGroup)
				# TweenMax.fromTo(size_value, 0.6, start, end)
				@textGroup = null

			this._hoverEnd(index)







###
	Hook into the IconNode and override hover methods to  
	display text on the globe 

	TODO :: we need UI to select the text display field  
###
class TextBillboardTree extends IconTree
	constructor: (@globe, @layer, @root) ->
		@NodeClass = TextBillboardNode
		super(@globe, @layer, @root)

		
		@identifyTextField()



class TextBillboardNode extends IconNode
	constructor: (@key, @tree, @icon) ->
		CullingNode.call(this, @key, @tree)

		@dataLoaded = false

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

			elevation = @tree.globe.tree.getLocationElevation(@icon.Center)
			@icon.Center.normalize().multiplyScalar(wgs84.earthRadius + elevation);
			@positionsBuffer.array[0] = @icon.Center.x
			@positionsBuffer.array[1] = @icon.Center.y
			@positionsBuffer.array[2] = @icon.Center.z

			@mesh = new THREE.Points( @geometry, materialFactory(@tree.sprite) )
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

			# @markAttributesForUpdate()

			@boundingSphere = new THREE.Sphere(
				@icon.Center,
				@icon.Radius
			)

	# trigger show animation
	animateShow: ->
		console.log "animate show"


	# trigger hide animation
	animateHide: ->
		console.log "animate hide"


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

		@mesh = new THREE.Points( @geometry, materialFactory(@tree.sprite) )
		@mesh.key = @key
		@tree.root.children.push(@mesh)
		@tree.meshes[@key] = @mesh


		@geometry.verticesNeedUpdate = true


		# since the vertex count changed we have to recreate the material
		# otherwise the attributes never move past 1
		# @mesh.material = materialFactory(@tree.sprite)

		style = @tree.layer.style.Icon
		palette = @tree.palette
		groupValues = _.map @data.Groups, (group) -> group.Values
		iconValues = _.map @data.Icons, (icon) -> icon.Values
		allValues = _.union(groupValues, iconValues)

		# try setting attributes after vertices attached
		for i in [0...@pointsCount]
			# set vertex attribute per vert
			@opacityBuffer.array[i] = 0.0
			@sizeBuffer.array[i] = @iconSize

			color = style.Color

			# treat group nodes differently from individual
			if i < @data.Groups.length
				@sizeBuffer.array[i] = @iconSize * PARENT_MULTIPLIER
				# TODO : set color on group nodes

				if style.Style is 'Palette' and style.PaletteExpression and allValues[i]
					valueObject = allValues[i][style.PaletteExpression]
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




module.exports =
	TextHoverTree: TextHoverTree
	TextHoverNode: TextHoverNode
	TextBillboardTree: TextBillboardTree
	TextBillboardNode: TextBillboardNode
