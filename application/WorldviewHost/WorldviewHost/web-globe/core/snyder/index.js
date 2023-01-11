/** 
	For now we just return the snyder projection implementation.

	We can change this up to expose more functionality later
*/

// return module definition
module.exports = {
	CoordLatLon: require('./coord_lat_lon'),
	Projection : require('./snyder_projection'),
	RhombusMath : require('./rhombus_math'),
	SphereMath : require('./sphere_math'),
	wgs84 : require('./wgs84'),
}