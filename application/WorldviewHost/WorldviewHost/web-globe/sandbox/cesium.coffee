###
	!!!  No longer being used, see cesium2

    This is a coffeescript file that compiles-to-javascript.

    The purpose here is to rapidly prototype behaviors and classes and then move them 
    into the core engine js module once a piece of functionality is stable. -Dean


    Here we are building a custom layer on top of Cesium / TerriaJS camera controls, but allowing for
    an integrated geometry/map source provided by WorldView     
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
glslify = require 'glslify'
EventEmitter = require 'event-emitter'

knockout = require('terriajs-cesium/Source/ThirdParty/knockout')

Terria = require('terriajs/lib/Models/Terria')
TerriaViewer = require('./custom').TerriaViewer  # adjusted some hardcoded defaults
registerKnockoutBindings = require('terriajs/lib/Core/registerKnockoutBindings')
corsProxy = require('terriajs/lib/Core/corsProxy')

NavigationViewModel = require('terriajs/lib/ViewModels/NavigationViewModel')
DistanceLegendViewModel = require('terriajs/lib/ViewModels/DistanceLegendViewModel')
LocationBarViewModel = require('terriajs/lib/ViewModels/LocationBarViewModel')
AnimationViewModel = require('terriajs/lib/ViewModels/AnimationViewModel')
createGlobalBaseMapOptions = require('terriajs/lib/ViewModels/createGlobalBaseMapOptions')


#  some initialization
configuration =
	terriaBaseUrl: 'build/TerriaJS'
	cesiumBaseUrl: 'assets/cesium' # use default
	bingMapsKey: undefined # use Cesium key
	proxyBaseUrl: 'proxy/'
	conversionServiceBaseUrl: 'convert'
	regionMappingDefinitionsUrl: 'data/regionMapping.json'

corsProxy.baseProxyUrl = configuration.proxyBaseUrl


###
    Create a custom viewer on top of TerriaViewer, which we will later throw out.
###
class GlobeViewer extends TerriaViewer
	foo: -> console.log 'asdf'

  #   constructor: (environment, options) ->
		# super(environment, options)

		# @on 'resize', =>
		# 	console.log 'asdf'


# copy methods from right to left, adds EventEmitter to prototype chain
_.extend GlobeViewer.prototype, EventEmitter.prototype, GlobeViewer.prototype



buildCesiumX = ->

	console.log "BUILD CESIUM"

	cesiumEnvironment = new Terria({
	    appName: 'GlobeTest',
	    baseUrl: configuration.terriaBaseUrl,
	    cesiumBaseUrl: configuration.cesiumBaseUrl,
	    regionMappingDefinitionsUrl: configuration.regionMappingDefinitionsUrl
	})

	window.CE = cesiumEnvironment



	cesiumEnvironment.error.addEventListener (e) ->
		PopupMessageViewModel.open('ui', {
			title: e.title
			message: e.message
		})


	cesiumRuntime = cesiumEnvironment.start({
	    # If you don't want the user to be able to control catalog loading via the URL, remove the applicationUrl property below
	    # as well as the call to "updateApplicationOnHashChange" further down.
	    applicationUrl: window.location,
	    configUrl: 'assets/cesium/config.json'
	})

	cesiumRuntime.otherwise (e) -> raiseErrorToUser(cesiumEnvironment, e)

	# setup the application
	cesiumRuntime.always ->
	    configuration.bingMapsKey = cesiumEnvironment.configParameters.bingMapsKey or configuration.bingMapsKey

	    # Create the map/globe.  TODO:: options
	    console.log "CREATE GLOBE VIEWER"
	    GlobeViewer.create(cesiumEnvironment, {})
	    console.log "FINISHED CREATE GLOBE VIEWER"

	    #We'll put the entire user interface into a DOM element called 'ui'.
	    ui = document.getElementById('ui')

	    # Create the various base map options.
	    # var australiaBaseMaps = createAustraliaBaseMapOptions(cesiumEnvironment);
	    globalBaseMaps = createGlobalBaseMapOptions(cesiumEnvironment, configuration.bingMapsKey)

	    allBaseMaps = globalBaseMaps
	    selectBaseMap(cesiumEnvironment, allBaseMaps, 'Bing Maps Aerial with Labels')


	    # Create the lat/lon/elev and distance widgets.
	    LocationBarViewModel.create({
	        container: ui
	        terria: cesiumEnvironment
	        mapElement: document.getElementById('cesiumContainer')
	    })

	    DistanceLegendViewModel.create({
	        container: ui
	        terria: cesiumEnvironment
	        mapElement: document.getElementById('cesiumContainer')
	    })

	    # Create the navigation controls.
	    NavigationViewModel.create({
	        container: ui
	        terria: cesiumEnvironment
	    })

	    # Create the animation controls.
	    AnimationViewModel.create({
	        container: document.getElementById('cesiumContainer')
	        terria: cesiumEnvironment
	        mapElementsToDisplace: [
	            'cesium-widget-credits',
	            'leaflet-control-attribution',
	            'distance-legend',
	            'location-bar'
	        ]
	    })

	    console.log "DONE WITH CESIUM CREATE"

	return cesiumEnvironment




module.exports = buildCesium

