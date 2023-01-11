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
router.use('/', express.static(__dirname));

// this will get mapped to '/site' or whatever route this folder is mapped onto
router.get('/', function (req, res) {
	var backend = config.backends.default;
    if (req.query.backend && config.backends[req.query.backend]) {
        backend = config.backends[req.query.backend];
    }

    res.render( currentDirectory + '/index', { 
    	title: 'PYXIS WorldView', 
    	isLocal: true, // is from localhost
    	runTests: false,
    	backendUrl: backend,
        host: req.header('host'),
    	inject: utilities.bundleInjector(req.app) // returns a closure with app reference
    });
});

module.exports = router;