/*
worldViewStudioController aimed to be the skin for web demos (not fully working studio)
*/
app.controller("worldViewStudioController", function (
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap,

    //demos
    featureWalkThrough,
    featureInspectorTools,
    demoClimate,
    demoLoop

    ) {

    worldViewStudioBootstrap($scope, { mode: 'demo' });
    
    featureInspectorTools.register($scope);

    $scope.$on("pyx-globe-ready", function () {
        featureWalkThrough.register($scope);
        demoLoop.register($scope);
    });   
});

