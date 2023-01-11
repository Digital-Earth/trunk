/** 
	Build using commonjs style modules.

	Everything underneath this api package gets bundled together via browserify. 
*/

var _ = require('underscore');
var config = require('./config');
var model = require('./model');
var shaderFactory = require('./shader-factory');

// set this up for export
defaultTheme = model.themeFactory(config.lightThemeConfig);

var exportObject = {
	Theme: model.Theme,
	default: defaultTheme,
	themeFactory: model.themeFactory,
	shaderFactory: shaderFactory
}

// copy all of the config themes into this export
_.each( config, function(themeConfig, name){
	exportObject[name] = model.themeFactory(themeConfig);
});

module.exports = exportObject;