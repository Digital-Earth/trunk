/* 
	This is the default server, which does nothing other than render a template
	at the root of the package.
*/

var express = require('express');
var router = express.Router();
var path = require('path');
var utilities = require('../server/utilities');
var config = require('../server/config');

// get the current directory name, which we can use to dynamically set a render view
var currentDirectory = __dirname.split(path.sep).pop();


// serve files from this directory
// router.use('/', express.static(__dirname));

// serve the desktop studio application interface
router.get('/beta-v1', function (req, res) {
    var backend = config.backends.default;
    var host = req.get('Host').split(':')[0]; // remove port if included
    if (req.query.backend && config.backends[req.query.backend]) {
        backend = config.backends[req.query.backend];
    } else if (config.nonProductionHostBackends[host]) {
        backend = config.nonProductionHostBackends[host];
    }

    res.render( currentDirectory + '/desktop-studio', { 
        title: 'PYXIS WorldView', 
        isLocal: host === 'localhost', // is from localhost
        runTests: req.query.dev === 'test',
        backendUrl: backend,
        host: req.header('host'),
        inject: utilities.serviceBundleInjector(req.app) // returns a closure with app reference
    });
});

// this will get mapped to '/site' or whatever route this folder is mapped onto
router.get('/web', function (req, res) {
	var backend = config.backends.default;
    var host = req.get('Host').split(':')[0]; // remove port if included
	if (req.query.backend && config.backends[req.query.backend]) {
		backend = config.backends[req.query.backend];
	}

    res.render( currentDirectory + '/web-studio', { 
    	title: 'PYXIS WorldView', 
    	isLocal: host === 'localhost', // is from localhost
    	runTests: req.query.dev === 'test',
    	backendUrl: backend,
        host: req.header('host'),
    	inject: utilities.serviceBundleInjector(req.app) // returns a closure with app reference
    });
});

module.exports = router;