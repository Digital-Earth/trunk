###
	Asset Loading Utilities

	currently using $.ajax for data and a custom THREE image loader
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
EventEmitter = require 'event-emitter'

# utility to check if listeners exist
hasListeners = require('event-emitter/has-listeners');




class PYX0Format
	constructor: (@buffer) ->
		@magic = 811096400 #ascii of PYX0

	createTexture: (range) ->
		uint32Buffer = new Uint32Array(@buffer,0)

		if uint32Buffer[0] != @magic
			return null

		size = uint32Buffer[1]
		binCount = uint32Buffer[2]
		offsetSize = uint32Buffer[3]
		bins = new Float32Array(@buffer,16)
		valueBin = new Uint8Array(@buffer,16 + 4*binCount)
		valueBinOffset = new Uint8Array(@buffer,16 + 4*binCount + size*size)

		floats = new Float32Array(size*size)

		for index in [0...size*size] by 1
			if valueBin[index] == 0 && valueBinOffset[index] == 0
				#mark as no value
				floats[index] = -1
			else
				bin = valueBin[index] - 1
				if (valueBinOffset[index] == 0)
					value = bins[bin]
				else
					value = bins[bin] + (bins[bin+1] - bins[bin]) * valueBinOffset[index] / offsetSize

				#normalize value from range into 0...1
				if value < range[0]
					floats[index] = 0
				else if value > range[1]
					floats[index] = 1
				else
					floats[index] = (value-range[0]) / (range[1] - range[0])

		if window.glSupport.floatTextures
			texture = new THREE.DataTexture( floats, size, size, THREE.LuminanceFormat, THREE.FloatType )
		else
			rgba = new Uint8Array(size*size*4)

			for index in [0...size*size] by 1
				if floats[index] == -1
					rgba[index*4+3] = 0
				else
					value = Math.floor(floats[index]*(256*256*256-1))
					rgba[index*4+0] = value / 256 / 256;
					rgba[index*4+1] = value / 256 % 256;
					rgba[index*4+2] = value % 256;
					rgba[index*4+3] = 255

			texture = new THREE.DataTexture( rgba, size, size, THREE.RGBAFormat )

		texture.needsUpdate = true

		return texture


class TextureLoader
	constructor: (options) ->
		defaults =
			maxRequests: 3 # only 3 requests at a time
			sortFunction: (requests) -> _.sortBy(requests,(request)->request.priority())
			manualSort : false
		options or= {}
		@options = _.extend defaults, options

		@requests = []
		@requestsRunning = 0
		@enabled = true  # use to enable/disable globally
		@sleeping = false # use to prevent handling new requests

		# setup object as EventEmitter
		EventEmitter(this)

		# set loadAsset handler default to loadImage
		# subclasses can override this for data assets
		@loadAsset = @loadImage

	requestsCount: () ->
		return @requests.length

	requestsInProgress: () ->
		return @requestsRunning

	# this is the public interface to queue a request
	load: (url, requestObject) ->
		self = this

		# handle case where url argument is ommitted
		if _.isObject(url)
			requestObject = url
			url = requestObject.url

		request =
			url: url
			success: () -> null
			error: () -> null
			enabled: () -> self.enabled
			priority: () -> 1

		_.extend request, requestObject

		@requests.push request

		@doRequest()

	###*
	 * reorder the request so we would request the most needed resource first
	 *
	 * however, this can be CPU instestive - and it doesn't change every frame,
	 * so this function will throttle every 500[ms]
	 *
	 * @return {nothing}
	###
	_refreshRequestOrder: () ->
		self = this
		@_refreshRequestOrder = _.throttle(
			() => @requests = @options.sortFunction(@requests),
			500)

	sort: () ->
		@requests = @options.sortFunction(@requests)


	sleep: ->
		@sleeping = true

	wake: ->
		@sleeping = false
		@sort()  # force a sort operation and then start requests
		@doRequest()



	doRequest: ->
		if (!@requests.length)
			return

		if (@requestsRunning >= @options.maxRequests)
			return

		return if @sleeping

		if not @options.manualSort and @requests.length > 10
			@_refreshRequestOrder()

		request = @requests.pop()

		#skip all request that are not enabled any more
		while !request.enabled()
			request.error()

			return unless @requests.length
			request = @requests.pop()

		@requestsRunning++

		# this will recursively pop request stack until we hit
		# maxRequests -- no need for a loop
		if @requestsRunning < @options.maxRequests
			@doRequest()

		@loadAsset(
			request,
			() =>
				@requestsRunning--
				# use apply in case the handler takes multiple arguments
				request.success.apply(request, arguments)
				@doRequest()
			() =>
				@requestsRunning--
				request.error()
				@doRequest()
			)



	###*
	 * load an image from a give url, and make sure we remove all event lister to avoid memory leaks
	 *
	 * Note: we are not using THREE.ImageLoader as it leak reference at the moment.
	 *
	 * @param  {object} request     request object. request.url is usually used to load
	 * @param  {function(texture)} onSuccess a function to invoke when image was loaded ok
	 * @param  {function} onError   a function to invoke when the image had failed to load
	 * @return {img}           the image object created by the function
	###
	loadImage: (request, onSuccess, onError) ->
		if @options.backgroundTasks
			run = @options.backgroundTasks.schedule
		else
			run = (callback) -> callback()

		if request.format == "PYX0"
			xhr = new XMLHttpRequest()
			xhr.open('GET', request.url, true)
			xhr.responseType = 'arraybuffer'

			processImage = () ->
				try
					texture = new PYX0Format(xhr.response).createTexture(request.range)
					if texture
						texture.needsUpdate = true;
						onSuccess(texture)
					else
						onError()
				catch e
					onError(e)
			
			loadHandler = () ->
				cleanHandlers()
				run(processImage)
				
			errorHandler = () ->
				cleanHandlers()
				onError()

			cleanHandlers = () ->
				xhr.removeEventListener('load', loadHandler, false)
				xhr.removeEventListener('error', errorHandler, false)

			xhr.addEventListener('load', loadHandler, false)
			xhr.addEventListener('error', errorHandler, false)

			xhr.send()
			return xhr

		else
			image = document.createElement( 'img' )
			
			processImage = () ->
				try
					texture = new THREE.Texture( image, THREE.UVMapping)
					texture.needsUpdate = true;
					onSuccess(texture)
				catch e
					onError(e)

			loadHandler = () ->
				cleanHandlers()
				run(processImage)

			errorHandler = () ->
				cleanHandlers()
				onError()

			cleanHandlers = () ->
				image.removeEventListener('load', loadHandler, false)
				image.removeEventListener('error', errorHandler, false)

			image.addEventListener('load', loadHandler, false)
			image.addEventListener('error', errorHandler, false)

			image.crossOrigin = 'Anonymous' #''
			image.src = request.url

			return image




