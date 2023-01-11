/**
	Moving Bundles here using a basic "express-bundles" middleware.

	Later on we can switch over to using browserify / require to bundle assets
*/

var fs = require('fs');
var _ = require('underscore');
var path = require('path');
var config = require('./config');

function addFromDirectory(bundle, folderPath, ext){
	ext = ext || '.js';

    var fullPath = path.join(__dirname, "..", folderPath);


	_.each(fs.readdirSync(fullPath), function(file){
		if(path.extname(file) === ext){
			bundle.push(path.join(folderPath, file))
		}
	});
}


var bundles = {};
// specify bundles to be post-processed to be proxyified for a service
var processedBundles = {};

// bundles are pretty simple, use the name of the compiled output path as the key
// and each is an array of source files
// /bundles    will be mapped to    /assets/build   on the server


bundles['/build/jquery.js'] = ['/client/assets/scripts/jquery-2.1.1.js'];
bundles['/build/tweenmax.js'] = ['/client/assets/scripts/TweenMax.min.js'];
bundles['/build/angular.js'] = [
	"/client/assets/scripts/angular-1.2.20.js",
    "/client/assets/scripts/angular-cookies-1.2.20.js",
    "/client/assets/scripts/angular-route-1.2.20.js",
    "/client/assets/scripts/angular-autocomplete.js"
];

bundles['/build/pyxis.js'] = [
	"/client/pyxis/pyxis.js",
    "/client/pyxis/pyxis.user.js",
    "/client/pyxis/pyxis.gallery.js",
    "/client/pyxis/pyxis.area.js"
];


// added bundles for the website
bundles['/build/site.js'] = [
	"/client/assets/scripts/jquery-2.1.1.js",
	"/client/assets/scripts/TweenMax.min.js",  
	"/client/assets/scripts/angular-1.2.20.js",
	"/client/assets/scripts/angular-cookies-1.2.20.js",
	"/client/assets/scripts/angular-route-1.2.20.js",
	"/client/assets/scripts/angular-autocomplete.js",
	"/client/pyxis/pyxis.js",
	"/client/pyxis/pyxis.user.js",
	"/client/pyxis/pyxis.gallery.js",
	"/client/pyxis/pyxis.area.js",
	"/client/pyxis-ui.js",
	"/client/flux/dispatcher.js",
	"/client/analytics.js",
	"/client/site.js",
	"/client/studio/legacyservices/positionHelper.js",
	"/client/assets/scripts/localization.js", // added here for landing page transition
	"/client/i18n/studio.default.js", // added here for landing page transition

	'/site/js/vendor/modernizr.custom.26732.js',
	'/site/js/vendor/jquery.truncate.js',
	'/site/js/vendor/underscore-min.js',
	'/site/js/vendor/bodymovin-dev.js',
	'/site/js/vendor/twitterFetcher.js',
	'/site/js/news.repository.js',

	'/site/js/vendor/angular-resource.min.js',
	'/site/js/vendor/angular-animate.min.js',

	'/site/js/vendor/angular-validation.min.js',
	'/site/js/vendor/angular-validation-schema.min.js',
	'/site/js/vendor/angular-validation-rule.min.js',
	'/site/js/vendor/angular-retina.js',
	'/site/js/vendor/angular-socialshare.min.js',
	'/site/js/vendor/svgDirs.js',

	'/site/js/app.controllers.js',
	'/site/js/app.services.js',
	'/site/js/app.filters.js',
	'/site/js/app.directives.js',

	'/site/js/vendor/contentful.min.js'
]
addFromDirectory(bundles['/build/site.js'], '/client/site', '.js');
addFromDirectory(bundles['/build/site.js'], '/client/site/controllers', '.js');
addFromDirectory(bundles['/build/site.js'], '/client/site/features', '.js');

