app.service("featureGoto", function ($pyx, $filter, $http, $pyxIntercom, worldViewStudioConfig) {
    var i18n = $filter("i18n");
    
    function register($scope) {
        $scope.addTemplate('sections', '/client/templates/studio/template/feature-navigation.html');
        
        $scope.gotoCamera = function (camera, duration) {
            $pyx.globe.setCamera(camera, duration || worldViewStudioConfig.globe.gotoCameraAnimationDuration);
        };
        
        $scope.gotoGeometry = function (geometry, duration) {
            $pyx.globe.gotoGeometry(geometry, duration || worldViewStudioConfig.globe.gotoCameraAnimationDuration);
        };
        
        $scope.gotoItem = function (item) {
            if (item && item.Active) {
                $scope.currentMap.getGeoSource(item).then(function (geoSource) {
                    $pyx.globe.gotoGeoSource(geoSource, worldViewStudioConfig.globe.gotoCameraAnimationDuration);
                });
            }
        };
        
        $scope.gotoUserLocation = function () {
            $http.get("//freegeoip.net/json/").success(function (ipinfo) {
                // inform the network state verification service about the successful request
                // (the message won't be shown to the user unless the service has previously been down)
                $scope.alertNetworkServiceState('featureGoto', true, i18n("Location detection service has recovered"));
                //should return json like this: 
                //{"ip":"68.147.146.219",
                // "country_code":"CA",
                // "country_name":"Canada",
                // "region_code":"AB",
                // "region_name":"Alberta",
                // "city":"Calgary",
                // "zip_code":"",
                // "time_zone":"America/Edmonton",
                // "latitude":51.083,
                // "longitude":-114.084,
                // "metro_code":0}
                var camera = $pyx.globe.getCamera();
                camera = {
                    'Latitude': ipinfo.latitude,
                    'Longitude': ipinfo.longitude,
                    'Tilt': camera.Tilt,
                    'Heading': 0,
                    'Range': worldViewStudioConfig.globe.gotoUserLocationCameraRange
                };
                $pyx.globe.setCamera(camera, worldViewStudioConfig.globe.gotoCameraAnimationDuration);
            }).error(function (error) {
                // no response to the request; alert the failure
                $scope.alertNetworkServiceState('featureGoto', false, i18n("Failed to obtain your current location"));
            });
            $pyxIntercom.track('goto-user-location');
        };
    };
    
    return {
        register: register
    };
});