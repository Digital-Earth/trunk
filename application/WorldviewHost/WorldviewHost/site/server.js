/* 
	This is the default server, which does nothing other than render a template
	at the root of the package.

	TODO:: find a solution to expose angular routes here. Right now they are duplicated
	with the client itself
*/

var express = require('express');
var router = express.Router();
var path = require('path');
var utilities = require('../server/utilities');
var config = require('../server/config');
var request = require('request');

// get the current directory name, which we can use to dynamically set a render view
var currentDirectory = __dirname.split(path.sep).pop();

// serve files from this directory
router.use('/site', express.static(__dirname));


// render using "index.ejs" for all content, but "landing.ejs" for the home page
function renderSite(req, res) {
    var backend = config.backends.default;
    var host = req.get('Host').split(':')[0]; // remove port if included
	if (req.query.backend && config.backends[req.query.backend]) {
		backend = config.backends[req.query.backend];
	} else if (config.nonProductionHostBackends[host]) {
	    backend = config.nonProductionHostBackends[host];
	}

	var templatePath = '/index';

	if (req.url === '/'){
		templatePath = '/landing';
		backend = config.backends['live'];
	}

	var urlParts = req.url.toLowerCase().split('/').splice(1);

	function renderPage(metadata) {
		res.render( currentDirectory + templatePath, { 
			title: 'PYXIS WorldView',
			description: 'Make sense of a universe of data with the WorldView globe.',
			metadata: metadata || {},
			backendUrl: backend,
			isLocal: true, // is from localhost
			inject: utilities.bundleInjector(req.app) // returns a closure with app reference
		});
	}
	
	if (urlParts[0] === 'geosource' || urlParts[0] === 'globe' || urlParts[0] === 'gallery')
	{
		request({'url':backend + "/Metadata/" + urlParts[1], json: true},function(err, response, resource) {
			if (!err && resource && resource.Metadata) {
				var image = '';
				
				if (resource.Metadata.ExternalUrls && resource.Metadata.ExternalUrls[0]) {
					image = resource.Metadata.ExternalUrls[0].Url;
				}
				
				var metadata = {
					card: 'summary_large_image',
					title: resource.Metadata.Name || 'PYXIS WorldView',
					description: resource.Metadata.Description || '',
					image: image,
					twitter: "@pyxisinnovation",
				}
				renderPage(metadata);
			} else {
				renderPage();
			}
		});
	} else {
		renderPage();
	}
}

// this will get mapped to '/site' or whatever route this folder is mapped onto
router.get('/', renderSite);
router.get('/site', renderSite);
router.get('/browse', renderSite);

router.get('/features', renderSite);
router.get('/news/:year?', renderSite);
router.get('/news/:year?/:post', renderSite);
router.get('/info/*', renderSite);
router.get('/contact', renderSite);
router.get('/mobile/*', renderSite);
router.get('/User/*', renderSite);
router.get('/GeoSource/*', renderSite);
router.get('/Globe/*', renderSite); 
router.get('/Gallery/*', renderSite);
router.get('/admin/*', renderSite);
router.get('/create/*', renderSite);
router.get('/forgotPassword', renderSite);
router.get('/resetPassword', renderSite);
router.get('/updatedPassword', renderSite);
router.get('/confirmEmail', renderSite);
router.get('/requestNews', renderSite);
router.get('/signUp', renderSite);
router.get('/sso', renderSite);
router.get('/download', renderSite);
router.get('/:name', renderSite);


module.exports = router;