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
router.get('/:id/:mapName?', function (req, res) {
    var id = req.params.id;
    var mapName = req.params.mapName;

    var mode = req.query.mode || 'default';

	var backend = config.backends.default;
    if (req.query.backend && config.backends[req.query.backend]) {
        backend = config.backends[req.query.backend];
    }

    if (!id || id == "new") {
        id = "GeoliteracyAustralia"
    }

    var guidRegex = /^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i;

    //if map name is a guid - we can use to resolve to map object
    if (guidRegex.test(mapName)) {
        id = mapName;
    }

    if (guidRegex.test(id))
    {
        var requestUrl = backend + '/Metadata/' + id;
    } else if (!mapName) { //assume gallery name
        var requestUrl = backend + '/Gallery/?name=' + encodeURIComponent(id)
    } else { //assume gallery name and map name
        var filter = "Metadata/Name eq '" + mapName + "' and Metadata/Providers/any(p: p/Name eq '" + id + "') and State eq 'Active'"
        var requestUrl = backend + "/Map/?$filter=" + encodeURIComponent(filter)
    }

    console.log(requestUrl);

    function renderResource(resource) {
        var imageUrl = config.defaultMapImage;
        if ( resource.Metadata && resource.Metadata.ExternalUrls && resource.Metadata.ExternalUrls[0] ){
            imageUrl = resource.Metadata.ExternalUrls[0].Url;
        }

        switch(mode) {
            case 'readonly':
            template = currentDirectory + '/index';
            break;

            case 'edit':
            case 'default':
            default:
            template = currentDirectory + '/editable-index';
            break;
        }

        res.render( template , {
            title: resource.Metadata.Name,
            description: resource.Metadata.Description,
            date: resource.Metadata.Updated,
            id: id,
            resource: JSON.stringify(resource),
            imageUrl: imageUrl,
            isLocal: true, // is from localhost
            runTests: false,
            backendUrl: backend,
            host: req.header('host'),
            inject: utilities.serviceBundleInjector(req.app) // returns a closure with app reference
        });
    }

    // fetch the map from api server and render out our template
    request({url: requestUrl, json: true}, function(err, response, resource){
        if (err) {
            utilities.errorHandler(req, res)(err);
        } else {
            //if the resource is a result of a search query - pick the first one
            if (resource.Items) {
                resource = resource.Items[0];
            }
            //validate the resource has Id and Metadata attached to it
            if (!resource || !resource.Id || !resource.Metadata) {
                utilities.errorHandler(req, res)("Failed to find requested resource");
            } else {
                renderResource(resource);
            }
        }
    });

});

module.exports = router;