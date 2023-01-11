'use strict';
/** 
	API Core module
*/

var wgs84 = require('./wgs84');
var hosts = require('./hosts');
var snyder = require('./snyder');

// return module definition
module.exports = {
	version: '1.0.0',
	wgs84: wgs84,
	snyder: snyder
}