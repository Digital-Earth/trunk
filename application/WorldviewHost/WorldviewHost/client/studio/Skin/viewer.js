/*
worldViewStudioController aimed to be the skin for web demos (not fully working studio)
*/
app.controller("worldViewStudioController", function (
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap, $window, $timeout,

    featureWalkThrough,
    featureInspectorTools

    ) {

    $scope.excludedTemplates = ["/client/templates/studio/template/feature-dashboard.html"];

    worldViewStudioBootstrap($scope, { mode: 'viewer' });



    // temporary debug
    $window.$scope = $scope;

    featureInspectorTools.register($scope);

    $scope.uiActive = true;
    $scope.uiReadonly = true;

    $scope.$on("pyx-engine-ready", function () {
        console.log("PYX ENGINE READY");

        // pass in map object that is embeded within the html from the server
        $scope.setCurrentMap($window.resourceObject);

        $('.loading-bar > span').css('width', '10%');

        if( $window.resourceObject.Camera){
            // TODO :: support an on load event for map/globe interface
            setTimeout(function(){
                $pyx.globe.setCamera($window.resourceObject.Camera, 2000);
            }, 3000);
        }


        // redefine the "addGeoSource" method to delete
        // the currentMap so that opens the geoSource rather than importing on top
        // also go to the geosource at the end
        var oldAddFunction = $scope.addGeoSource;

        $scope.addGeoSource = function(geoSource, style){

            // pass in geoSource name to new map
            $scope.createNewMap(geoSource.Metadata.Name);

            console.log("custom add geoSource ", geoSource);
            oldAddFunction(geoSource, style);
            $pyx.globe.gotoGeoSource(geoSource, 1000);
        }
    });
});

