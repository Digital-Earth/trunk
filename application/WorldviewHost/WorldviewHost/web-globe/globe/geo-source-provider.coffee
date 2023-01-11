###
	This is a coffeescript file that compiles-to-javascript.

	GeoSourceProvider provides logic for loading and processing texture information
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
TextureLoader = require('../utilities/loader').TextureLoader
RhombusHierarchy = require('./tree/hierarchy').RhombusHierarchy

#generate unique hash string based on geosource GUID and supplied options
geoSourceHash = (geoSource, options) ->
	hash = geoSource
	if options.range
		hash+="(float)min=#{options.range[0]}&max=#{options.range[1]}"
	else if options.style
		hash+="(style)" + JSON.stringify(options.style)
	return hash

class GeoSourceProviderItem
	constructor: (@parent, @name, @index, @options) ->
		@keys = {}
		@hash = geoSourceHash(@name, @options)
		@textureLoader = new TextureLoader(@parent.textureLoaderOptions)

	#
	# 	* returns THREE.Texture for specified key and options
	# 	*
	# 	* key - rhombus index
	# 	*
	# 	* loaderCallbacks - callbacks for TextureLoader, see TextureLoader for details
	getData: (key, loaderCallbacks) ->
		self = this

		if key of @keys
			record = @keys[key]

			if record.disposeTime
				@parent._recoverFromDisposeList(record)

			if record.data
				#console.log "Pickup from cache", @name, key
				@_processData(record.data, key, loaderCallbacks)
			else
				#console.log "Join requests ", @name, key
				record.notifyCallbacks.push loaderCallbacks
			return
		#TODO We need move format into getGeosource function
		@options.format = loaderCallbacks.format
		url = @parent._getUrl(key, @name, @options)

		#add record to the cache
		@_cacheNetworkData(key, null)
		record = @keys[key]
		record.notifyCallbacks.push loaderCallbacks

		request = {
			key: key,
			loaderCallbacks: loaderCallbacks,
			success: (texture) ->
				record = self.keys[this.key]
				if record
					record.data = texture
					for i in [0...record.notifyCallbacks.length]
						self._processData(texture, this.key, record.notifyCallbacks[i])
					record.notifyCallbacks = []

			error: () ->
				record = self.keys[this.key]
				if record
					for i in [0...record.notifyCallbacks.length]
						callbacks = record.notifyCallbacks[i]
						callbacks.error() if callbacks["error"]
					record.notifyCallbacks = []

					delete self.keys[record.key]

			enabled: () ->
				#check if one at least is enabled
				record = self.keys[this.key]
				if record
					for i in [0...record.notifyCallbacks.length]
						callbacks = record.notifyCallbacks[i]
						if callbacks["enabled"] and callbacks.enabled()
							return true
				return false

			priority: () ->
				#find max priority for all requests
				record = self.keys[this.key]
				maxPriority = 0

				if !record or record.notifyCallbacks.length == 0
					return maxPriority

				maxPriority = record.notifyCallbacks[0].priority() if record.notifyCallbacks[0]["priority"]
				for i in [1...record.notifyCallbacks.length]
					callbacks = record.notifyCallbacks[i]
					if callbacks["priority"]
						priority = callbacks.priority()
						maxPriority = priority if priority > maxPriority
				return maxPriority

			format: loaderCallbacks["format"]
			range: loaderCallbacks["range"]
		}
		@textureLoader.load(url, request)

	#
	# 	* Notify GeosourceProvider about texture release
	# 	*
	# 	* key - rhombus index for texture
	# 	* data - texture
	releaseData: (key, data) ->

		record = @keys[key]
		if record
			#console.log "Release ref", @name, key
			record.refs--
			if record.refs <= 0
				@parent._putInLazyDeleteList(record)
			return

		throw "GeoSourceProviderItem::releaseData: Can't find data"

	#
	# 	* returns object with cached values
	#	*
	#	* depth - cache recursion depth, for example depth:3 will cache keys [0..9] + [00..99] + [000...999]
	cacheGeosource: (depth) ->
		self = this

		cacheBucket = { geosourceID: @index, depth: depth, disposed: false, data:[] }
		loadingQueue = []

		hierarchy = @parent.hierarchy

		fillqueue = (parent, d) ->
			if d == 0
				return
			childs = hierarchy.children(parent)
			for idx in childs
				loadingQueue.push idx
			for idx in childs
				fillqueue idx, d - 1
		fillqueue("", depth)
		for idx in loadingQueue
			@getData idx,
				{
					idx: idx,
					cacheBucket: cacheBucket,
					success: (texture) ->
						if cacheBucket.disposed
							self.releaseData(this.idx, texture)
						else
							cacheBucket.data.push([this.idx, texture])
					error: () ->
						console.log "GeoSourceProviderItem::cacheGeosource Failed to cache:", this.cacheBucket.geosourceID, "key:", this.idx
					enabled: () ->
						return not this.cacheBucket.disposed
					priority: () ->
						return 0
					format: self.options.format,
					range: self.options.range,
				}
		return cacheBucket
	#
	# 	* free cached values with cacheGeosource function
	# 	*
	# 	* cacheBucket - object returned by cacheGeosource function
	freeGeosourceCache: (cacheBucket) ->
		cacheBucket.disposed = true
		for rec in cacheBucket.data
			@releaseData(rec[0], rec[1] )
		cacheBucket.data = []

	_cacheNetworkData: (key, data) ->

		#check if we already have data for this key
		if key of @keys
			return @keys[key].data

		record = {
			geosourceID: @index,
			key: key,
			data: data,
			refs: 0,
			notifyCallbacks: []
		}
		@keys[key] = record

		#console.log "cache", @name, key
		return data

	_processData: (data, key, loaderCallbacks) ->
		self = this

		record = @keys[key]
		record.refs++

		if record.refs > 0 and record.disposeTime
			@parent._recoverFromDisposeList(record)

		loaderCallbacks.success(data) if loaderCallbacks["success"]