class DataLoader extends TextureLoader
	constructor: (options) ->
		super(options)
		@loadAsset = @loadData


	###*
	 * load generic data -- simply pass through to handlers
	###
	loadData: (request, onSuccess, onError) ->
		# console.log("load data", url)
		$.get(request.url)
			.done(onSuccess)
			.fail(onError)




###
	This class is intended to be a global layer for providing loading status to a
	LoadFeedbackView model.

	Takes in a reference to canvas and delegates loading events to it.
###
class LoadFeedbackService
	constructor: (globeCanvas, options) ->
		self = this

		# we need to keep references around to handlers in order to turn them
		# off individually
		@handlers = {}

		# state is an object to track
		@resetState = =>
			@state =
				requests: 0
				requestsRunning: 0
				startTime: new Date().getTime()
				totalRequests: 0

		@resetState()

		# use this method to ensure that we remove old handlers before binding new ones
		# handlerName is optional otherwise we use the eventName for both
		@applyHandler = (eventName, handlerName, handler) =>
			if _.isFunction(handlerName)
				handler = handlerName
				handlerName = eventName

			if hasListeners(globeCanvas, eventName)
				globeCanvas.off eventName, @handlers[handlerName]

			@handlers[handlerName] = handler
			globeCanvas.on eventName, @handlers[handlerName]


		# hide the canvas until appLoad
		TweenMax.set globeCanvas.container, {opacity: 0}
		globeCanvas.emit 'loadInit'


		@applyHandler 'ready', ->
			TweenMax.to globeCanvas.container, 0.4, {opacity: 1, easing: Quad.easeOut}


		@applyHandler 'preloadLayers', =>
			@resetState()
			#globeCanvas.emit 'showLoader'

		@applyHandler 'preloadLayersComplete', =>
			globeCanvas.emit 'hideTitle'

		@title = ''
		@applyHandler 'preloadTitle', (newTitle) =>
			return if @title is newTitle

			@title = newTitle
			globeCanvas.emit 'showTitle', newTitle


		# load status interval, we check every second to see if
		# we are in a high load state and then trigger the loader to show
		# checkLoadingOnNext allows us to wait a full interval before showing loader
		@loadingActive = false
		@checkLoadingOnNext = false
		@loaderVisible = false

		checkLoadStatus = =>
			newState = globeCanvas.getLoadingStatus()
			_.extend @state, newState

			# console.log "loading ", @state.total, @state.requests
			# @state.totalRequests = Math.max(@state.totalRequests, @state.requests)

			if @loadingActive
				if @state.requests is 0
					@loadingActive = false
					globeCanvas.emit 'hideLoader'
					@loaderVisible = false
					@resetState()
				else
					globeCanvas.emit 'loadUpdate', @state
			else
				if @state.requests > 0
					if @checkLoadingOnNext
						globeCanvas.emit 'showLoader'
						@loadingActive = true
						@loaderVisible = true
						@checkLoadingOnNext = false
					else
						@checkLoadingOnNext = true
				else
					@checkLoadingOnNext = false
					if @loaderVisible
						globeCanvas.emit 'hideLoader'
						@loaderVisible = false
						@resetState()


		#changed from setInterval to put into canvas.update method
		@update = _.throttle(checkLoadStatus, 500)









