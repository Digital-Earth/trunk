'use strict';
/** 
	Build using commonjs style modules.

	Everything underneath this api package gets bundled together via browserify.
*/

var underscore = require('underscore');
var core = require('./core');
var globe = require('./globe');
var THREE = require('three');

var PYXIS = _.extend({}, core, globe);

// expose the default load feedback view for the app to instantiate
PYXIS.LoadFeedbackView = require('./utilities/loader').LoadFeedbackView;

window.PYXIS = PYXIS;
window.THREE = THREE;  // we need this global reference for the THREE in interface.js

module.exports = PYXIS;