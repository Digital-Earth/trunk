/* 
	Here is where we pull together the routes from the current worldview.gallery ApiController
*/

var express = require('express');
var router = express.Router();
var path = require('path');
var azure = require('azure-storage');
var multiparty = require('multiparty');  // handle multipart form uploads
var https = require('https');
var request = require('request'); // request 3rd party library
var geoip = require('geoip-lite');
var _ = require('underscore');

var utilities = require('./utilities');

// get the current directory name, which we can use to dynamically set a render view
var currentDirectory = __dirname.split(path.sep).pop();

router.post('/uploadcrash', function (req, res) {
    var form = new multiparty.Form();
    
    form.on('part', function (part) {
        if (!part.filename) return;
        
        var blobService = azure.createBlobService();
        var size = part.byteCount;
        var name = part.filename;
        var container = 'crash-dumps'; // azure 
        
        blobService.createBlockBlobFromStream(container, name, part, size, function (error) {
            if (error) {
                res.status(500).send('Error uploading to Azure Storage');
            }
            res.send('File uploaded successfully');
        });
    });

    form.on('error', function(error) {
        res.status(error.status || 500).send(error.message);
    });
    
    form.parse(req);
});


newsRequestModel = {
    FirstName: '',
    LastName: '',
    Email: '',
    HearAbout: '',
    HearAboutOther: ''
}

router.post('/requestnews', function (req, res) {
    var data = req.body;
    if (!data || !data.Email || !data.Email.length) {
        res.status(400).send('Provide a valid email address');
        return;
    }
    var solveUrl = 'https://secure.solve360.com/contacts';
    var username = 'cjohnson@pyxisinnovation.com';
    var password = 'R8O2vd09W7B5N4LcccF8naqdlaD8reg0m858D7r5';
    
    // get info from user's IP address
    var ip = req.connection.remoteAddress;
    var geo = geoip.lookup(ip) || { city: 'Unknown', country: 'Unknown' };

    var solve360ContactModel = {
        firstname: data.Email,
        businessemail: data.Email,
        categories: {
            add: ['123502088']
        },
        ownership: '112499556',
        custom15743704: geo.country,
        custom15812839: geo.city
    };
    
    var postData = JSON.stringify(solve360ContactModel);
    var contactsPost = {
        url: solveUrl,
        method: 'POST',
        body: postData,
        auth: {
            username: username,
            password: password
        },
        headers: {
            'Accept': 'application/json',
            'Content-Type': 'application/json',
            'Content-Length': Buffer.byteLength(postData)
        }
    };

    request(contactsPost, function (err, contactsResponse, body) {
        if (err) {
            utilities.errorHandler(req, res)(err);
        } else {
            res.writeHead(200, {
                'content-type': 'text/plain'
            });
            res.write('post successful:\n\n');
            res.end('Complete');
        }
    });
});

module.exports = router;