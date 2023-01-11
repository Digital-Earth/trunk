/**
	This file is just to be able to override imports with custom components
*/

var CesiumWidget = require('terriajs-cesium/Source/Widgets/CesiumWidget/CesiumWidget');
var TerriaViewer = require('terriajs/lib/ViewModels/TerriaViewer');
var SingleTileImageryProvider = require('terriajs-cesium/Source/Scene/SingleTileImageryProvider');
var CesiumTerrainProvider = require('terriajs-cesium/Source/Core/CesiumTerrainProvider');
var EllipsoidTerrainProvider = require('terriajs-cesium/Source/Core/EllipsoidTerrainProvider');
var PopupMessageViewModel = require('terriajs/lib/ViewModels/PopupMessageViewModel');


TerriaViewer.prototype._createCesiumViewer = function(container) {

    var that = this;

    var terrainProvider = that._terrainProvider;


    console.log("CREATE CESIUM VIEWER inside terria", this.terria.dataSources);
    var options = {
        dataSources:  this.terria.dataSources,
        clock:  this.terria.clock,
        // terrainProvider : terrainProvider,
        imageryProvider : new SingleTileImageryProvider({ url: 'assets/cesium/images/nicta.png' }),
        scene3DOnly: true
    };

    // Workaround for Firefox bug with WebGL and printing:
    // https://bugzilla.mozilla.org/show_bug.cgi?id=976173
    if (FeatureDetection.isFirefox()) {
        options.contextOptions = {webgl : {preserveDrawingBuffer : true}};
    }

     //create CesiumViewer
    var viewer = new CesiumWidget(container, options);
    console.log("after cesium widget created");

    viewer.scene.imageryLayers.removeAll();

    //catch Cesium terrain provider down and switch to Ellipsoid
    terrainProvider.errorEvent.addEventListener(function(err) {
        console.log('Terrain provider error.  ', err.message);
        if (viewer.scene.terrainProvider instanceof CesiumTerrainProvider) {
            console.log('Switching to EllipsoidTerrainProvider.');
            viewer.scene.terrainProvider = new EllipsoidTerrainProvider();
            if (!defined(that.TerrainMessageViewed)) {
                PopupMessageViewModel.open('ui', {
                    title : 'Terrain Server Not Responding',
                    message : '\
The terrain server is not responding at the moment.  You can still use all the features of '+that.terria.appName+' \
but there will be no terrain detail in 3D mode.  We\'re sorry for the inconvenience.  Please try \
again later and the terrain server should be responding as expected.  If the issue persists, please contact \
us via email at '+that.terria.supportEmail+'.'
                });
                that.TerrainMessageViewed = true;
            }
        }
    });

    var scene = viewer.scene;

    scene.globe.depthTestAgainstTerrain = false;

    var d = this._getDisclaimer();
    if (d) {
        scene.frameState.creditDisplay.addDefaultCredit(d);
    }

    if (defined(this._developerAttribution)) {
        scene.frameState.creditDisplay.addDefaultCredit(new Credit(this._developerAttribution.text, undefined, this._developerAttribution.link));
    }
    scene.frameState.creditDisplay.addDefaultCredit(new Credit('CESIUM', undefined, 'http://cesiumjs.org/'));

    var inputHandler = viewer.screenSpaceEventHandler;

    // Add double click zoom
    inputHandler.setInputAction(
        function (movement) {
            zoomIn(scene, movement.position);
        },
        ScreenSpaceEventType.LEFT_DOUBLE_CLICK);
    inputHandler.setInputAction(
        function (movement) {
            zoomOut(scene, movement.position);
        },
        ScreenSpaceEventType.LEFT_DOUBLE_CLICK, KeyboardEventModifier.SHIFT);

    return viewer;
};



module.exports = {
	TerriaViewer: TerriaViewer
}