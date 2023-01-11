# WorldviewHost

Application host and static content delivery server.

Before starting on Windows:
	Install node from https://nodejs.org (select V5.5.0 Stable) - this will also install the npm package manager. If you have it installed already, make sure you are running an npm version higher than 3.0 - check via running 'npm --version'.  See here for more:  http://www.felixrieseberg.com/npm-v3-is-out-and-its-a-really-big-deal-for-windows/
	Install the gulp build tools globally with: 'npm install -g gulp'

Get Started via Command Line:
	Navigate to the project directory with the command line.
	Install npm packages with: 'npm install'
	Run gulp dev server and watcher with a single command: 'gulp'
	Open your browser at: http://localhost:52023

	If you make changes to the server code you will need to restart the gulp process (this may change soon).

Get Started via Visual Studio:
    Install node.js tools 1.1 for visual studio (Tools->Extensions & Updates and search for "node")
	Open the solution. Install packages by right clicking on npm and selecting "Install missing packages".
	Run with F5 developer tools, which will call "node watch.js". 
	You will need to stop the server and restart if changes are made to server code (everything else will update on browser refresh).
	Open your browser at: http://localhost:52023

	See this page for the official documentation from Microsoft:
	https://github.com/Microsoft/nodejstools



Application Structure
	Everything you need to know is in app.js - the application is very basic, within this file we import other apps as middleware.

	A very basic example can be seen with the viewer application. The folder contains two files: an index.ejs template and server.js. The server here binds a route '/:id' to the relative route passed in from app.js via: app.use('/view', require('./viewer/server'));  this will allow a route on the server of  "/view/:id".

	The embedded javascript "ejs" templates are very basic - use your javascript code within <% %>  and <%= %>  brackets:  http://www.embeddedjs.com/


	/client			← library code from /Contents/Scripts
	/server			← server code for routes/views etc.
	/site			← the website and gallery codebase
	/studio			← studio and web-studio templates
	/demo			← demo app
	/web-globe		← migrate from WebGlobe project
	/viewer			← standalone viewer app
	/embed			← embeddable app
	/build			← build output location
	/bin 			← project binaries
	/app.js


	The project has been split up into self-contained projects and application entry points, but most of the code will reside in /client.

	The /client folder is the equivalent of the old /Contents/Scripts folder but with all the PYXIS javascript assets moved within it. Any vendor scripts should be moved to the /client/assets/scripts folder.

	Static assets are mounted at /assets, which will point first to /client/assets and then to /site/assets and /web-globe/assets  (and down the chain if any other modules are added). The reasoning for this is to allow a universal path to an asset via "/assets/<asset_filename>" that can be accessed no matter where the current page is mounted. Using relative paths breaks down as soon as you have routes at subdirectories.

	The /web-globe project is set to compile it's output to the /build directory, other than that it is a 1-1 copy of the code from the Web-Globe git repo.



Build Tool Setup
	You must have "gulp" installed to execute the build tasks, install it with: 

	The build tools are all loaded from the gulpfile.js at the root, which loads scripts in the '/server/build' folder.

	Run "gulp build" to build the static assets needed for the site, which will do the following:
	- reads 'server/bundles.js' to bundle and minify all javascript code
	- compiles 'less' files and 'stylus' files into compressed css
	- packages angular templates into a template cache
	- compiles webglobe coffeescript to js
	- builds the webglobe bundle with browserify

	The "gulp build" command is set to execute as a Visual Studio PostBuildEvent. You might need to do "Rebuild" on VS in order to force 'gulp build' run.


Azure Deployment
	The server is setup to run custom operations on deployment. The script in ./bin/deploy.cmd specifies a number of steps to be executed after deployment including 'npm install' and 'gulp build'. 

	These were all generated using the "azure-cli" tools which can be installed via:  'npm install -g azure-cli'

