/*
worldViewStudioController aimed to work within an embedded web browser with access to C# SDK
*/
app.controller("worldViewStudioController", function (
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap
    ) {
    worldViewStudioBootstrap($scope);
});