bundles['/build/studio-frameworks.js'] = [
    "/client/assets/scripts/jquery-2.1.1.js",
    "/client/assets/scripts/TweenMax.min.js",
    "/client/assets/scripts/angular-1.2.20.js",
    "/client/assets/scripts/angular-cookies-1.2.20.js",
    "/client/assets/scripts/angular-route-1.2.20.js",
    "/client/assets/scripts/angular-autocomplete.js",
    "/client/assets/scripts/angular-sanitize.min.js",
    "/client/assets/scripts/underscore-min.js",
    "/client/assets/scripts/dat.gui.min.js",
    //"/client/assets/scripts/three.min.js",
];


bundles['/build/studio.js'] = [
	"/client/flux/dispatcher.js",
	"/client/analytics.js",
	"/client/pyxis-ui.js",
	"/client/studio.js",
	"/client/studioConfig.js",
	"/client/site/directives.js",
	"/client/studio/directives.js"
];
addFromDirectory(bundles['/build/studio.js'], '/client/studio/legacyservices', '.js');
addFromDirectory(bundles['/build/studio.js'], '/client/studio/features', '.js');
addFromDirectory(bundles['/build/studio.js'], '/client/studio/services', '.js');
addFromDirectory(bundles['/build/studio.js'], '/client/studio/stores', '.js');
addFromDirectory(bundles['/build/studio.js'], '/client/site/features', '.js');
processedBundles['/build/studio.js'] = config.service;

bundles['/build/studio.css'] = [
    "/client/assets/styles/studio.css"
];
processedBundles['/build/studio.css'] = config.service;

bundles['/build/viewer.css'] = [
    "/client/assets/styles/viewer.css"
];
processedBundles['/build/viewer.css'] = config.service;

bundles['/build/studio-localization.js'] = [
	"/client/assets/scripts/localization.js",
	"/client/i18n/studio.default.js"
];

bundles['/build/landing-demo-bundle.js'] = [
	"/client/pyxis/pyxis.js",
    "/client/pyxis/pyxis.user.js",
    "/client/pyxis/pyxis.gallery.js",
    "/client/pyxis/pyxis.area.js",
    "/client/flux/dispatcher.js",
	"/client/analytics.js",
	"/client/pyxis-ui.js",
	"/client/studio.js",
	"/client/studioConfig.js",
	"/client/site/directives.js",
	"/client/studio/directives.js",
	"/client/studio/skin/bootstrap.js",
    "/client/studio/skin/demo.js",
    "/demo/interface.js"
];
addFromDirectory(bundles['/build/landing-demo-bundle.js'], '/client/studio/legacyservices', '.js');
addFromDirectory(bundles['/build/landing-demo-bundle.js'], '/client/studio/features', '.js');
addFromDirectory(bundles['/build/landing-demo-bundle.js'], '/client/studio/services', '.js');
addFromDirectory(bundles['/build/landing-demo-bundle.js'], '/client/studio/stores', '.js');
addFromDirectory(bundles['/build/landing-demo-bundle.js'], '/client/site/features', '.js');

bundles['/build/studio-default-skin.js'] = [
    "/client/studio/skin/bootstrap.js",
    "/client/studio/skin/default.js"
];

bundles['/build/studio-web-skin.js'] = [
    "/client/studio/skin/bootstrap.js",
    "/client/studio/skin/web.js"
];

bundles['/build/studio-demo-skin.js'] = [
    "/client/studio/skin/bootstrap.js",
    "/client/studio/skin/demo.js"
];
addFromDirectory(bundles['/build/studio-demo-skin.js'], '/client/studio/demos', '.js');

bundles['/build/studio-viewer-skin.js'] = [
    "/client/studio/skin/bootstrap.js",
    "/client/studio/skin/viewer.js"
];

bundles['/build/studio-editable-viewer-skin.js'] = [
    "/client/studio/skin/bootstrap.js",
    "/client/studio/skin/editable-viewer.js"
];

bundles['/build/studio-landing-skin.js'] = [
    "/client/studio/skin/bootstrap.js",
    "/client/studio/skin/landing.js"
];

bundles['/build/studio-embed-skin.js'] = [
    "/client/studio/skin/bootstrap.js",
    "/client/studio/skin/embed.js"
];

module.exports = {
    bundles: bundles,
    processedBundles: processedBundles
};