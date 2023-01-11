/**

	1) Determine if the browser supports the globe
	2) If So, load the web-globe build and then pull everything in

*/


var files = [
	"/client/pyxis/pyxis.js",
	"/client/pyxis/pyxis.user.js",
	"/client/pyxis/pyxis.gallery.js",
	"/client/pyxis/pyxis.area.js",
	"/client/flux/dispatcher.js",
	"/client/analytics.js",
	"/client/pyxis-ui.js",
	// "/client/studio.js",
	"/client/studioConfig.js",
	"/client/site/directives.js",
	"/client/studio/directives.js",
	"/client/studio/skin/bootstrap.js",
	"/client/studio/skin/demo.js",
	"/client/studio/demos/loop.js",
	"/client/studio/demos/climate.js",
	"/demo/interface.js",
	"/client/studio/legacyservices/connectionIndex.js",
	"/client/studio/legacyservices/delayEvent.js",
	"/client/studio/legacyservices/geocodeService.js",
	"/client/studio/legacyservices/importService.js",
	"/client/studio/legacyservices/localSearch.js",
	"/client/studio/legacyservices/mapFactory.js",
	"/client/studio/legacyservices/numberUtils.js",
	"/client/studio/legacyservices/ogcService.js",
	"/client/studio/legacyservices/paletteService.js",
	"/client/studio/legacyservices/positionHelper.js",
	"/client/studio/legacyservices/searchServices.js",
	"/client/studio/legacyservices/studioConsole.js",
	"/client/studio/legacyservices/styleOptions.js",
	"/client/studio/legacyservices/userPolling.js",
	"/client/studio/legacyservices/userUsageTracker.js",
	"/client/studio/legacyservices/whereFilter.js",
	"/client/studio/features/aboutDialog.js",
	"/client/studio/features/authentication.js",
	"/client/studio/features/currentMap.js",
	"/client/studio/features/dashboard.js",
	"/client/studio/features/editResources.js",
	"/client/studio/features/feedback.js",
	"/client/studio/features/firstUse.js",
	"/client/studio/features/fps.js",
	"/client/studio/features/globeHotKeys.js",
	"/client/studio/features/goto.js",
	"/client/studio/features/helpCenter.js",
	"/client/studio/features/import.js",
	"/client/studio/features/inspectorTools.js",
    "/client/studio/features/intercomTrack.js",
	"/client/studio/features/library.js",
	"/client/studio/features/localPublish.js",
	"/client/studio/features/mosaic.js",
	"/client/studio/features/myConnections.js",
	"/client/studio/features/notifications.js",
	"/client/studio/features/propertiesWindow.js",
	"/client/studio/features/search.js",
	"/client/studio/features/selectionTools.js",
	"/client/studio/features/style.js",
	"/client/studio/features/walkThrough.js",
	"/client/studio/services/globeService.js",
	"/client/studio/services/resourceResolver.js",
	"/client/studio/services/selectionViewer.js",
	"/client/studio/stores/currentMapStore.js",
	"/client/studio/stores/resourcesStore.js",
	"/client/studio/stores/searchTagsStore.js",
	"/client/studio/stores/selectionStore.js"
	// "/build/studio-templates.js"
	];



var DEMO_UI = [
'<div ng-repeat="url in templates.sections" ng-include="url"></div>',
'<div ng-repeat="url in templates.dialogs" ng-include="url"></div>',
'<div ng-repeat="url in templates.widgets" ng-include="url"></div>',

'<modal-dialog modal-background-class="no-background" modal-class="demo" ng-if="state.activeStep.message">',
'    <h1>{{state.activeStep.message.title}}</h1>',
'    <p>{{state.activeStep.message.content}}</p>',
'</modal-dialog>',

'<div style="position: fixed; left: 300px; bottom: 100px; right: 300px">',
'    <i class="fa timeline-play-pause" ng-click="state.play = !state.play" ng-class="{\'fa-play\':state.play, \'fa-pause\':!state.play}"></i>',
'    <time-line time-line="timeLine" sections="sections" state="state" on-time-line-click="onTimeLineClick($time)"></time-line>',
'    <span class="timeline-elapsed-time-display">{{state.currentTime.toFixed(2)}}</span>',
'</div>',
].join('')





$.getMultiScripts = function(arr, path) {
    var _arr = $.map(arr, function(scr) {
        return $.getScript( (path||"") + scr );
    });

    _arr.push($.Deferred(function( deferred ){
        $( deferred.resolve );
    }));

    return $.when.apply($, _arr);
}

// allow for cached files
$.ajax({cache: true});


