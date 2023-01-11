/*
worldViewStudioController aimed to be working on the web version only.
*/
app.controller("worldViewStudioController", function (
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap,

    //demos
    featureShare,
    featureInspectorTools
    ) {

    worldViewStudioBootstrap($scope);

	featureShare.register($scope);
    featureInspectorTools.register($scope);
});