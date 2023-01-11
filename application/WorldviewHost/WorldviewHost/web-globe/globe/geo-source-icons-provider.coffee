###
	This is a coffeescript file that compiles-to-javascript.

	GeoSourceIconsProvider provides logic for loading icons from a GeoSource
###

_ = require 'underscore'
$ = require 'jquery'
wgs84 = require '../core/wgs84'
THREE = require 'three'
DataLoader = require('../utilities/loader').DataLoader

class GeoSourceIconsProvider
	constructor: (@layer,options) ->
		@dataLoader = new DataLoader(options)
		
		@fields = [ encodeURIComponent(@layer.style.Icon.PaletteExpression) ]
		if (@layer.style.Icon.HoverExpression)
			@fields.push(encodeURIComponent(@layer.style.Icon.HoverExpression))
		else if (@layer.styleHint && @layer.styleHint.Icon && @layer.styleHint.Icon.HoverExpression)
			@fields.push(encodeURIComponent(@layer.styleHint.Icon.HoverExpression))

		@baseUrl = "#{window.gwcHost.get(@layer.geoSource.Id)}/api/v1/Local/Icons?geoSource=#{@layer.geoSource.Id}&field=#{@fields.join(',')}"

	load: (key, success, error, priority) ->
		url = @baseUrl + "&groupId=#{encodeURIComponent(key)}"

		fixData = (data) =>
			SCALE = wgs84.earthRadius
			for icon in data.Icons
				icon.Center = new THREE.Vector3(SCALE * icon.X, SCALE * icon.Z, SCALE * icon.Y * -1.0)
				delete icon.X
				delete icon.Y
				delete icon.Z
				icon.Radius *= SCALE if icon.Radius

			for group in data.Groups
				group.Center = new THREE.Vector3(SCALE * group.X, SCALE * group.Z, SCALE * group.Y * -1.0)
				delete group.X
				delete group.Y
				delete group.Z
				group.Radius *= SCALE if group.Radius

			success(data)

		@dataLoader.load {
			url: url
			success: fixData
			error: error
			priority: priority
		}

module.exports = GeoSourceIconsProvider