class GeoSourceProvider
	constructor: (@backgroundTasks) ->
		@geoSources = []

		@defaultSize = 244
		@disposeDelay = 10.0 #in seconds

		@textureLoaderOptions = {
			maxRequests: 3,
			manualSort: false,
			backgroundTasks: @backgroundTasks
		}

		@lazyDisposeList = []

		@hierarchy = new RhombusHierarchy()

	update: () ->
		currentTime = Date.now()

		#check dispose list
		if @lazyDisposeList.length > 0
			@_checkLazyDeleteList(currentTime)

	#
	# 	* returns ID for the Geosource
	# 	*
	# 	* geoSource - geoSource GUID
	# 	* options.range - max and minimum of the geoSource
	# 	* options.style - style to use (when requesting vectors)
	# 	* options.format - format of the image
	getGeoSource: (geoSource, options) ->
		hash = geoSourceHash(geoSource, options)
		#TODO: Maybe better to search from the end
		for i in [0...@geoSources.length]
			id = @geoSources[i]
			if id.hash == hash
				return id
			#erter

		item = new GeoSourceProviderItem(this, geoSource, @geoSources.length, options )
		#TODO maybe put part of the URL here, to optimize instructions count on calling getData function
		console.log "Creating geoSource", item
		@geoSources.push(item)
		return item
	#
	# 	* returns current loading state of geosourceID list
	# 	*
	# 	* geosourceIDs - array of geosourceIDs
	isGeosourceListIsLoading: (geosources) ->
		for geoSource in geosources
			loader = geoSource.textureLoader
			if loader and (loader.requestsCount() + loader.requestsInProgress()) > 0
				return true
		return false

	#
	# 	* returns current loading status in terms of pending requests and requests in progress
	getLoadingStatus: (data, options) ->
		options or= {}

		if options.priorityMap
			data.priorityMap or= {}

		for gs in @geoSources
			loader = gs.textureLoader
			if loader
				data.total += data.visibleKeys.length
				_.each loader.requests, (req) ->
					if req.key and req.key in data.visibleKeys
						data.requests += 1
						if req.running
							data.requestsRunning += 1

						if options.priorityMap
							data.priorityMap[req.key] = req.priority()



				# result.requestsPending += loader.requestsCount()
				# result.requestsInProgress += loader.requestsInProgress()

		return data


	#
	# 	* returns current loading state of the geosource
	# 	*
	# 	* geoSource - geosource name
	isGeosourceIsLoading: (geoSource) ->
		for gs in @geoSources
			if gs.name == geoSource
				loader = gs.textureLoader
				if loader and (loader.requestsCount() + loader.requestsInProgress()) > 0
					return true
		return false

	_getUrl: (key, geoSource, options) ->
		format = "PYX0" #default format
		format = options.format if options.format?
		size = @defaultSize
		size = options.size if options.size?
		if options.range
			if format == "PYX0"
				url = "#{window.gwcHost.get(geoSource)}/api/v1/rhombus/TextureValue?key=#{key}&size=#{size}&geosource=#{geoSource}&format=#{format}"
			else
				url = "#{window.gwcHost.get(geoSource)}/api/v1/rhombus/TextureValue?key=#{key}&size=#{size}&geosource=#{geoSource}&format=#{format}&min=#{options.range[0]}&max=#{options.range[1]}"
		else if options.style
			url = "#{window.gwcHost.get(geoSource)}/api/v1/rhombus/style?key=#{key}&size=#{size}&geosource=#{geoSource}&format=png&style=" + encodeURIComponent(JSON.stringify(options.style))
		else
			url = "#{window.gwcHost.get(geoSource)}/api/v1/rhombus/rgb?key=#{key}&size=#{size}&geosource=#{geoSource}&format=png"
		return url

	_checkLazyDeleteList: (currentTime) ->
		idx = 0
		maxIdx = @lazyDisposeList.length
		while(idx < maxIdx and @lazyDisposeList[idx].disposeTime <= currentTime)
			idx++
		if idx > 0
			for i in [0...idx]
				record = @lazyDisposeList[i]
				#console.log "lazy dispose", record.geosourceID, record.key

				geosource = @geoSources[record.geosourceID]
				delete geosource.keys[record.key]

				record.data.dispose() if record.data
				record.data = undefined

			@lazyDisposeList = @lazyDisposeList.slice(idx)

	_putInLazyDeleteList: (record) ->
		if record.disposeTime
			return
		#console.log "putInLazyDeleteList", record.geosourceID, record.key
		record.disposeTime = Date.now() + @disposeDelay * 1000
		@lazyDisposeList.push(record)

	_recoverFromDisposeList: (record) ->
		for i in [0...@lazyDisposeList.length]
			item = @lazyDisposeList[i]
			if item.geosourceID == record.geosourceID and item.key == record.key
				#console.log "recovered from dispose", record.geosourceID, record.key
				@lazyDisposeList.splice(i, 1)
				record.disposeTime = undefined
				return
		console.log "Failed to recover from dispose", record.geosourceID, record.key



module.exports = {
	GeoSourceProvider: GeoSourceProvider
}