app.directive('landingGlobe', function ($timeout, $window, $parse, Modernizr, ViewportCutoffs) {

	function postlink(scope, element, attributes){
		var webglSupported = true;
		var canvas;
		var ctx;
		var exts;

		try {
			canvas = createElement('canvas');
			ctx = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
			exts = ctx.getSupportedExtensions();
		}
		catch (e) { }

		if (ctx !== undefined) {
			webglSupported = true;
		}


		console.log("LANDING GLOBE ", webglSupported);


		if (webglSupported){

			console.log("WEBGL supported");

			// hide the static globe so there isn't two
			$('.banner__globe').hide(); 

			// bring in text from below
			TweenMax.from('.banner__header', 1.4, {y: '+=200', opacity: 0, ease: Quad.easeOut});


			// we need to disable getScript from appending a random number to query
			$.ajaxSetup({cache: true});

			$.getScript('/build/web-globe-bundle.js').done(function(){

				$('.banner').css({height: $(window).height()})
				
				console.log("Setup web globe");


				element.css({
					position: 'absolute',
				    top: 0,
				    left: 0,
				    height: '100%',
				    width: '100%',
				    opacity: 0
				});

				$('.header').css('background-color', 'transparent');


				

				// keep window references for debugging, remove later
				window.GC = new window.PYXIS.GlobeCanvas(element[0], 'globe-loader', {
				    disableZoomControls: true,
				    defaultTheme: 'landingGlobeConfig' 
				});

				// hide stats and dat-gui
				$('#stats, .dg').hide();


				window.GC.scene.cameraController.animateCamera({
						"Tilt":0.00003199088826022489,
						"Heading":9.342414354297793,
						"Latitude":-3.3147970622125937,
						"Longitude":-73.57307499134482,
						"Altitude":0,
						"Range":31362120.10535658
					}, 1);

				// fade in the globe
				TweenMax.to(element, 3.0, {opacity: 1.0, ease: Quad.easeOut, delay: 1.8});

				function animateCamera(){
					window.GC.scene.cameraController.animateCamera({ 
						"Tilt": 0.00005708803308834831, 
						"Heading": 7.444434175076949, 
						"Latitude": 24.325189786472162, 
						"Longitude": 63.97448440588935, 
						"Altitude": 0, 
						"Range": 12053978.35941361
					}, 10000);
				}
				
				setTimeout(animateCamera, 1000);




				// instead of loading the site at /demo we drop out the rest of the site
				// and then
				console.log("HAS DEMO LINK ?", $('#demo-link').length);
				$('#demo-link').click(function(e){
					e.preventDefault();
					e.stopPropagation();

					console.log("SETUP DEMO");

					

					// use prerender to load demo page before we go there
					// $('head').append("<link rel='prerender' href='/demo'>");

					// scroll to the top of the page
					$(window).scrollTop(0);

					// preload some of the lighting images so they don't pop in
					var images = [
						'/assets/images/textures/matcaps/globe_matcap_black_white_smooth.png',
						'/assets/images/textures/matcaps/globe_matcap_silver_bright.png'
					];
					_.each(images, function(url){
						$('body').append("<img src='"+ url +"' style='position: absolute; left: -99999px; top: -99999px; opacity: 0;'>");
					})


					// this will get set to the startAnimation function when the demo assets have
					// loaded
					window.startDemoAnimation = null;
					var hasCompletedTransition = false;

					function startDemo(){
						window.location = '/view/a305117d-b744-4c35-93a3-c50e2e994aae';
						// if (window.startDemoAnimation) window.startDemoAnimation();
						// hasCompletedTransition = true;
					}

					// just go to the link
					startDemo();


					// this camera position will match the first camera position in the demo
					var cameraPos = { "Tilt": 0.00005708803308834831, "Heading": 7.444434175076949, "Latitude": 24.325189786472162, "Longitude": -63.97448440588935, "Altitude": 0, "Range": 8600575.872065304 };

					window.GC.scene.cameraController.animateCamera(cameraPos, 2000, startDemo);


					// rebind the zoom controls
					window.GC.bindZoomControls();


					// fade out the text overlays and menu
					TweenMax.to('.banner__header', 1.0, {
						opacity: 0.0, 
						y:'-=200', 
						ease: Quad.easeIn,
						onComplete: function(){
							$('.banner > section').remove();
						}
					});

					TweenMax.to('nav.menu', 0.2, {
						opacity: 0.0, 
						delay: 0.6,
						y:'-=100', 
						ease: Quad.easeIn,
						onComplete: function(){
							$('nav.menu').hide();
						}
					});

					// get rid of all the rest of the web content
					$('.banner').css('margin-bottom', 0);
					$('.discover, .tour, .subscribe, .updates, .pusher, .footer').remove();



					// setTimeout(function(){window.location = "/demo";}, 6000);

					// append the rest of the scripts
					// $('head').append(demoDOM);
					// console.log("LOAD STUDIO DEMO SCRIPTSx");
					// $.getScript( "/build/studio.js")
					// 	.done(function(){ console.log("STUDIO LOADED");})
					// 	.fail(function(data, status, jqxhr){ console.log("FAILED Load ", data, status, jqxhr);});

					// $.when(
					//     // $.getScript( "/bundles/studio-frameworks.js" ),
					//     // $.getScript( "/bundles/pyxis.js" ),
					//     $.getScript( "/bundles/studio.js" ),
					//     // $.getScript( "/bundles/studio-localization.js" ),
					//     // $.getScript( "/build/web-globe-bundle.js" ),
					//     // $.getScript( "/demo/interface.js" ),
					//     // $.getScript( "/bundles/studio-templates.js" ),
					//     // $.getScript( "https://maps.googleapis.com/maps/api/js" )
					// ).done(function(){

					//     //place your code here, the scripts are all loaded
					//     console.log("ALL DEMO SCRIPTS LOADED -- inject studioShim");
					//     var injector = angular.element(".ng-scope").injector();
					// 	injector.get("studioShim");

					// });


					if (window.BVersion){

						$('#demo-chooser').show();

						var currentMap = window.map1;

						window.getCurrentMapModel = function () {
				            return currentMap;
				        };

				        window.getCurrentMap = function () {
				            return angular.toJson(window.getCurrentMapModel());
				        }


						function loadMap(map){

							var layers = [];
							_.each(map.Groups, function(group){
								_.each(group.Items, function(geoSource){
									geoSource.Id = geoSource.Resource.Id;
									layer = {
										geoSource: geoSource,
										style: geoSource.Style
									}
									layers.push(layer);
								});
							});

							window.GC.preloadLayers(layers, {}, function(){
								window.GC.doSwapScene();
								currentMap = map;
							});
							// window.GC.loadLayers(layers);
						}
						
						window.loadMap = loadMap;

						loadMap(window.map1);
						return;
					}



					$('head').append('<link rel="stylesheet" type="text/css" href="/assets/styles/studio.css"/>');


					//$('head').append(DOMP);
					// $('html').attr('ng-controller', 'worldViewStudioController');
					$.getMultiScripts(files).done(function(){
						console.log("DONE LOADING !!!");


						app.provider({
							$rootElement:function() {
								this.$get = function() {
								return angular.element('<div ng-app></div>');
								};
							}
						});

						var $injector = angular.injector(['worldViewSite']);
						

						var $scope = angular.element(document).scope(),
							$pyx = $injector.get('$pyx'), 
							$pyxconfig = $injector.get('$pyxconfig'), 
							$pyxuser = $injector.get('$pyxuser'),
							$interval =  $injector.get('$interval'),
							worldViewStudioBootstrap = $injector.get('worldViewStudioBootstrap'),
							featureWalkThrough = $injector.get('featureWalkThrough'),
							featureInspectorTools = $injector.get('featureInspectorTools'),
							demoClimate = $injector.get('demoClimate'),
							demoLoop = $injector.get('demoLoop'),
							studioShim = $injector.get('studioShim');

						// assign vars
						window.$injector = $injector;
						window.$pyx = $pyx;

						
						$injector.invoke(function() {
 							console.log("INVOKE DEMO SETUP ");

							worldViewStudioBootstrap($scope, { mode: 'demo' });

							featureInspectorTools.register($scope);

							$scope.demo = true;
					
						    $scope.timeLine = [];   

						    $scope.timeLine.push({
						        duration: 10,
						        start: 0,
						        map: { "Type": "Map", "Metadata": { "Name": "New Map", "Theme": { "name": "Light Theme", "specular": 0.4, "shininess": 16, "diffuseStrength": 2, "lightingEnable": 0, "dataMin": 0, "dataMax": 9000, "backgroundColor": "#f0f0f0", "environmentMultiplier": 1.5839464882943144, "heightMultiplier": 1, "normalmapStrength": 1, "landOnly": 1, "gammaValue": 2.946397660560254, "colorBoost": "#000000", "colorTint": "#898989", "tiltshiftValue": 0, "tiltshiftRes": 0.45, "rangeMax": 19260263, "rangeMin": 24594, "latMax": -1, "latMin": -1, "lonMax": -1, "lonMin": -1, "fogColor": "#dcdcdc", "fogIntensity": 0.5, "fogHeightOffset": -200, "fogBlendCoef": 3.2, "fogCameraDistCoef": 5.6, "color0": "#525252", "color1": "#2f2f2f", "color2": "#aa3232", "color3": "#ff9d11", "color4": "#ff4b00", "color5": "#dc8320", "lightIntensity": 0, "roughMatcap": "assets/images/textures/matcaps/globe_matcap_silver.png", "glossMatcap": "assets/images/textures/matcaps/globe_matcap_silver_bright.png", "noiseMap": "assets/images/textures/noisemap512.jpg", "globeBackground": "#898989", "noiseMultiplier": 0, "basemapRoughness": 0.6100334448160535, "basemapLightingMultiplier": 2, "surfaceRoughness": 1, "surfaceLightingMultiplier": 0, "layer0Alpha": 1, "layer0Tint": "#5f5f5f", "layer1Alpha": 0.7002688573219419, "layer1Tint": "#ffffff", "layer2Alpha": 1, "layer2Tint": "#777777", "layer3Alpha": 1, "layer3Tint": "#777777", "layer4Alpha": 1, "layer4Tint": "#777777", "layer5Alpha": 1, "layer5Tint": "#777777", "layer6Alpha": 1, "layer6Tint": "#777777", "layer7Alpha": 1, "layer7Tint": "#777777", "layer8Alpha": 1, "layer8Tint": "#777777", "layer9Alpha": 1, "layer9Tint": "#777777", "iconAlpha": 1, "iconBorderColor": "#000000", "defaultRenderLayers": [{ "geoSource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Specification": { "OutputType": "Coverage" } }, "style": { "ShowAsElevation": true, "Fill": { "Style": "Palette", "PaletteExpression": "RGB", "Palette": { "Steps": [{ "Value": -20, "Color": "rgba(0,0,0,0.0)" }, { "Value": 0, "Color": "#000000" }, { "Value": 3341, "Color": "#111111" }, { "Value": 5052, "Color": "#111111" }] } } } }], "defaultMapGroupObject": { "Metadata": { "Name": "Theme Basemap" }, "Items": [{ "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "c509bb71-192b-4fa9-aa58-58ab58f43473", "Name": "Pyxis" }, "Providers": [{ "Type": "Gallery", "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557", "Name": "Microsoft Bing Maps" }], "Category": "Demo", "Created": "2014-05-05T21:21:34Z", "Updated": "2015-01-11T00:09:40.058Z", "BasedOnVersion": "1d6a3b01-50ef-4d80-906c-9e1524d77c32", "Visibility": "Public", "Name": "Global Elevation (GTOPO30)", "Description": "GTOPO30..", "Tags": ["Global", "Elevation"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "https://www.pyxisinnovation.com/images/pipelines/8be6a2ec-5110-49cf-a295-1008f8e9a21b.jpg" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Number", "FieldUnit": { "Name": "m" }, "Metadata": { "Name": "Elevation", "Description": "", "SystemTags": ["Favorite"] }, "Starred": true, "OnDashboard": false, "Value": 1848 }] }, "Resource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Type": "GeoSource", "Version": "0329caed-c92e-49c7-b7ce-d0b81af25d97" }, "Style": { "Fill": { "PaletteExpression": "RGB", "Style": "Palette", "Palette": { "Steps": [{ "Value": -20, "Color": "rgba(0,0,0,0.0)" }, { "Value": 0, "Color": "#000000" }, { "Value": 3341.39, "Color": "#111111" }, { "Value": 5052.27, "Color": "#111111" }] }, "Color": null }, "ShowAsElevation": true }, "Active": true, "ShowDetails": true, "Legend": { "Expanded": true } }] } } }, "Expanded": true, "Groups": [{ "Metadata": { "Name": "" }, "Items": [{ "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "c509bb71-192b-4fa9-aa58-58ab58f43473", "Name": "Pyxis" }, "Providers": [{ "Type": "Gallery", "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557", "Name": "Microsoft Bing Maps" }], "Category": "Demo", "Created": "2014-05-05T21:21:34Z", "Updated": "2015-01-11T00:09:40.058Z", "BasedOnVersion": "1d6a3b01-50ef-4d80-906c-9e1524d77c32", "Visibility": "Public", "Name": "Global Elevation (GTOPO30)", "Description": "GTOPO30 is a global digital elevation model (DEM) with a horizontal grid spacing of 30 arc seconds (approximately 1 kilometer). GTOPO30 was derived from several raster and vector sources of topographic information.\r\n\r\nGTOPO30, completed in late 1996, was developed over a three year period through a collaborative effort led by staff at the U.S. Geological Survey's Center for Earth Resources Observation and Science (EROS). The following organizations participated by contributing funding or source data:  the National Aeronautics and Space Administration (NASA), the United Nations Environment Programme/Global Resource Information Database (UNEP/GRID), the U.S. Agency for International Development (USAID), the Instituto Nacional de Estadistica Geografica e Informatica (INEGI) of Mexico, the Geographical Survey Institute (GSI) of Japan, Manaaki Whenua Landcare Research of New Zealand, and the Scientific Committee on Antarctic Research (SCAR).\r\n\r\nsource: https://lta.cr.usgs.gov/GTOPO30", "Tags": ["Global", "Elevation"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "https://www.pyxisinnovation.com/images/pipelines/8be6a2ec-5110-49cf-a295-1008f8e9a21b.jpg" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Number", "FieldUnit": { "Name": "m" }, "Metadata": { "Name": "Elevation", "Description": "", "SystemTags": [] } }] }, "Resource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Type": "GeoSource", "Version": "0329caed-c92e-49c7-b7ce-d0b81af25d97" }, "Style": { "Fill": { "PaletteExpression": "RGB", "Style": "Palette", "Palette": { "Steps": [{ "Value": -200, "Color": "rgba(0,0,0,0.0)" }, { "Value": -0.38, "Color": "#808080" }, { "Value": 9000, "Color": "#808080" }] }, "Color": null }, "ShowAsElevation": true }, "Active": true, "ShowDetails": true, "Legend": { "Expanded": true } }] }], "Dashboards": [{ "Widgets": [] }], "EmbeddedResources": [] },
						        camera: { "Tilt": 0.00005708803308834831, "Heading": 7.444434175076949, "Latitude": 24.325189786472162, "Longitude": -63.97448440588935, "Altitude": 0, "Range": 8600575.872065304 },
						        message: { title: "Welcome", content: "This is the future. " }
						    });
						    $scope.timeLine.push({
						        duration: 10,
						        start: 10,
						        name: "Purple",
						        color: "#808",
						        map: { "Type": "Map", "Metadata": { "Name": "New Map", "Theme": { "name": "Light Theme", "specular": 0.4, "shininess": 16, "diffuseStrength": 2, "lightingEnable": 0, "dataMin": 0, "dataMax": 9000, "backgroundColor": "#f0f0f0", "environmentMultiplier": 1.3316588106450042, "heightMultiplier": 1, "normalmapStrength": 1, "landOnly": 1, "gammaValue": 2.3822154383000673, "colorBoost": "#000000", "colorTint": "#828282", "tiltshiftValue": 0, "tiltshiftRes": 0.45, "rangeMax": 19260263, "rangeMin": 24594, "latMax": -1, "latMin": -1, "lonMax": -1, "lonMin": -1, "fogColor": "#dcdcdc", "fogIntensity": 0.5, "fogHeightOffset": -200, "fogBlendCoef": 3.2, "fogCameraDistCoef": 5.6, "color0": "#525252", "color1": "#2f2f2f", "color2": "#aa3232", "color3": "#ff9d11", "color4": "#ff4b00", "color5": "#dc8320", "lightIntensity": 0, "roughMatcap": "assets/images/textures/matcaps/globe_matcap_black_white_smooth.png", "glossMatcap": "assets/images/textures/matcaps/globe_matcap_black_white_smooth.png ", "noiseMap": "assets/images/textures/noisemap512.jpg", "globeBackground": "#ffffff", "noiseMultiplier": 0.2, "basemapRoughness": 1, "basemapLightingMultiplier": 0, "surfaceRoughness": 1, "surfaceLightingMultiplier": 1, "layer0Alpha": 0.5969505013236226, "layer0Tint": "#f0f0f0", "layer1Alpha": 0.45919269332586354, "layer1Tint": "#ffffff", "layer2Alpha": 1, "layer2Tint": "#777777", "layer3Alpha": 1, "layer3Tint": "#777777", "layer4Alpha": 1, "layer4Tint": "#777777", "layer5Alpha": 1, "layer5Tint": "#777777", "layer6Alpha": 1, "layer6Tint": "#777777", "layer7Alpha": 1, "layer7Tint": "#777777", "layer8Alpha": 1, "layer8Tint": "#777777", "layer9Alpha": 1, "layer9Tint": "#777777", "iconAlpha": 1, "iconBorderColor": "#000000", "defaultRenderLayers": [{ "geoSource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Specification": { "OutputType": "Coverage" } }, "style": { "ShowAsElevation": true, "Fill": { "Style": "Palette", "PaletteExpression": "RGB", "Palette": { "Steps": [{ "Value": -20, "Color": "rgba(0,0,0,0.0)" }, { "Value": 0, "Color": "#000000" }, { "Value": 3341, "Color": "#111111" }, { "Value": 5052, "Color": "#111111" }] } } } }], "defaultMapGroupObject": { "Metadata": { "Name": "Theme Basemap" }, "Items": [{ "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "c509bb71-192b-4fa9-aa58-58ab58f43473", "Name": "Pyxis" }, "Providers": [{ "Type": "Gallery", "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557", "Name": "Microsoft Bing Maps" }], "Category": "Demo", "Created": "2014-05-05T21:21:34Z", "Updated": "2015-01-11T00:09:40.058Z", "BasedOnVersion": "1d6a3b01-50ef-4d80-906c-9e1524d77c32", "Visibility": "Public", "Name": "Global Elevation (GTOPO30)", "Description": "GTOPO30..", "Tags": ["Global", "Elevation"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "https://www.pyxisinnovation.com/images/pipelines/8be6a2ec-5110-49cf-a295-1008f8e9a21b.jpg" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Number", "FieldUnit": { "Name": "m" }, "Metadata": { "Name": "Elevation", "Description": "", "SystemTags": ["Favorite"] }, "Starred": true, "OnDashboard": false, "Value": 1848 }] }, "Resource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Type": "GeoSource", "Version": "0329caed-c92e-49c7-b7ce-d0b81af25d97" }, "Style": { "Fill": { "PaletteExpression": "RGB", "Style": "Palette", "Palette": { "Steps": [{ "Value": -20, "Color": "rgba(0,0,0,0.0)" }, { "Value": 0, "Color": "#000000" }, { "Value": 3341.39, "Color": "#111111" }, { "Value": 5052.27, "Color": "#111111" }] }, "Color": null }, "ShowAsElevation": true }, "Active": true, "ShowDetails": true, "Legend": { "Expanded": true } }] } } }, "Expanded": true, "Groups": [{ "Metadata": { "Name": "" }, "Items": [{ "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "c509bb71-192b-4fa9-aa58-58ab58f43473", "Name": "Pyxis" }, "Providers": [{ "Type": "Gallery", "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557", "Name": "Microsoft Bing Maps" }], "Category": "Demo", "Created": "2014-05-05T21:21:34Z", "Updated": "2015-01-11T00:09:40.058Z", "BasedOnVersion": "1d6a3b01-50ef-4d80-906c-9e1524d77c32", "Visibility": "Public", "Name": "Global Elevation (GTOPO30)", "Description": "GTOPO30 is a global digital elevation model (DEM) with a horizontal grid spacing of 30 arc seconds (approximately 1 kilometer). GTOPO30 was derived from several raster and vector sources of topographic information.\r\n\r\nGTOPO30, completed in late 1996, was developed over a three year period through a collaborative effort led by staff at the U.S. Geological Survey's Center for Earth Resources Observation and Science (EROS). The following organizations participated by contributing funding or source data:  the National Aeronautics and Space Administration (NASA), the United Nations Environment Programme/Global Resource Information Database (UNEP/GRID), the U.S. Agency for International Development (USAID), the Instituto Nacional de Estadistica Geografica e Informatica (INEGI) of Mexico, the Geographical Survey Institute (GSI) of Japan, Manaaki Whenua Landcare Research of New Zealand, and the Scientific Committee on Antarctic Research (SCAR).\r\n\r\nsource: https://lta.cr.usgs.gov/GTOPO30", "Tags": ["Global", "Elevation"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "https://www.pyxisinnovation.com/images/pipelines/8be6a2ec-5110-49cf-a295-1008f8e9a21b.jpg" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Number", "FieldUnit": { "Name": "m" }, "Metadata": { "Name": "Elevation", "Description": "", "SystemTags": [] } }] }, "Resource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Type": "GeoSource", "Version": "0329caed-c92e-49c7-b7ce-d0b81af25d97" }, "Style": { "Fill": { "PaletteExpression": "RGB", "Style": "Palette", "Palette": { "Steps": [{ "Value": -200, "Color": "rgba(0,0,0,0.0)" }, { "Value": -69.09, "Color": "#9126FF" }, { "Value": 1616.36, "Color": "#247BFE" }, { "Value": 3500, "Color": "#A6E1FF" }, { "Value": 4705.82, "Color": "#33E0FE" }, { "Value": 7000, "Color": "#00C305" }] }, "Color": null }, "ShowAsElevation": true }, "Active": true, "ShowDetails": true, "Legend": { "Expanded": true } }, { "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "e5d51f7e-32f3-4916-ad53-dfe3faebd369", "Name": "ppeterson" }, "Providers": [{ "Type": "Gallery", "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557", "Name": "Microsoft Bing Maps" }], "Category": "Demo", "Created": "2014-05-06T05:26:30.561Z", "Updated": "2014-07-03T22:04:54.894Z", "BasedOnVersion": "5f75efbb-0386-4362-8cbf-10e90cee9950", "Visibility": "Public", "Name": "Global Base Imagery - Terra Pixel", "Description": "(C) 2013 Microsoft Digital Globe - for non commercial demonstration only", "Tags": ["Imagery", "WebService"], "SystemTags": ["WebService"], "ExternalUrls": [{ "Type": "Image", "Url": "https://www.pyxisinnovation.com/images/pipelines/ac8436e7-e492-4bed-ba17-e2007f6d1099.jpg" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Color", "FieldUnit": null, "Metadata": { "Comments": null, "Ratings": null, "User": null, "Providers": null, "Category": null, "Created": "0001-01-01T00:00:00", "Updated": "0001-01-01T00:00:00", "BasedOnVersion": null, "Visibility": null, "Name": "RGB", "Description": "", "Tags": null, "SystemTags": null, "ExternalUrls": null }, "ValueTranslation": null }] }, "Resource": { "Id": "ac8436e7-e492-4bed-ba17-e2007f6d1099", "Type": "GeoSource", "Version": "46958385-0c92-4c71-b708-1493be991900" }, "Style": { "ShowAsElevation": false }, "Active": true, "ShowDetails": true }] }], "Dashboards": [{ "Widgets": [] }], "EmbeddedResources": [] },
						        camera: { "Tilt": 0.00008232166783272987, "Heading": 3.2274335143307478, "Latitude": -4.8492399062998865, "Longitude": -66.38478511135592, "Altitude": 0, "Range": 16648243.617421042 },
						        message: { title: "DGGS", content: "The ability to integrate any geospatial information on everywhere on the globe. " }
						    });
						    $scope.timeLine.push({
						        duration: 5,
						        start: 20,
						        map: { "Type": "Map", "Metadata": { "Name": "New Map" }, "Expanded": true, "Groups": [{ "Metadata": { "Name": "" }, "Items": [{ "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "23119ff4-b1c8-47a9-9817-65deb2d02a4e", "Name": "JoJonesMaps" }, "Providers": [{ "Type": "Gallery", "Id": "5b434fa1-b578-4eb2-81c5-a823377aa931", "Name": "JoJonesMaps" }], "Category": "SDK", "Created": "2015-09-27T02:50:11.578Z", "Updated": "2015-09-27T02:50:11.578Z", "BasedOnVersion": "20571fad-20f4-4869-b5fe-ebca54cbd890", "Visibility": "Public", "Name": "Land Surface, Ocean Colour, Sea Ice, and Clouds", "Description": "This spectacular “blue marble” image is the most detailed true-color image of the entire Earth to date. Using a collection of satellite-based observations, scientists and visualizers stitched together months of observations of the land surface, oceans, sea ice, and clouds into a seamless, true-color mosaic of every square kilometer (.386 square mile) of our planet. \n\nCredit: NASA Goddard Space Flight Center Image by Reto Stöckli (land surface, shallow water, clouds). Enhancements by Robert Simmon (ocean color, compositing, 3D globes, animation). Data and technical support: MODIS Land Group; MODIS Science Data Support Team; MODIS Atmosphere Group; MODIS Ocean Group Additional data: USGS EROS Data Center (topography); USGS Terrestrial Remote Sensing Flagstaff Field Center (Antarctica); Defense Meteorological Satellite Program (city lights).", "Tags": ["Clouds", "Land", "Ice", "Earth", "Ocean"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "http://www.pyxisinnovation.com/images/pipelines/ced24668-a781-4abe-b86b-a7a98c43c165.png" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Color", "Metadata": { "Name": "RGB", "Description": "", "SystemTags": [] } }] }, "Resource": { "Id": "ced24668-a781-4abe-b86b-a7a98c43c165", "Type": "GeoSource", "Version": "365046f3-1fee-4e65-bb30-13c9902279fd" }, "Style": { "ShowAsElevation": false }, "Active": true, "ShowDetails": true }, { "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "c509bb71-192b-4fa9-aa58-58ab58f43473", "Name": "Pyxis" }, "Providers": [{ "Type": "Gallery", "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557", "Name": "Microsoft Bing Maps" }], "Category": "Demo", "Created": "2014-05-05T21:21:34Z", "Updated": "2015-01-11T00:09:40.058Z", "BasedOnVersion": "1d6a3b01-50ef-4d80-906c-9e1524d77c32", "Visibility": "Public", "Name": "Global Elevation (GTOPO30)", "Description": "GTOPO30 is a global digital elevation model (DEM) with a horizontal grid spacing of 30 arc seconds (approximately 1 kilometer). GTOPO30 was derived from several raster and vector sources of topographic information.\r\n\r\nGTOPO30, completed in late 1996, was developed over a three year period through a collaborative effort led by staff at the U.S. Geological Survey's Center for Earth Resources Observation and Science (EROS). The following organizations participated by contributing funding or source data:  the National Aeronautics and Space Administration (NASA), the United Nations Environment Programme/Global Resource Information Database (UNEP/GRID), the U.S. Agency for International Development (USAID), the Instituto Nacional de Estadistica Geografica e Informatica (INEGI) of Mexico, the Geographical Survey Institute (GSI) of Japan, Manaaki Whenua Landcare Research of New Zealand, and the Scientific Committee on Antarctic Research (SCAR).\r\n\r\nsource: https://lta.cr.usgs.gov/GTOPO30", "Tags": ["Global", "Elevation"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "https://www.pyxisinnovation.com/images/pipelines/8be6a2ec-5110-49cf-a295-1008f8e9a21b.jpg" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Number", "FieldUnit": { "Name": "m" }, "Metadata": { "Name": "Elevation", "Description": "", "SystemTags": [] } }] }, "Resource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Type": "GeoSource", "Version": "0329caed-c92e-49c7-b7ce-d0b81af25d97" }, "Style": { "Fill": { "PaletteExpression": "RGB", "Style": "Palette", "Palette": { "Steps": [{ "Value": 0, "Color": "#000000" }, { "Value": 9000, "Color": "#FFFFFF" }] }, "Color": null }, "ShowAsElevation": true }, "Active": true, "ShowDetails": true, "Legend": { "Expanded": true } }] }], "Dashboards": [{ "Widgets": [] }], "EmbeddedResources": [] },
						        camera: { "Tilt": 0.00005954420871034927, "Heading": 347.75218911007534, "Latitude": 36.40414238536614, "Longitude": -29.392922662412214, "Altitude": 0, "Range": 11828209.224191863 }
						    });
						    $scope.timeLine.push({
						        duration: 5,
						        start: 25,                
						        map: { "Type": "Map", "Metadata": { "Name": "New Map" }, "Expanded": true, "Groups": [{ "Metadata": { "Name": "" }, "Items": [{ "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "23119ff4-b1c8-47a9-9817-65deb2d02a4e", "Name": "JoJonesMaps" }, "Providers": [{ "Type": "Gallery", "Id": "5b434fa1-b578-4eb2-81c5-a823377aa931", "Name": "JoJonesMaps" }], "Category": "SDK", "Created": "2015-09-27T02:50:11.578Z", "Updated": "2015-09-27T02:50:11.578Z", "BasedOnVersion": "20571fad-20f4-4869-b5fe-ebca54cbd890", "Visibility": "Public", "Name": "Land Surface, Ocean Colour, Sea Ice, and Clouds", "Description": "This spectacular “blue marble” image is the most detailed true-color image of the entire Earth to date. Using a collection of satellite-based observations, scientists and visualizers stitched together months of observations of the land surface, oceans, sea ice, and clouds into a seamless, true-color mosaic of every square kilometer (.386 square mile) of our planet. \n\nCredit: NASA Goddard Space Flight Center Image by Reto Stöckli (land surface, shallow water, clouds). Enhancements by Robert Simmon (ocean color, compositing, 3D globes, animation). Data and technical support: MODIS Land Group; MODIS Science Data Support Team; MODIS Atmosphere Group; MODIS Ocean Group Additional data: USGS EROS Data Center (topography); USGS Terrestrial Remote Sensing Flagstaff Field Center (Antarctica); Defense Meteorological Satellite Program (city lights).", "Tags": ["Clouds", "Land", "Ice", "Earth", "Ocean"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "http://www.pyxisinnovation.com/images/pipelines/ced24668-a781-4abe-b86b-a7a98c43c165.png" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Color", "Metadata": { "Name": "RGB", "Description": "", "SystemTags": [] } }] }, "Resource": { "Id": "ced24668-a781-4abe-b86b-a7a98c43c165", "Type": "GeoSource", "Version": "365046f3-1fee-4e65-bb30-13c9902279fd" }, "Style": { "ShowAsElevation": false }, "Active": true, "ShowDetails": true }, { "Metadata": { "Comments": [], "Ratings": { "Likes": 0, "Dislikes": 0 }, "User": { "Id": "c509bb71-192b-4fa9-aa58-58ab58f43473", "Name": "Pyxis" }, "Providers": [{ "Type": "Gallery", "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557", "Name": "Microsoft Bing Maps" }], "Category": "Demo", "Created": "2014-05-05T21:21:34Z", "Updated": "2015-01-11T00:09:40.058Z", "BasedOnVersion": "1d6a3b01-50ef-4d80-906c-9e1524d77c32", "Visibility": "Public", "Name": "Global Elevation (GTOPO30)", "Description": "GTOPO30 is a global digital elevation model (DEM) with a horizontal grid spacing of 30 arc seconds (approximately 1 kilometer). GTOPO30 was derived from several raster and vector sources of topographic information.\r\n\r\nGTOPO30, completed in late 1996, was developed over a three year period through a collaborative effort led by staff at the U.S. Geological Survey's Center for Earth Resources Observation and Science (EROS). The following organizations participated by contributing funding or source data:  the National Aeronautics and Space Administration (NASA), the United Nations Environment Programme/Global Resource Information Database (UNEP/GRID), the U.S. Agency for International Development (USAID), the Instituto Nacional de Estadistica Geografica e Informatica (INEGI) of Mexico, the Geographical Survey Institute (GSI) of Japan, Manaaki Whenua Landcare Research of New Zealand, and the Scientific Committee on Antarctic Research (SCAR).\r\n\r\nsource: https://lta.cr.usgs.gov/GTOPO30", "Tags": ["Global", "Elevation"], "SystemTags": [], "ExternalUrls": [{ "Type": "Image", "Url": "https://www.pyxisinnovation.com/images/pipelines/8be6a2ec-5110-49cf-a295-1008f8e9a21b.jpg" }] }, "Specification": { "OutputType": "Coverage", "Fields": [{ "Name": "RGB", "FieldType": "Number", "FieldUnit": { "Name": "m" }, "Metadata": { "Name": "Elevation", "Description": "", "SystemTags": [] } }] }, "Resource": { "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b", "Type": "GeoSource", "Version": "0329caed-c92e-49c7-b7ce-d0b81af25d97" }, "Style": { "Fill": { "PaletteExpression": "RGB", "Style": "Palette", "Palette": { "Steps": [{ "Value": 0, "Color": "#000000" }, { "Value": 9000, "Color": "#FFFFFF" }] }, "Color": null }, "ShowAsElevation": true }, "Active": true, "ShowDetails": true, "Legend": { "Expanded": true } }] }], "Dashboards": [{ "Widgets": [] }], "EmbeddedResources": [] },
						        camera: { "Tilt": 0.00006820043071797954, "Heading": 335.7346951090322, "Latitude": 36.633369183827064, "Longitude": -5.760856598212501, "Altitude": 0, "Range": 1033244.0324160167 }
						    });

						    $scope.sections = [
						        {
						            duration: 10,
						            start: 0,
						            name: "Metallic",
						            color: "#888"
						        },
						        {
						            duration: 10,
						            start: 10,
						            name: "Purple",
						            color: "#808"
						        },
						        {
						            duration: 10,
						            start: 20,
						            name: "Clouds",
						            color: "#383",
						        }
						    ]

						    $scope.state = {
						        currentTime: 0,
						        activeStep: undefined,
						        startClockTime: new Date().getTime(),
						        endTime: 30,        
						        loop: true,
						        play: true
						    }


						    window.killDemo = false;

						    function startAnimation() {

						        $interval(function () {
						        	if (window.killDemo) return;

						            if ($scope.state.play) {
						                $scope.state.currentTime = (new Date().getTime() - $scope.state.startClockTime) / 1000.0;
						                if ($scope.state.currentTime > $scope.state.endTime) {
						                    $scope.state.startClockTime = new Date().getTime();
						                    $scope.state.currentTime = 0;
						                }
						            }

						            var activeStep = undefined;
						            angular.forEach($scope.timeLine, function (step) {
						                if (step.start <= $scope.state.currentTime) {
						                    activeStep = step;
						                }
						            });

						            if (activeStep !== $scope.state.activeStep) {
						                $scope.state.activeStep = activeStep;
						                $scope.setCurrentMap($scope.state.activeStep.map);
						                $pyx.globe.setCamera($scope.state.activeStep.camera, $scope.state.activeStep.duration * 0.9 * 1000);
						            }

						        }, 15);
						    }

						    $scope.onTimeLineClick = function (time) {
						        $scope.state.currentTime = time;
						        $scope.state.startClockTime = (new Date().getTime()) - $scope.state.currentTime * 1000;
						    }


						    // used for debugging
						    window.startDemoAnimation = startAnimation;
						    $('#demo-controls').show();
						    
						    // $('body').append(DEMO_UI);


						    $scope.$apply();
						    // $scope.$apply();

						    if (hasCompletedTransition){
						    	startAnimation();
						    }
							
							
							// set the URL to /demo without causing page transition
							// if (history && history.pushState){
							// 	history.pushState({}, 'WorldView Demo', '/demo');	
							// }
							

							// featureWalkThrough.register($scope);
							// demoLoop.register($scope);

							// $scope.$on("pyx-globe-ready", function () {
							// 	console.log("PYX GLOBE READY?? ");
							// 	featureWalkThrough.register($scope);
							// 	demoLoop.register($scope);
							// });   
						});

						// var injector = angular.element(".ng-scope").injector();
			          


					}).fail(function(){
						console.log("ERROR ", arguments);
					});

					// var injector = angular.element(".ng-scope").injector();
		            // injector.get('studioShim');

					// trying bundle now
					// before I attempted to load scripts individually
					// $.getScript('/build/landing-demo-bundle.js').done(function(){
					// 	console.log("LOADED DEMO BUNDLE");
					// }).fail(function(){
					// 	console.log("ERR LOADING", arguments);
					// });

				});
			});

		}	
	}
	

	return {
		restrict: 'A',
		link: postlink
	};
});

app.directive('timeLine', function ($timeout) {
    return {
        restrict: "E",
        scope: {
            'timeLine': "=",
            'sections': "=",
            'state': "=",
            'onSelectionClick': "&",
            'onTimeLineClick': "&",
            'onCurrentTimeMousedown': "&",
            'onCurrentTimeMousemove': "&",
            'onCurrentTimeMouseup': "&"
        },
        replace: true,
        template: [
            '<div class="timeline">',
            '<div ng-repeat="section in sections" class="section" ng-class="{active: currentSection == section }" ng-style="{left: timeToPercent(section.start), width: timeToPercent(section.duration)}" ng-click="sectionClick(section,$event)"><span class="section-name">{{section.name}}</span><div class="section-bar"></div></div>',
            '<div class="background" ng-click="timelineClick(null,$event)"><div class="current-time-fill" ng-style="{width: timeToPercent(state.currentTime)}"></div></div>',
            '<span ng-repeat="step in timeLine" class="step step-{{$index}}" ng-style="{left: stepPosition($index)}" ng-click="timelineClick(step,$event)"></span>',
            '<span class="current-time" ng-style="{left: timeToPercent(state.currentTime)}" ng-mousedown="currentTimeMousedown($event)" ng-mousemove="currentTimeMousemove($event)" ng-mouseup="currentTimeMouseup($event)"></span>',
            '</div>'
        ].join(""),
        link: function (scope, element, attrs) {
            console.log("bootstrap timeline", scope.state);

            scope.stepStartTime = [];
            scope.endTime = 0;
            angular.forEach(scope.timeLine, function (step) {
                scope.stepStartTime.push(scope.endTime);
                scope.endTime += step.duration;
            });
            
            scope.stepPosition = function (index) {
                return scope.timeToPercent(scope.stepStartTime[index]);
            }

            scope.timeToPercent = function (time) {
                return (100.0 * time / scope.endTime) + "%";
            }

            function updateActiveSection() {
                angular.forEach(scope.sections, function (section) {
                    if (scope.state.currentTime >= section.start && scope.state.currentTime < section.start + section.duration) {
                        scope.currentSection = section;
                    }
                });
            }

            // function handleState (state, prevState) {
            //     if (!state) {
            //         TweenMax.pauseAll();
            //     } else {
            //         TweenMax.resumeAll();
            //     }
            // };

            updateActiveSection();

            scope.$watch('state.currentTime', updateActiveSection);
            //scope.$watch('state.play', handleState);

            scope.getTimeFromEvent = function (event) {
                var offset = element.offset();
                var left = event.pageX - offset.left;

                var time = (scope.endTime * 1.0 * left) / element.width();
                if (time < 0) {
                    time = 0;
                }
                if (time > scope.endTime) {
                    time = scope.endTime;
                }

                return time;
            }

            scope.sectionClick = function (section, $event) {
                scope.onTimeLineClick({ section: section, $event: event, $time: scope.getTimeFromEvent(event) });
            }

            scope.timelineClick = function (step, event) {
                var time = scope.getTimeFromEvent(event);
                if (!step) {
                    angular.forEach(scope.timeLine, function (aStep, index) {
                        if (time >= scope.stepStartTime[index]) {
                            step = aStep;
                        }
                    });
                }
                scope.onTimeLineClick({ $step: step, $event: event, $time: time });
            }

            scope.currentTimeMousedown = function (event) {
                scope.onCurrentTimeMousedown({ $event: event, $time: scope.getTimeFromEvent(event) });
            }
            scope.currentTimeMousemove = function (event) {
                scope.onCurrentTimeMousemove({ $event: event, $time: scope.getTimeFromEvent(event) });
            }
            scope.currentTimeMouseup = function (event) {
                scope.onCurrentTimeMouseup({ $event: event, $time: scope.getTimeFromEvent(event) });
            }
        }
    }
});