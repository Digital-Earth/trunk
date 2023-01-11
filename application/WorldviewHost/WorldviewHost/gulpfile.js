'use strict';

var gulp = require('gulp');
var gutil = require('gulp-util');
var wrench = require('wrench');
var browserify = require('browserify');
var transform = require('vinyl-transform');

var options = {
	src: '.',
	dist: 'build',
	tmp: '.tmp',
	e2e: 'e2e',
	errorHandler: function(title) {
		return function(err) {
			gutil.log(gutil.colors.red('[' + title + ']'), err.toString());
			this.emit('end');
		};
	},

	wiredep: {
		directory: 'bower_components'
	}
};


// Import all the scripts inside the build directory
wrench.readdirSyncRecursive('./server/build').filter(function(file) {
	return (/\.(js|coffee)$/i).test(file);
}).map(function(file) {
	require('./server/build/' + file)(options);
});



gulp.task('watch', ['watch-bundlejs', 'watch-globe', 'watch-templates', 'watch-less-stylus'], function(){
	console.log("Watch directory for changes");
});

gulp.task('bundling', function() {
    gulp.start('bundlejs');
    gulp.start('bundlecss');
})

gulp.task('build', [
		'bundling',
		'build-globe',
		'build-templates'], function(){
	console.log("Build process");
});


// default task is to run the server and watch for changes
gulp.task('default', [ 'serve' ], function () {
    gulp.start('watch');
});
