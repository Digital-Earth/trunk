/*
worldViewStudioController aimed to be working on the web version only.
*/
app.controller("worldViewStudioController", function (
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap,
    //demos
    featureInspectorTools
    ) {

    worldViewStudioBootstrap($scope);

    featureInspectorTools.register($scope);
});