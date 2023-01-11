###
	Attempting to port CesiumViewer / CesiumWidget to combine with THREE.js  
###

_ = require 'underscore'
$ = require 'jquery'
THREE = require 'three'
glslify = require 'glslify'
EventEmitter = require 'event-emitter'

# LocationBarViewModel = require('terriajs/lib/ViewModels/LocationBarViewModel')
# DistanceLegendViewModel = require('terriajs/lib/ViewModels/DistanceLegendViewModel')
# NavigationViewModel = require('terriajs/lib/ViewModels/NavigationViewModel')


# note:  window.Cesium should already be set
CesiumViewer = window.Cesium.Viewer
CesiumWidget = window.Cesium.CesiumWidget




extendGlobeCanvas = (globeCanvas) ->
	console.log "EXTEND GLOBE CANVAS ", globeCanvas

	ui = document.getElementById('ui')

	terriaObj = {
		cesium: globeCanvas.viewer
	}

	LocationBarViewModel.create({
		container: ui,
		terria: terriaObj,
		mapElement: document.getElementById('cesiumContainer')
		})	



# // Create the lat/lon/elev and distance widgets.
#    LocationBarViewModel.create({
#        container: ui,
#        terria: terria,
#        mapElement: document.getElementById('cesiumContainer')
#    });

#    DistanceLegendViewModel.create({
#        container: ui,
#        terria: terria,
#        mapElement: document.getElementById('cesiumContainer')
#    });

#    // Create the navigation controls.
#    NavigationViewModel.create({
#        container: ui,
#        terria: terria
#    });
	


module.exports = extendGlobeCanvas