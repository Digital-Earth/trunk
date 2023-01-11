app.service("featureLocalPublish", function ($pyx) {
    
    function register($scope) {
        $scope.publishItemLocally = function (item) {
            $scope.currentMap.getGeoSource(item).then(function (geoSource) {
                $pyx.engine.publishLocally(geoSource);
            });
        };

        $scope.unpublishItemLocally = function (item) {
            $scope.currentMap.getGeoSource(item).then(function (geoSource) {
                $pyx.engine.unpublishLocally(geoSource);
            });
        };
    };

    return {
        register: register
    };
});