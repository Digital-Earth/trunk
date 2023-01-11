###
	Grunt build system
###
module.exports = (grunt) ->

	config =
		pkg: grunt.file.readJSON 'package.json'
		watch:
			stylus:
				files: ['css/*.styl', 'css/*/*.styl']
				tasks: ['stylus', 'cssmin']
				#tasks: ['stylus','cssmin','sftp-deploy']

			coffee:
				files: ['server.coffee', 'js/*.coffee', 'vendor/*.coffee']
				tasks: [ 'coffee', 'concat' ]

	
				
		stylus:
			compile:
   			 	files:
   			 		'css/app.css' : 'css/app.styl'
   			options:
   			 	compress : false 



		coffee:
			compile:
				files:
					'js/app.js' : 'js/app.coffee'
					'server.js': 'server.coffee'


		csslint: 
			lax: 
				options: 
					import: false
				src: ['css/*.css']

		cssmin:
			dist: 
				# src: [ 'css/utility/normalize.css',
				# 'css/utility/boxmodel.css',
				# 'css/structure.dev.css'
				# ]
				src: ['css/normalize.css', 'css/app.css'] 
				dest: 'css/development.css'
				
		concat:
			dist:
				dest: 'js/development.js'
				src: [
					'js/vendor/underscore-min.js'
					'js/vendor/jquery-2.1.3.min.js'
					'js/app.js'
					]
		uglify:
			build:
				src: 'js/development.js'
				dest: 'js/build.js'

		
		
	grunt.initConfig config

	grunt.loadNpmTasks 'grunt-contrib-coffee'
	grunt.loadNpmTasks 'grunt-contrib-concat'
	grunt.loadNpmTasks 'grunt-contrib-uglify'
	grunt.loadNpmTasks 'grunt-contrib-stylus'
	grunt.loadNpmTasks 'grunt-contrib-cssmin'
	grunt.loadNpmTasks 'grunt-contrib-csslint'
	grunt.loadNpmTasks 'grunt-contrib-watch'
	grunt.loadNpmTasks 'grunt-contrib-coffee'
	grunt.loadNpmTasks 'grunt-contrib-jst'


