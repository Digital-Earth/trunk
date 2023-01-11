#!/usr/bin/env node
var debug = require('debug')('WVHost');
var app = require('./app');
var config = require('./server/config');
var gulp = require('gulp');
require('./gulpfile.js');


app.set('port', process.env.PORT || config.defaultPort);

// start the default 'watch' command
gulp.start('default');