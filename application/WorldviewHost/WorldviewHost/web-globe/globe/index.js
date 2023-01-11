'use strict';
/**
	Build using commonjs style modules

	Note:: the sandbox is there for development staging, with each component moving out
	of the sanbox once it's mature it enough
*/

var underscore = require('underscore');
var GlobeCanvas = require('./canvas');
var Globe = require('./globe');
var GlobeCamera = require('./camera').GlobeCamera;
var RhombusHierachy = require('./tree/hierarchy').RhombusHierachy;

// remove once included in scene
var FontTexture = require('./shaders/text/font-texture');

var themes = require('./theme');

module.exports = {
	themes: themes,
	GlobeCanvas: GlobeCanvas,
	Globe: Globe,
	GlobeCamera: GlobeCamera
}