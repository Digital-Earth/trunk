/*
worldViewStudioController aimed to be the skin for web demos (not fully working studio)
*/
app.controller("worldViewStudioController", function (
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap, $window,

    featureWalkThrough,
    featureInspectorTools

    ) {

    worldViewStudioBootstrap($scope, { mode: 'viewer' });

    // temporary debug
    $window.$scope = $scope;

    featureInspectorTools.register($scope);

    $scope.onTimeLineClick = function (time) {
        $scope.state.currentTime = time;
        $scope.state.startClockTime = (new Date().getTime()) - $scope.state.currentTime * 1000;
    }

    $scope.uiActive = true;

    $scope.$on("pyx-engine-ready", function () {
        console.log("PYX ENGINE READY");
        

        // pass in map object that is embeded within the html from the server
        $scope.setCurrentMap($window.mapObject);

        if( $window.mapObject.Camera){
            $pyx.globe.setCamera($window.mapObject.Camera, 1000);    
        }

        
    });
});

