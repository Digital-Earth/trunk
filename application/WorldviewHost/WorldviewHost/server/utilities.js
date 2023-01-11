/*
*	Define some Server utilities like:
*	Error handling, build tasks on development server
*/

var _ = require('underscore');
var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

// build bundles
var bundles = require('./bundles');
var config = require('./config');




function setupErrorHandling(app) {
	// catch 404 and forward to error handler
	app.use(function(req, res, next) {
		var err = new Error('Not Found');
		err.status = 404;
		next(err);
	});



	// development error handler
	// will print stacktrace
	if (app.get('env') === 'development') {
		app.use(function(err, req, res, next) {
			res.status(err.status || 500);
			res.render('server/error', {
				message: err.message,
				error: err
			});
		});
	}

	// production error handler
	// no stacktraces leaked to user
	app.use(function(err, req, res, next) {
		res.status(err.status || 500);
		res.render('server/error', {
			message: err.message,
			error: {}
		});
	});
}


// return a default errorHandler function that takes err as argument
// this can be used with request module for .on('error')
function errorHandler(req, res) {
	return function(err) {
		res.status(err && err.status || 500);
		res.render('server/error', {
			message: err && err.message || '',
			error: {}
		});
	}
}



// loop through the environment variables defined
// in config.js, and set defaults if they don't exist
function setDefaultEnvironmentVariables() {
	_.each(config.env, function(value, key) {
		if (process.env[key] === undefined) {
			process.env[key] = value;
		}
	});
}



var parentPath = path.dirname(module.parent.filename);


/*
*	Called from within templates to make this easier.
* 	all files when the env is 'development' and only a single bundle file in production.
*/
function bundleInjector(app) {

	return function(bundleName) {
		var ext = path.extname(bundleName);
		var str = '';
		var bundleArray = [bundleName];

		if (app.get('env') === 'development') {
			bundleArray = bundles.bundles[bundleName];
		}


		if (ext === '.js') {
			_.each(bundleArray, function(name) {
				str += '<script src="' + name + '"></script>\n';
			})
		} else if (ext === '.css') {
			_.each(bundleArray, function(name) {
				str += '<link rel="stylesheet" href="' + name + '"/>\n';
			})
		}

		return str;
	}
}

/*
*	Called from within templates to make this easier.
* 	all files when the env is 'development' and only a single service-aware bundle reference in production.
*/
function serviceBundleInjector(app) {

	var assetRoot = '/assets/' + config.service;

	var routes = {
		'/build/' : assetRoot + '/scripts/',
		'\\build\\' : assetRoot + '/scripts/',
		'/client/' : assetRoot + '/',
		'\\client\\' : assetRoot + '/',
		'/assets/styles/' : assetRoot + '/styles/'
	}

	function fixRoute(route) {
		for (map in routes) {
			if (route.substr(0,map.length) == map) {
				return routes[map] + route.substr(map.length)
			}
		}
		return route;
	}

	return function(bundleName) {
		var ext = path.extname(bundleName);
		var str = '';
		var bundleArray = [bundleName];

		//on dev machine - we want to show all the source files. 
		//however, we can only do that for bundles that not need to be processed (aka - deep fix url inside the file content)
		if (bundles.bundles[bundleName] && app.get('env') === 'development' && !bundles.processedBundles[bundleName] ) {
			bundleArray = _.map(bundles.bundles[bundleName],fixRoute);
		} else {
			bundleArray[0] = fixRoute(bundleArray[0]);
		}


		if (ext === '.js') {
			_.each(bundleArray, function(name) {
				str += '<script src="' + name + '"></script>\n';
			})
		} else if (ext === '.css') {
			_.each(bundleArray, function(name) {
				str += '<link rel="stylesheet" href="' + name + '"/>\n';
			})
		}

		return str;
	}
}



module.exports = {
	setupErrorHandling: setupErrorHandling,
	setDefaultEnvironmentVariables: setDefaultEnvironmentVariables,
	bundleInjector: bundleInjector,
	serviceBundleInjector: serviceBundleInjector,
	errorHandler: errorHandler
};