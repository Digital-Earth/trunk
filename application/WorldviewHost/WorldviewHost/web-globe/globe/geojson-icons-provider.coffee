###
	This is a coffeescript file that compiles-to-javascript.

	GeoJsonIconsProvider provides logic for loading and processing geojson file as icons
###

_ = require 'underscore'
$ = require 'jquery'
wgs84 = require '../core/wgs84'
THREE = require 'three'

findLatLonCenter = (coordinates) ->
	box = new THREE.Box3()
	points = []
	for coord in @coordinates
		points.push wgs84.latLonToXyz({lon:coord[0],lat:coord[1]})
	box.setFromPoints(points)
	return wgs84.xyzToLatLon(box.center())

convertGeometryToPoint = (geometry) ->
	switch geometry.type
		when 'Point' then return { lon: geometry.coordinates[0], lat: geometry.coordinates[1] }
		when 'Circle' then return { lon: geometry.coordinates[0], lat: geometry.coordinates[1] }
		when 'LineString' then return findLatLonCenter(geometry.coordinates)
		#flatten all line segments
		when 'MultiLineString' then return findLatLonCenter(_.flatten(geometry.coordinates,true))
		#take the first ring only
		when 'Polygon' then return findLatLonCenter(geometry.coordinates[0])
		#take the first ring from every polygon
		when 'MultiPolygon' then return findLatLonCenter(_.flatten(_.map(geometry.coordinates,(polygon) => polygon[0]),true))
		else throw "unknown geometry type"

indexFeatures = (features) ->
	return _.map features, (feature) ->
		centerLatLon = convertGeometryToPoint(feature.geometry)
		ll = new PYXIS.snyder.CoordLatLon();
		ll.setInDegrees(centerLatLon.lat, centerLatLon.lon)
		index = PYXIS.snyder.RhombusMath.wgs84LatLonToRhombusUV(ll);

		return {
			center: wgs84.latLonToXyz(centerLatLon),
			index: index,
			feature: feature
		}

divideIntoGroups = (indexedFeatures) ->
	return _.groupBy( indexedFeature, (f) -> f.index.rhombus )

createIcon = (indexedFeature) ->
	return {
		Values: indexedFeature.feature.properties
		Id: indexedFeature.feature.id
		Center: indexedFeature.center
	}

addGroupValue = (groupValues, field, value, count) ->
	count or= 1
	groupValues[field] = { Values: {} } if !(field of groupValues)
	groupValues[field].Values[value] = 0 if !(value of groupValues[field].Values)
	groupValues[field].Values[value]+= count

createIconGroup = (key, indexedFeatures) ->
	group =
		Id: key
		GroupId: key
		Count: indexedFeatures.length
		Icons: []
		Groups: []
		Values: {}

	sphere = new THREE.Sphere()
	sphere.setFromPoints(_.map(indexedFeatures,(indexedFeature) -> indexedFeature.center))
	group.Center = sphere.center
	
	group.Center.normalize().multiplyScalar(wgs84.earthRadius)
	group.Radius = sphere.radius * 1.5

	if (key && indexedFeatures.length < 100)
		for indexedFeature in indexedFeatures
			group.Icons.push(createIcon(indexedFeature))
			for field,value of indexedFeature.feature.properties
				addGroupValue(group.Values,field,value)
	else
		#increate subdivision only if this is not the root group
		if key
			for indexedFeature in indexedFeatures
				#increase resolution
				indexedFeature.index = PYXIS.snyder.RhombusMath.increaseRhombusResolution(indexedFeature.index)
		
		#order the features
		subGroups = _.groupBy indexedFeatures, (indexedFeature) -> indexedFeature.index.rhombus
		
		for subKey, subGroup of subGroups
			if key && subGroup.length < 10
				for indexedFeature in subGroup
					group.Icons.push(createIcon(indexedFeature))
					for field,value of indexedFeature.properties
						addGroupValue(group.Values,field,value)
			else
				iconSubGroup = createIconGroup(subKey,subGroup)
				group.Groups.push(iconSubGroup)
				
				for field,fieldStat of iconSubGroup.Values
					for value,count of fieldStat.Values
						addGroupValue(group.Values,field,value,count)
	
	return group

flattenTree = (node,target) ->
	flatNodes = target || {}
	flatNodes[node.GroupId] = node;

	for group in node.Groups
		flattenTree(group,flatNodes)

	return flatNodes
	

class GeoJsonIconsProvider
	constructor: (@layer) ->
		@iconTree = createIconGroup('',indexFeatures(@layer.data.features))
		@groups = flattenTree(@iconTree)

	load: (key, success, error, priority) ->
		if key of @groups
			success(@groups[key]) if success
		else
			error('not icon group with that key was found') if error
		return @groups[key]

module.exports = GeoJsonIconsProvider