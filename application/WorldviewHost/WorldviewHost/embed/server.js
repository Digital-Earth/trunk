/* 
    This is the default server, which does nothing other than render a template
    at the root of the package.
*/

var express = require('express');
var router = express.Router();
var path = require('path');
var utilities = require('../server/utilities');
var config = require('../server/config');
var request = require('request');
var _ = require('underscore');

// get the current directory name, which we can use to dynamically set a render view
var currentDirectory = __dirname.split(path.sep).pop();

// serve files from this directory
// router.use('/', express.static(__dirname));

// this will get mapped to '/site' or whatever route this folder is mapped onto
router.get('/:id', function (req, res) {
    var id = req.params.id;

    var backend = config.backends.default;
    if (req.query.backend && config.backends[req.query.backend]) {
        backend = config.backends[req.query.backend];
    }

    // fetch the map from api server and render out our template
    request({url: backend + '/map/' + id, json: true}, function(err, response, map){
        if (err){
            utilities.errorHandler(req, res)(err);
        } else {

            if (!map || !map.Metadata){
                utilities.errorHandler(req, res)(new Error("No map or map service unavailable."));
                return;
            }

            var imageUrl = config.defaultMapImage;
            if (map.Metadata.ExternalUrls && map.Metadata.ExternalUrls[0]){
                imageUrl = map.Metadata.ExternalUrls[0].Url;
            }

            res.render( currentDirectory + '/index', { 
                title: map.Metadata.Name,
                description: map.Metadata.Description,
                date: map.Metadata.Updated,
                id: id,
                resource: JSON.stringify(map),
                imageUrl: imageUrl, 
                isLocal: true, // is from localhost
                runTests: false,
                backendUrl: backend,
                host: req.header('host'),
                inject: utilities.bundleInjector(req.app) // returns a closure with app reference
            });
        }
    });

});

module.exports = router;