###
	This class provides a default view implementation of the loading feedback.
	It can be left to the angular app to decide how to display the load bar etc. but
	this is a default implementation that can be used outside of app context.

	It will typically be instantiated in the globe-interface.js with:
	new $window.PYXIS.LoadFeedbackView(this.globe)
###
class LoadFeedbackView
	constructor: (globeCanvas, options) ->
		self = this

		@titleShownOnce = false
		@loadTitle = $('.loading-title')
		@loadBarContainer = $('.loading-bar')
		@loadBar = $('.loading-bar > span')
		@progress = 20  # percentage loaded

		globeCanvas.on 'ready', =>
			TweenMax.to @loadBarContainer, 0.8, {opacity: 0, easing: Quad.easeIn}

			TweenMax.to @loadTitle, 0.8, {y: '+50', opacity: 0, easing: Quad.easeIn, delay: 1.5}


		globeCanvas.on 'loadUpdate', (state) =>
			# progressVal = ((state.totalRequests - state.requests) / state.totalRequests) * 100
			progressVal = ((state.total - state.requests) / state.total) * 100
			# console.log("load update ", progressVal, state.total, state.requests
			TweenMax.to @loadBar, 0.4, {width: "#{progressVal}%"}

		globeCanvas.on 'showLoader', =>
			@loadBar.css('width', "0%")
			@loadBarContainer.show()
			TweenMax.to @loadBarContainer, 1.0, {opacity: 1, easing: Quad.easeOut}

		globeCanvas.on 'hideLoader', =>
			@loadBar.css('width', "100%")
			TweenMax.to @loadBarContainer, 0.8, {
				opacity: 0,
				easing: Quad.easeIn
				onComplete: -> self.loadBarContainer.hide()
			}

		@titleVisible = false

		hideTitle = (title) =>

			#if we given a title - make sure it still the same title
			if title && @loadTitle.text() != title
				return

			if @titleVisible
				@titleVisible = false

				TweenMax.to @loadTitle, 0.8, {
					y: '+50',
					opacity: 0,
					easing: Quad.easeOut,
					delay: 0.5
					onComplete: -> self.loadTitle.hide()
				}

		showTitle = (title) =>
			unless @titleShownOnce
				@titleShownOnce = true
				return

			@titleVisible = true
			@loadTitle.text(title).show()
			TweenMax.fromTo( @loadTitle, 0.8,
				{y: '-50', opacity: 0 },
				{y: '0', opacity: 1, easing: Quad.easeOut})

		globeCanvas.on 'showTitle', (title) =>
			showTitle(title)
			#make sure we are hidnig the title after 5 sec
			_.delay(hideTitle, 5000, title)

		globeCanvas.on 'hideTitle', =>
			hideTitle()







# debug tool
window.getPriorities = ->
	priorityMap = {}

	_.each window.GC.globe.tree.visibleKeys, (key) ->
		r = GC.globe.tree.nodes[key]
		priorityMap[key] = r.getGeoSourcePriority()

	return priorityMap


# enable debug shader to show loading priorities
window.debugLoading = ->

	update = ->
		priorityMap = window.getPriorities()
		meshes = window.GC.globe.tree.meshes

		_.each priorityMap, (priority, key) ->
			mesh = meshes[key]
			mesh.material.uniforms['debugLoadingEnabled_1'].value = 1.0
			mesh.material.uniforms['debugLoadingAmount_1'].value = (priority / 100.0)

	window.debugInterval = setInterval(update, 500)

window.disableDebugLoading = ->
	clearInterval(window.debugInterval)





module.exports =
	TextureLoader: TextureLoader
	DataLoader: DataLoader
	LoadFeedbackService: LoadFeedbackService
	LoadFeedbackView: LoadFeedbackView

