angles = require "./angles"
THREE = require 'three'

wgs84 = {}

knEarthRadius = 6371007

kfSemiMajorAxis = 6378137.0
kfFlattening = 1.0 / 298.257223563
kfOneMinusFSqr = (1.0 - kfFlattening) * (1.0 - kfFlattening)
kfSemiMinorAxis = kfSemiMajorAxis * (1.0 - kfFlattening)

kfDefaultDoublePrecision = 1.0e-10

xyzToGeocentricLatLon = (xyz) ->
	size = Math.sqrt(xyz.x*xyz.x+xyz.y*xyz.y+xyz.z*xyz.z)
	
	return null if size == 0
	
	nx = xyz.x / size
	ny = xyz.y / size
	nz = xyz.z / size

	if Math.abs(1 - ny) < kfDefaultDoublePrecision
		return { lat: angles.const.degrees90InRadians, lon: 0 }

	if Math.abs(-1 - ny) < kfDefaultDoublePrecision
		return { lat: -angles.const.degrees90InRadians, lon: 0 }

	return {
		lat: Math.asin(ny),
		lon: -Math.atan2(nz,nx)
	}

geocentricLatLonToXyz = (latlon) ->
	x = Math.cos(latlon.lat) * Math.cos(-latlon.lon)
	z = Math.cos(latlon.lat) * Math.sin(-latlon.lon)
	y = Math.sin(latlon.lat)

	return new THREE.Vector3(x,y,z)

geocentricLatLonToWgs84LatLon = (latlon) ->
	return {
		lat: Math.atan(Math.tan(latlon.lat) / kfOneMinusFSqr),
		lon: latlon.lon
	}

wgs84LatLonToGeocentricLatLon = (latlon) ->
	return {
		lat: Math.atan(Math.tan(latlon.lat) * kfOneMinusFSqr),
		lon: latlon.lon
	}

wgs84.xyzToLatLon = (xyz) ->
	radiansLatLon = geocentricLatLonToWgs84LatLon(xyzToGeocentricLatLon(xyz))
	return { lat: angles.degrees(radiansLatLon.lat), lon: angles.degrees(radiansLatLon.lon) }

wgs84.latLonToXyz = (latlon) ->
	radiansLatLon = 
		lat: angles.radians(latlon.lat)
		lon: angles.radians(latlon.lon)

	xyz = geocentricLatLonToXyz(wgs84LatLonToGeocentricLatLon(radiansLatLon))
	xyz.multiplyScalar(knEarthRadius);
	return xyz;

wgs84.fromRadiansGeocentricLatLon = (latlon) ->
	radiansLatLon = geocentricLatLonToWgs84LatLon(latlon)
	return { lat: angles.degrees(radiansLatLon.lat), lon: angles.degrees(radiansLatLon.lon) }

wgs84.toRadiansGeocentricLatLon = (latlon) ->
	radiansLatLon = 
		lat: angles.radians(latlon.lat)
		lon: angles.radians(latlon.lon)

	return wgs84LatLonToGeocentricLatLon(radiansLatLon)

wgs84.earthRadius = knEarthRadius

window.wgs84 = wgs84

module.exports = wgs84