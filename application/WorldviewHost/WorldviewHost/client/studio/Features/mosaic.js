app.service("featureMosaic", function ($pyx, $filter, worldViewStudioConfig) {
    var i18n = $filter("i18n");
    
    function register($scope) {
        /**
        * Attempt to create a GeoSource that is a mosaic of the GeoSources in the Map.
        * @param map - the map to attempt create a mosaic from its contained GeoSources
        * @param index - the index of the map in the library
        */
        $scope.mosaic = function (map, index) {
            if (!map.Groups || !map.Groups.length) {
                $scope.notifyError(i18n("Globe must not be empty to attempt to create a mosaic."));
                return;
            }
            var geoSources = [];
            var items = [];
            angular.forEach(map.Groups, function (group) {
                items.push.apply(items, group.Items);
            });
            
            var createMosaic = function (geoSources) {
                $pyx.engine.mosaic(geoSources).success(function (geoSource) {
                    $scope.createNewMap();
                    $scope.currentMap.importGeoSource(geoSource).success(function (style) {
                        $scope.addResource(geoSource, style);
                    });
                }).error(function (error) {
                    $scope.notifyError(error);
                });
            }
            
            var resolveGeoSourceInMap = function (index) {
                $scope.currentMap.getGeoSource(items[index]).then(function (geoSource) {
                    geoSources.push(geoSource);
                    ++index;
                    if (index < items.length) {
                        resolveGeoSourceInMap(index);
                    } else {
                        createMosaic(geoSources);
                    }
                }, function (error) {
                    $scope.notifyError(i18n("Failed to resolve a GeoSource in the Globe.  Cancelling mosaic operation."));
                });
            }
            
            resolveGeoSourceInMap(0);
        }
    };
    
    return {
        register: register
    };
});