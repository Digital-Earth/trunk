
var express = require('express');
var compression = require('compression');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var lessMiddleware = require('express-less-middleware');
var stylus = require('stylus');


var utilities = require('./server/utilities');
var config = require('./server/config');

// set environment variables like AZURE CLOUD STORAGE
utilities.setDefaultEnvironmentVariables();
console.log("ENVIRONMENT", process.env);


var app = express();


// set views root as the root of this folder
// when we render we will call the views directly
app.set('views', __dirname);
app.set('view engine', 'ejs');
app.set('view options', {layout: false});

app.use(favicon(__dirname + '/client/assets/images/favicon.ico'));
// app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(compression());


// treat assets as a shared asset directory for images, vendor scripts, and other items
app.use('/assets', express.static(path.join(__dirname, 'client', 'assets')));
app.use('/assets', express.static(path.join(__dirname, 'site', 'assets')));  //  site assets lower priority

// Create proxyified routes for assets
// All asset requests should accessible with /assets/{service} to work with the proxy
app.use('/assets/'+config.service+'/styles/studio', express.static(path.join(__dirname, 'build', 'studio')));
app.use('/assets/'+config.service+'/scripts', express.static(path.join(__dirname, 'build')));
app.use('/assets/'+config.service, express.static(path.join(__dirname, 'client', 'assets')));
app.use('/assets/'+config.service, express.static(path.join(__dirname, 'client')));

app.use('/test', express.static(path.join(__dirname, 'test')));

// load each of our packages into variables and then plug them
// into the application router
app.use('/studio', require('./studio/server'));  // serve the web studio at /studio
app.use('/demo', require('./demo/server'));
app.use('/embed', require('./embed/server'));
app.use('/view', require('./viewer/server'));
app.use('/api', require('./server/api'));
// app.use('/', require('./gallery/server'));

// make sure the site package is last as it has catch all routes for
// /:name
app.use('/', require('./site/server'));



// 404 and error handlers provided by server utilities
utilities.setupErrorHandling(app);


module.exports = app;
